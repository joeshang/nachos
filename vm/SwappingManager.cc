#include "system.h"
#include "machine.h"
#include "SwappingManager.h"

SwappingManager::SwappingManager()
{
	swappingSpaceMap = new BitMap(MAX_SWAP_SPACE);

	fileSystem->Create(SWAP_SPACE_NAME, MAX_SWAP_SPACE * PageSize);
	swappingFile = fileSystem->Open(SWAP_SPACE_NAME);
}

SwappingManager::~SwappingManager()
{
	delete swappingSpaceMap;
	fileSystem->Remove(SWAP_SPACE_NAME);
}

int
SwappingManager::swapIn(int physicalPage)
{
	int swappingPage;
	int phyMemPosition;
	int swappingPosition;

	if (swappingSpaceMap->NumClear() != 0)
	{	
		swappingPage = swappingSpaceMap->Find();
		phyMemPosition = physicalPage * PageSize;
		swappingPosition = swappingPage * PageSize;

		swappingFile->WriteAt(&(machine->mainMemory[phyMemPosition]), 
								PageSize, 
								swappingPosition);
	}
	else
	{
		printf("Swap In Failed!\n");
		swappingPage = -1;
	}

	return swappingPage;
}

void
SwappingManager::swapOut(int physicalPage, int swappingPage)
{
	int phyMemPosition;
	int swappingPosition;

	if (swappingSpaceMap->Test(swappingPage))
	{
		phyMemPosition = physicalPage * PageSize;
		swappingPosition = swappingPage * PageSize;
		swappingFile->ReadAt(&(machine->mainMemory[phyMemPosition]),
							 PageSize,
							 swappingPosition);

		swappingSpaceMap->Clear(swappingPage);
	}
	else
	{
		printf("Swap Out Failed!\n");
	}
}

void
SwappingManager::clearOnePage(int swappingPage)
{
	swappingSpaceMap->Clear(swappingPage);
}

bool
SwappingManager::isPageValid(int swappingPage)
{
	return (swappingSpaceMap->Test(swappingPage));
}

