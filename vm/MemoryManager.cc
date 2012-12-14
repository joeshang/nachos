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

AddrSpace*
MemoryManager::createAddrSpace(int mainThreadId, OpenFile* executable)
{
	return (virtMemManager->createAddrSpace(mainThreadId, executable));
}

AddrSpace*
MemoryManager::shareAddrSpace(int mainThreadId, int currThreadId)
{
	return (virtMemManager->shareAddrSpace(mainThreadId, currThreadId));
}

void
MemoryManager::deleteAddrSpace(int threadId)
{
	virtMemManager->deleteAddrSpace(threadId);
}

void
MemoryManager::process(int virtPage)
{
	int swapPhyPage;

	int currThreadId = currentThread->getThreadID();
	AddrSpace** globalVirtMemTable = virtMemManager->getVirtMemCtrlTable();
	TranslationEntry* currPageTable = globalVirtMemTable[currThreadId]->getPageTable();

	if (!currPageTable[virtPage].valid)
	{
		bool isSwapping;

		// 1. Find one physical page to swap out.
		swapPhyPage = phyMemManager->findOnePage(&isSwapping);

		if (isSwapping) // Check is any physical page swapping?
		{
			// 2. Get this swapping page's virtual page table.
			int swapMainThreadId = phyMemManager->getMainThreadId(swapPhyPage);
			int swapVirtPage = phyMemManager->getVirtualPage(swapPhyPage);
			TranslationEntry* swapPageTable = globalVirtMemTable[swapMainThreadId]->getPageTable();
			
			// 3. Check the page whether dirty or not.
			if (swapPageTable[swapVirtPage].dirty) // Modified, moving page into swapping space.
			{
				swapPageTable[swapVirtPage].valid = FALSE;
				swapPageTable[swapVirtPage].swappingPage = swappingManager->swapIn(swapPhyPage);
				// TODO: handle swapIn return -1 (No Space in swapping file).
			}
		}

		// 4. Set new information of physical page.
		int mainThreadId = globalVirtMemTable[currThreadId]->getMainThreadId();
		phyMemManager->setVirtualPage(swapPhyPage, virtPage);
		phyMemManager->setMainThreadId(swapPhyPage, mainThreadId);
		phyMemManager->setLastModifyTime(swapPhyPage, stats->totalTicks);

		// 5. Modify attributes of related virtual page.
		currPageTable[virtPage].valid = TRUE;
		currPageTable[virtPage].physicalPage = swapPhyPage;

		// 6. Load contents of virtual page into virtual page.
		if (currPageTable[virtPage].dirty) // Virtual page in the swapping space.
		{
			swappingManager->swapOut(swapPhyPage, currPageTable[virtPage].swappingPage);
			currPageTable[virtPage].dirty = FALSE;
			currPageTable[virtPage].swappingPage = -1;
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

