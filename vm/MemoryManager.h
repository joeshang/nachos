#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "openfile.h"
#include "VirtMemManager.h"
#include "PhyMemManager.h"
#include "SwappingManager.h"

class MemoryManager
{
	public:
		MemoryManager();
		~MemoryManager();

		void process(int virtPage);

		AddrSpace* createAddrSpace(int mainThreadId, OpenFile* executable);
		AddrSpace* shareAddrSpace(int mainThreadId, int currThreadId);
		void deleteAddrSpace(int threadId);

		VirtMemManager* getVirtMemManager()	{ return virtMemManager; }
		PhyMemManager* getPhyMemManager() { return phyMemManager; }
		SwappingManager* getSwappingManager() { return swappingManager; }

	private:
		VirtMemManager* virtMemManager;
		PhyMemManager* phyMemManager;
		SwappingManager* swappingManager;
};

#endif

