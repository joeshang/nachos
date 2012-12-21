#include "system.h"
#include "PhyMemManager.h"
#include "SwappingLRU.h"

PhyMemManager::PhyMemManager(int pageNums)
{
	phyPageNums = pageNums;
	phyMemoryMap = new BitMap(pageNums);
	phyMemPageTable = new PhyMemPageEntry[pageNums];
	swappingStrategy = new SwappingLRU(pageNums);
}

PhyMemManager::~PhyMemManager()
{
	delete phyMemoryMap;
	delete [] phyMemPageTable;
	delete swappingStrategy;
}

int
PhyMemManager::findOneEmptyPage()
{
	int page;

	if (phyMemoryMap->NumClear() != 0)
	{
		page = phyMemoryMap->Find();
	}
	else
	{
		page = -1;
	}

	return page;
}

int
PhyMemManager::swapOnePage()
{
	return (swappingStrategy->findOneElementToSwap());
}

void
PhyMemManager::clearOnePage(int phyPage)
{
	phyMemoryMap->Clear(phyPage);
}

// TODO: if multiple virtual pages can map to one physical page, clearing page
// should check whether that physical page has multiple virtual pages or not.
// Multiple virtual pages: remove that virtual page from list.
// One virtula page: clear that page.
// void
// PhyMemManager::clearOnePage(int phyPage, int virtPage)
// {
// }

bool
PhyMemManager::isPageValid(int phyPage)
{
	return (phyMemoryMap->Test(phyPage));
}

int
PhyMemManager::getMainThreadId(int phyPage)
{
	if (phyMemoryMap->Test(phyPage))
	{
		return (phyMemPageTable[phyPage].mainThreadId);
	}
	else
	{
		return -1;
	}
}

int
PhyMemManager::getVirtualPage(int phyPage)
{
	if (phyMemoryMap->Test(phyPage))
	{
		return (phyMemPageTable[phyPage].virtualPage);
	}
	else
	{
		return -1;
	}
}

void
PhyMemManager::setMainThreadId(int phyPage, int threadId)
{
	if (phyMemoryMap->Test(phyPage))
	{
		phyMemPageTable[phyPage].mainThreadId = threadId;
	}
}

void
PhyMemManager::setVirtualPage(int phyPage, int virtualPage)
{
	if (phyMemoryMap->Test(phyPage))
	{
		phyMemPageTable[phyPage].virtualPage = virtualPage;
	}
}

void
PhyMemManager::updatePageWeight(int phyPage)
{
	if (phyMemoryMap->Test(phyPage))
	{
		swappingStrategy->updateElementWeight(phyPage);
	}
}
