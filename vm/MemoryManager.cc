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
MemoryManager::getAddrSpaceOfThread(int threadId)
{
	return (virtMemManager->getAddrSpaceOfThread(threadId));
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
	AddrSpace* currThreadAddrSpace = virtMemManager->getAddrSpaceOfThread(currThreadId);
	TranslationEntry* currPageTable = currThreadAddrSpace->getPageTable();

	if (!currPageTable[virtPage].valid)
	{
		// 1. Find one physical page to swap out.
		swapPhyPage = phyMemManager->findOneEmptyPage();

		if (swapPhyPage == -1) // Check is any physical page swapping?
		{
			// 2. Get this swapping page's virtual page table.
			swapPhyPage = phyMemManager->swapOnePage();
			int swapMainThreadId = phyMemManager->getMainThreadId(swapPhyPage);
			int swapVirtPage = phyMemManager->getVirtualPage(swapPhyPage);
			TranslationEntry* swapPageTable = virtMemManager->getAddrSpaceOfThread(swapMainThreadId)->getPageTable();
			
			// 3. Check the page whether dirty or not.
			if (swapPageTable[swapVirtPage].dirty) // Modified, moving page into swapping space.
			{
				swapPageTable[swapVirtPage].swappingPage = swappingManager->swapIn(swapPhyPage);
				machine->tlb->invalidPageEntry(swapMainThreadId, swapVirtPage);
				// TODO: handle swapIn return -1 (No Space in swapping file).
			}

			swapPageTable[swapVirtPage].valid = FALSE;
		}

		// 4. Set new information of physical page.
		int mainThreadId = currThreadAddrSpace->getMainThreadId();
		phyMemManager->setVirtualPage(swapPhyPage, virtPage);
		phyMemManager->setMainThreadId(swapPhyPage, mainThreadId);
		phyMemManager->updatePageWeight(swapPhyPage);

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

