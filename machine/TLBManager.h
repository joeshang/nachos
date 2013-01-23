#ifndef TLBMANAGER_H
#define TLBMANAGER_H

#include "translate.h"
#include "SwappingStrategy.h"

class TLBManager
{
	public:
		TLBManager(int size);
		~TLBManager();

		// Find one page entry by virtual page number.
		TranslationEntry* findPageEntry(int threadId, unsigned int vpn);
		// Invalid one page entry by virtual page number.
		void invalidPageEntry(int threadId, unsigned int vpn);
		// Cache one page entry in TLB by page number.
		void cacheOnePageEntry(unsigned int vpn);

	private:
		int tlbSize;
		int emptyEntryNum;
		int* relatedThreadId;		// Related thread id of page entry.
		TranslationEntry* tlbPageTable;	// TLB page table.
		SwappingStrategy* swappingStrategy;
};

#endif
