#ifndef TLBMANAGER_H
#define TLBMANAGER_H

#include "translate.h"
#include "SwappingStrategy.h"

class TLBManager
{
	public:
		TLBManager(int size);
		~TLBManager();

		// Find one page entry by page number.
		TranslationEntry* findPageEntry(int threadId ,unsigned int pageNum);
		// Cache one page entry in TLB by page number.
		void cacheOnePageEntry(unsigned int pageNum);

	private:
		int tlbSize;
		int emptyEntryNum;
		int* relatedThreadId;		// Related thread id of page entry.
		TranslationEntry* tlbPageTable;	// TLB page table.
		SwappingStrategy* swappingStrategy;
};

#endif
