#include "addrspace.h"
#include "system.h"
#include "TLBManager.h"

TLBManager::TLBManager(int size)
{
	tlbSize = size;
	emptyEntryNum = size;
	relatedThreadId = new int[size];
	lastModifyTime = new int[size];
	tlbPageTable = new TranslationEntry[size];

	for (int i = 0; i < size; i++)
	{
		tlbPageTable[i].valid = FALSE;
		relatedThreadId[i] = -1;
		lastModifyTime[i] = 0;
	}
}

TLBManager::~TLBManager()
{
	delete [] relatedThreadId;
	delete [] lastModifyTime;
	delete [] tlbPageTable;
}

TranslationEntry*
TLBManager::findPageEntry(int threadId, unsigned int pageNum)
{
	TranslationEntry* entry = NULL;

	for (int i = 0; i < tlbSize; i++)
	{
		if (tlbPageTable[i].valid == TRUE
			&& relatedThreadId[i] == threadId
			&& tlbPageTable[i].virtualPage == pageNum)
		{
			// Found the page entry in TLB.
			lastModifyTime[i] = stats->totalTicks;
			entry = &tlbPageTable[i];
			break;
		}
	}

	return entry;
}

void
TLBManager::cacheOnePageEntry(unsigned int pageNum)
{
	int target;

	// 1. Find one proper entry in TLB.
	if (emptyEntryNum > 0)
	{
		for (int i = 0; i < tlbSize; i++)
		{
			if (!tlbPageTable[i].valid)
			{
				target = i;
				emptyEntryNum--;

				break;
			}
		}
	}
	else
	{
		int min;
		for (int i = 0; i < tlbSize; i++)
		{
			if (i == 0 || lastModifyTime[i] < min)
			{
				min = lastModifyTime[i];
				target = i;
			}
		}
	}

	// 2. Write the target page entry back to global page table.
	if (tlbPageTable[target].valid)
	{
		AddrSpace* space = memoryManager->getAddrSpaceOfThread(relatedThreadId[target]);
		TranslationEntry* pageTable = space->getPageTable();
		pageTable[tlbPageTable[target].virtualPage] = tlbPageTable[target];
	}

	// 3. Cache the new page entry in TLB.
	tlbPageTable[target] = machine->pageTable[pageNum];
	relatedThreadId[target] = currentThread->getThreadID();
	lastModifyTime[target] = stats->totalTicks;
}

