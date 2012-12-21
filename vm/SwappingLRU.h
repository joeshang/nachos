#ifndef SWAPPINGLRU_H
#define SWAPPINGLRU_H

#include "SwappingStrategy.h"

class SwappingLRU : public SwappingStrategy
{
	public:
		SwappingLRU(int size);
		~SwappingLRU();

		virtual int findOneElementToSwap();
		virtual void updateElementWeight(int index);
	
	private:
		int* lastUsedTimeTable;
		int tableSize;
};

#endif
