#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "VirtMemManager.h"
#include "PhyMemManager.h"
#include "SwappingManager.h"

class MemoryManager
{
	public:
		MemoryManager();
		~MemoryManager();

		void process(int virtPage);

		TranslationEntry* createPageTable(int mainThreadId, int size);
		TranslationEntry* sharePageTable(int mainThreadId, int currThreadId);
		void deletePageTable(int threadId);

		VirtMemManager* getVirtMemManager()	{ return virtMemManager; }
		PhyMemManager* getPhyMemManager() { return phyMemManager; }
		SwappingManager* getSwappingManager() { return swappingManager; }

	private:
		VirtMemManager* virtMemManager;
		PhyMemManager* phyMemManager;
		SwappingManager* swappingManager;
};

#endif

