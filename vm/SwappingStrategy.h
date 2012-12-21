#ifndef SWAPPINGSTRATEGY_H
#define SWAPPINGSTRATEGY_H

class SwappingStrategy
{
	public:
		virtual int findOneElementToSwap() = 0;
		virtual void updateElementWeight(int index) = 0;
};

#endif
