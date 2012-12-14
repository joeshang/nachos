#ifndef SWAPPINGMANAGER_H
#define SWAPPINGMANAGER_H

#include "bitmap.h"

#define SWAP_SPACE_NAME	"Swapping"
#define MAX_SWAP_SPACE 	4096

class SwappingManager
{
	public:
		SwappingManager();
		~SwappingManager();

		int swapIn(int physicalPage);
		void swapOut(int physicalPage, int swappingPage);
		
		void clearOnePage(int swappingPage);
		bool isPageValid(int swappingPage);
	
	private:
		BitMap* swappingSpaceMap;
		OpenFile* swappingFile;
};

#endif
