#include "phymemmanager.h"
#include "system.h"

PhyMemManager::PhyMemManager(int num)
{
	int i;

	pageNum = num;
	memoryMap = new BitMap(num);

	lastModifyTime = new int[num];
	fileIdentifier = new int[num];
	dirty = new bool[num];
	for (i = 0; i < num - 1; i++)
	{
		lastModifyTime[i] = 0;
		dirty[i] = false;
	}
}

PhyMemManager::~PhyMemManager()
{
	delete memoryMap;
	delete lastModifyTime;
	delete fileIdentifier;
	delete dirty;
}

int
PhyMemManager::FindOnePage()
{
	int page;
	int min;

	if (memoryMap->NumClear() != 0)
	{
		page = memoryMap->Find();
		lastModifyTime[page] = stats->totalTicks;
	}
	else
	{
		int i;

		for (i = 0; i < pageNum - 1; i++)
		{
			if (i == 0)
			{
				min = lastModifyTime[i];
			}
			else
			{
				if (lastModifyTime[i] < min)
				{
					min = lastModifyTime[i];
				}
			}
		}	

		page = i;
	}

	return page;
}

bool
PhyMemManager::getDirty(int page)
{
	if (page < 0)
	{
		page = 0;
	}

	if (page >= pageNum)
	{
		page = pageNum - 1;
	}

	return (dirty[page]);
}

void
PhyMemManager::setDirty(int page, bool value)
{
	if (page >= 0 && page < pageNum)
	{
		dirty[page] = value;
	}
}

int
PhyMemManager::getFileIdentifier(int page)
{
	if (page < 0)
	{
		page = 0;
	}

	if (page >= pageNum)
	{
		page = pageNum - 1;
	}

	return (fileIdentifier[page]);
}

void
PhyMemManager::setFileIdentifier(int page, int value)
{
	if (page >= 0 && page < pageNum)
	{
		fileIdentifier[page] = value;
	}
}

