#ifndef PHYMEMMANAGER_H
#define PHYMEMMANAGER_H

#include "bitmap.h"

class PhyMemManager
{
	public:
		PhyMemManager(int num);
		~PhyMemManager();

		int FindOnePage();
		void setFileIdentifier(int page, int value);
		int  getFileIdentifier(int page);
		void setDirty(int page, bool value);
		bool getDirty(int page);

	private:
		int pageNum;
		BitMap *memoryMap;
		int *lastModifyTime;
		int *fileIdentifier;
		bool *dirty;
};

#endif
