#include "system.h"
#include "VirtMemManager.h"

//----------------------------------------------------------------------
// VirtMemManager::VirtMemManager
//	Create a virtual memory manager to contain all virtual pages in the
//	system.
//
//	"size" -- the number of virtual memory table.
//----------------------------------------------------------------------

VirtMemManager::VirtMemManager(int size)
{
	ASSERT(size > 0);

	virtPageNums = 0;
	virtMemCtrlTableSize = size;
	
	virtMemCtrlTable = new AddrSpace*[size];
	for (int i = 0; i < size; i++)
	{
		virtMemCtrlTable[i] = NULL;
	}
}

//----------------------------------------------------------------------
// VirtMemManager::~VirtMemManager
//	Dealloc a virtual memory manager. 
//----------------------------------------------------------------------

VirtMemManager::~VirtMemManager()
{
	delete [] virtMemCtrlTable;
}

AddrSpace*
VirtMemManager::getAddrSpaceOfThread(int threadId)
{
	if (threadId >= 0 && threadId < virtMemCtrlTableSize)
	{
		return (virtMemCtrlTable[threadId]);
	}
	else
	{
		return NULL;
	}
}

//----------------------------------------------------------------------
// VirtMemManager::createAddrSpace
//	Create a page table for thread. The virtual memory manager take over 
//	address space  creation of every thread and combine them into an 
//	overall table.
//
//	"mainThreadId" -- the main thread which hold address space.
//	"executable" -- the executable file identifier.
//----------------------------------------------------------------------

AddrSpace*
VirtMemManager::createAddrSpace(int mainThreadId, OpenFile* executable)
{
	AddrSpace* entry;

	// Check whether the entry valid or not.
	entry = virtMemCtrlTable[mainThreadId];
	if (entry != NULL)
	{
		return entry;
	}
	
	// Empty entry, so create it.
	entry = new AddrSpace(mainThreadId, executable);

	int size = entry->getNumPages();
	if (virtPageNums + size > MAX_VIRT_PAGES)
	{
		delete entry;
		entry = NULL;
	}
	else
	{
		virtMemCtrlTable[mainThreadId] = entry;
		virtPageNums += size;
	}

	return entry;
}

//----------------------------------------------------------------------
// VirtMemManager::shareAddrSpace
//	Children thread ask for parent thread's address space.
//
//	"mainThreadId" -- parent thread identifier.
//	"currThreadId" -- children thread identifier.
//----------------------------------------------------------------------

AddrSpace*
VirtMemManager::shareAddrSpace(int mainThreadId, int currThreadId)
{
	// Invalid parameter check.
	if (currThreadId < 0 || mainThreadId < 0
		|| currThreadId >= virtMemCtrlTableSize
		|| mainThreadId >= virtMemCtrlTableSize
		|| virtMemCtrlTable[mainThreadId] == NULL)
	{
		return NULL;
	}

	virtMemCtrlTable[currThreadId] = virtMemCtrlTable[mainThreadId];
	virtMemCtrlTable[mainThreadId]->incRefCount();
    virtMemCtrlTable[mainThreadId]->allocStackSpace(currThreadId);

	return virtMemCtrlTable[mainThreadId];
}

//----------------------------------------------------------------------
// VirtMemManager::deleteAddrSpace
//	Delete a page table for thread. 
//	NOTICE: If you want to delete the parent thread, you should delete 
//			its child thread first, or it will raise an assert!
//
//	"threadId" -- the thread identifier of deleting page table.
//----------------------------------------------------------------------

void
VirtMemManager::deleteAddrSpace(int threadId)
{
	if (threadId >= 0 && threadId < virtMemCtrlTableSize)
	{
		AddrSpace* entry = virtMemCtrlTable[threadId];
		if (entry != NULL)
		{
			int mainThreadId = entry->getMainThreadId();
			unsigned int refCount = entry->getRefCount();
			unsigned int size = entry->getNumPages();
			TranslationEntry* pageTable = entry->getPageTable();

			// If the child thread dealloc itself, just decrease reference count.
			if (mainThreadId != threadId && refCount > 1)
			{
				entry->decRefCount();
                entry->freeStackSpace(threadId);
			}
			// Delete the address space which process hold(all threads share).
			else if (mainThreadId == threadId && refCount == 1)
			{
				for (int i = 0; i < size; i++)
				{
					// Clear pages in physical memory.
					if (pageTable[i].valid)
					{
						memoryManager->getPhyMemManager()->clearOnePage(pageTable[i].physicalPage);
					}

					// Clear pages in swapping space.
					if (pageTable[i].swappingPage != -1)
					{
						memoryManager->getSwappingManager()->clearOnePage(pageTable[i].swappingPage);
					}

					// TODO: A better way is using message communication(eg.singal-slot) instead of global variation.

				}

				delete entry;
                virtMemCtrlTable[threadId] = NULL;
			}
			else
			{
				// Raising an assert because of deleting parent thread first.
				ASSERT(FALSE);
			}
		}
	}
}
