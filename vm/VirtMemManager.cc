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
	ASSERT(size < 0);

	virtPageNums = 0;
	virtMemTableSize = size;
	
	virtMemTable = new VirtMemEntry*[size];
	for (int i = 0; i < size; i++)
	{
		virtMemTable[i] = NULL;
	}
}

//----------------------------------------------------------------------
// VirtMemManager::~VirtMemManager
//	Dealloc a virtual memory manager. 
//----------------------------------------------------------------------

VirtMemManager::~VirtMemManager()
{
	for (int i = 0; i < virtMemTableSize; i++)
	{
		if (virtMemTable[i] != NULL)
		{
			delete virtMemTable[i];
		}
	}

	delete virtMemTable;
}

//----------------------------------------------------------------------
// VirtMemManager::createVirtMemTable
//	Create a page table for thread. The virtual memory manager take over 
//	page table creation of every thread and combine them into an overall
//	table. Return NULL if threadId/size invalid.
//
//	"size" -- the number of page table in every thread.
//	"mainThreadId" -- the main thread which hold address space.
//----------------------------------------------------------------------

TranslationEntry*
VirtMemManager::createPageTable(int size, int mainThreadId)
{
	VirtMemEntry* entry;

	// Invalid parameter check.
	if (mainThreadId < 0 || mainThreadId >= virtMemTableSize 
		|| size < 0 || virtPageNums + size > MAX_VIRT_PAGES)
	{	
		return NULL;
	}

	// Check whether the entry valid or not.
	entry = virtMemTable[mainThreadId];
	if (entry != NULL)
	{
		return entry->pageTable;
	}
	
	// Empty entry, so create it.
	entry = new VirtMemEntry;
	virtMemTable[mainThreadId] = entry;
	entry->mainThreadId = mainThreadId;
	entry->refCount = 1;
	entry->size = size;
	entry->pageTable = new TranslationEntry[size];

	virtPageNums += size;

	// Initialize thread's page table.
	for (int i = 0; i < size; i++)
	{
		entry->pageTable[i].virtualPage = i;
		entry->pageTable[i].physicalPage = -1;
		entry->pageTable[i].swappingPage = -1;
		entry->pageTable[i].valid = FALSE;
		entry->pageTable[i].readOnly = FALSE;
		entry->pageTable[i].use = FALSE;
		entry->pageTable[i].dirty = FALSE;
	}

	return entry->pageTable;
}

//----------------------------------------------------------------------
// VirtMemManager::shareVirtMemTable
//	Children thread ask for parent thread's page table. 
//
//	"mainThreadId" -- parent thread identifier.
//	"currThreadId" -- children thread identifier.
//----------------------------------------------------------------------

TranslationEntry*
VirtMemManager::sharePageTable(int mainThreadId, int currThreadId)
{
	// Invalid parameter check.
	if (currThreadId < 0 || mainThreadId < 0
		|| currThreadId >= virtMemTableSize
		|| mainThreadId >= virtMemTableSize
		|| virtMemTable[mainThreadId] == NULL)
	{
		return NULL;
	}

	virtMemTable[currThreadId] = virtMemTable[mainThreadId];
	virtMemTable[mainThreadId]->refCount++;	

	return (virtMemTable[mainThreadId]->pageTable);	
}

//----------------------------------------------------------------------
// VirtMemManager::deleteVirtMemTable
//	Delete a page table for thread. 
//	NOTICE: If you want to delete the parent thread, you should delete 
//			its child thread first, or it will raise an assert!
//
//	"threadId" -- the thread identifier of deleting page table.
//----------------------------------------------------------------------

void
VirtMemManager::deletePageTable(int threadId)
{
	int physicalPage;

	if (threadId >= 0 && threadId < virtMemTableSize)
	{
		VirtMemEntry* entry = virtMemTable[threadId];
		if (entry != NULL)
		{
			// If the child thread dealloc itself, just decrease reference count.
			if (entry->mainThreadId != threadId && entry->refCount > 1)
			{
				entry->refCount--;
			}
			// Delete the address space which process hold(all threads share).
			else if (entry->mainThreadId == threadId && entry->refCount == 1)
			{
				for (int i = 0; i < entry->size; i++)
				{
					physicalPage = entry->pageTable[i].physicalPage;
					// TODO: physical memory manager handle those physical pages.
				}

				delete entry;
			}
			else
			{
				// Raising an assert because of deleting parent thread first.
				ASSERT(TRUE);
			}

			entry = NULL;
		}
	}
}
