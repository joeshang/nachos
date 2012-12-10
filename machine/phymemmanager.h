#ifndef PHYMEMMANAGER_H
#define PHYMEMMANAGER_H

#include "bitmap.h"

class PhyMemManager
{
	public:
		PhyMemManager(int num);
		~PhyMemManager();

		int FindOnePage();

	private:
		int pageNum;
		BitMap *memoryMap;
		int *lastModifyTime;
};

#endif
