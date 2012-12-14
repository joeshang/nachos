#include "system.h"
#include "machine.h"
#include "noff.h"
#include "threadmanager.h"
#include "MemoryManager.h"

MemoryManager::MemoryManager()
{
	virtMemManager = new VirtMemManager(THREAD_COUNT_MAX);
	phyMemManager = new PhyMemManager(NumPhysPages);
	swappingManager = new SwappingManager();
}

MemoryManager::~MemoryManager()
{
	delete virtMemManager;
	delete phyMemManager;
	delete swappingManager;
}

TranslationEntry* 
MemoryManager::createPageTable(int mainThreadId, int size)
{
	return (virtMemManager->createPageTable(mainThreadId, size));
}

TranslationEntry*
MemoryManager::sharePageTable(int mainThreadId, int currThreadId)
{
	return (virtMemManager->sharePageTable(mainThreadId, currThreadId));
}

void
MemoryManager::deletePageTable(int threadId)
{
	virtMemManager->deletePageTable(threadId);
}

void
MemoryManager::process(int virtPage)
{
	bool isSwapping;
	int swapPhyPage;
	int mainThreadId;
	int threadVirtPage;
	int currThreadId;
	TranslationEntry* currPageEntry;
	TranslationEntry* swapPageEntry;
	VirtMemEntry** globalVirtMemTable;

	currThreadId = currentThread->getThreadID();
	globalVirtMemTable = virtMemManager->getVirtMemTable();
	currPageEntry = &(globalVirtMemTable[currThreadId]->pageTable[virtPage]);

	if (!currPageEntry->valid)
	{
		// 1. Find one physical page to swap out.
		swapPhyPage = phyMemManager->findOnePage(&isSwapping);

		if (isSwapping) // Check is any physical page swapping?
		{
			// 2. Get this swapping page's virtual page table.
			mainThreadId = phyMemManager->getMainThreadId(swapPhyPage);
			threadVirtPage = phyMemManager->getVirtualPage(swapPhyPage);
			swapPageEntry = &(globalVirtMemTable[mainThreadId]->pageTable[threadVirtPage]);
			
			// 3. Check the page whether dirty or not.
			if (swapPageEntry->dirty) // Modified, moving page into swapping space.
			{
				swapPageEntry->valid = FALSE;
				swapPageEntry->swappingPage = swappingManager->swapIn(swapPhyPage);
				// TODO: handle swapIn return -1 (No Space in swapping file).
			}
		}

		// 4. Set new information of physical page.
		mainThreadId = globalVirtMemTable[currThreadId]->mainThreadId;
		phyMemManager->setVirtualPage(swapPhyPage, virtPage);
		phyMemManager->setMainThreadId(swapPhyPage, mainThreadId);
		phyMemManager->setLastModifyTime(swapPhyPage, stats->totalTicks);

		// 5. Modify attributes of related virtual page.
		currPageEntry->valid = TRUE;
		currPageEntry->physicalPage = swapPhyPage;

		// 6. Load contents of virtual page into virtual page.
		if (currPageEntry->dirty) // Virtual page in the swapping space.
		{
			swappingManager->swapOut(swapPhyPage, currPageEntry->swappingPage);
			currPageEntry->dirty = FALSE;
			currPageEntry->swappingPage = -1;
		}
		else // Virtual page in the disk.
		{
			OpenFile* executable = currentThread->space->getExeFileId();

			// NOTICE: Read executable file should calculate noff header size.
			executable->ReadAt(&(machine->mainMemory[swapPhyPage * PageSize]),
							   PageSize,
							   virtPage * PageSize + sizeof(NoffHeader));
		}
	}
}

