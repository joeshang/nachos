#ifndef PHYMEMMANAGER_H
#define PHYMEMMANAGER_H

#include "bitmap.h"
#include "SwappingStrategy.h"

// The following class defines an entry of physical page.

class PhyMemPageEntry
{
	public:
		int mainThreadId;
		int virtualPage;
};

// The following class defines a manager of physical memory. The physical 
// memory manager contains the using status of physical memory and the 
// mapping/swapping strategy between physical page and virtual page.

class PhyMemManager
{
	public:
		PhyMemManager(int pageNums);
		~PhyMemManager();

		int findOneEmptyPage();
		int swapOnePage();
		void clearOnePage(int phyPage);
		bool isPageValid(int phyPage);

		int getMainThreadId(int phyPage);
		void setMainThreadId(int phyPage, int threadId);
		int getVirtualPage(int phyPage);
		void setVirtualPage(int phyPage, int virtualPage);
		void updatePageWeight(int phyPage);

	private:
		int phyPageNums;
		BitMap* phyMemoryMap;
		PhyMemPageEntry* phyMemPageTable;
		SwappingStrategy* swappingStrategy;
};

#endif
