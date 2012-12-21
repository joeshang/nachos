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

		AddrSpace* getAddrSpaceOfThread(int threadId);
		AddrSpace* createAddrSpace(int mainThreadId, OpenFile* executable);
		AddrSpace* shareAddrSpace(int mainThreadId, int currThreadId);
		void deleteAddrSpace(int threadId);

		// Because the lack of singal-slot notification strategy, these 3
		// functions listed below are using for inner communication.
		// So DON"T use those function out of MemoryManager, outside
		// shouldn't know the detail of memory manager.
		VirtMemManager* getVirtMemManager()	{ return virtMemManager; }
		PhyMemManager* getPhyMemManager() { return phyMemManager; }
		SwappingManager* getSwappingManager() { return swappingManager; }

	private:
		VirtMemManager* virtMemManager;
		PhyMemManager* phyMemManager;
		SwappingManager* swappingManager;
};

#endif

