// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "translate.h"

#define UserThreadMax       10
#define UserThreadStackSize 1024    // increase this as necessary!
#define UserStackSize		(UserThreadMax * UserThreadStackSize) 	

class AddrSpace {
  public:
    AddrSpace(int threadId, OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 

	OpenFile *getExeFileId() { return (exeFileId); }
	int getMainThreadId() { return (mainThreadId); }
	unsigned int getRefCount() { return (refCount); }
	unsigned int getNumPages() { return (numPages); }
	TranslationEntry* getPageTable() { return (pageTable); }

	void incRefCount() { (refCount++); }
	void decRefCount();

    void allocStackSpace(int threadId);
    void freeStackSpace(int threadId);
    int getThreadStackTop(int threadId);

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
									// for now!
	int mainThreadId;				// The thread ID which hold address space.
	unsigned int numPages;			// Number of pages in address space.
    unsigned int refCount;			// Reference counts of address space.
    int stackSpace[UserThreadMax];  // The usage of user thread's private stack space.
	OpenFile *exeFileId;			// Executable file identifier
};

#endif // ADDRSPACE_H
