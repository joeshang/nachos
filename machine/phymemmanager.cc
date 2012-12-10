#include "phymemmanager.h"
#include "system.h"

PhyMemManager::PhyMemManager(int num)
{
	pageNum = num;
	memoryMap = new BitMap(num);
	lastModifyTime = new int[num];
	for (int i = 0; i < num - 1; i++)
	{
		lastModifyTime[i] = 0;
	}
}

PhyMemManager::~PhyMemManager()
{
	delete memoryMap;
	delete lastModifyTime;
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

