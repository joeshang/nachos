#include "addrspace.h"
#include "system.h"
#include "TLBManager.h"
#include "SwappingLRU.h"

TLBManager::TLBManager(int size)
{
	tlbSize = size;
	emptyEntryNum = size;
	swappingStrategy = new SwappingLRU(size);

	relatedThreadId = new int[size];
	tlbPageTable = new TranslationEntry[size];

	for (int i = 0; i < size; i++)
	{
		tlbPageTable[i].valid = FALSE;
		relatedThreadId[i] = -1;
	}
}

TLBManager::~TLBManager()
{
	delete swappingStrategy;
	delete [] relatedThreadId;
	delete [] tlbPageTable;
}

TranslationEntry*
TLBManager::findPageEntry(int threadId, unsigned int vpn)
{
	TranslationEntry* entry = NULL;

	for (int i = 0; i < tlbSize; i++)
	{
		if (tlbPageTable[i].valid == TRUE
			&& relatedThreadId[i] == threadId
			&& tlbPageTable[i].virtualPage == vpn)
		{
			// Found the page entry in TLB.
			entry = &tlbPageTable[i];
			swappingStrategy->updateElementWeight(i);

			break;
		}
	}

	return entry;
}

void
TLBManager::invalidPageEntry(int threadId, unsigned int vpn)
{
	TranslationEntry* entry = findPageEntry(threadId, vpn);
	if (entry)
	{
		entry->valid = FALSE;
		if (emptyEntryNum < tlbSize)
		{
			emptyEntryNum++;
		}
	}
}

void
TLBManager::cacheOnePageEntry(unsigned int vpn)
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
   		target = swappingStrategy->findOneElementToSwap();
	}

	// 2. Write the target page entry back to global page table.
	if (tlbPageTable[target].valid)
	{
		AddrSpace* space = memoryManager->getAddrSpaceOfThread(relatedThreadId[target]);
        if (space != NULL)
        {
            TranslationEntry* pageTable = space->getPageTable();
            pageTable[tlbPageTable[target].virtualPage] = tlbPageTable[target];
        }
	}

	// 3. Cache the new page entry in TLB.
	tlbPageTable[target] = machine->pageTable[vpn];
	relatedThreadId[target] = currentThread->getThreadID();
	swappingStrategy->updateElementWeight(target);
}

