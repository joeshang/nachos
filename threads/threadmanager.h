// FileName: threadmanager.h
// Descriptions:
//	The manager of all threads in the operating system.
//	Scheduler only handles threads with ready status, the key responsibility
//	is scheduling. So as SRP, the management of all threads should be divided
//	into a new class. The ThreadManager hand over the creation and deletion of 
//	thread because of its overall perspective.
// Author: Joe Shang, 13/10/2012

#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

#define THREAD_COUNT_MAX	128

// The following class defines the manager of all threads -- which contains
// information of all threads.

class ThreadManager
{
	public:
		ThreadManager();
		~ThreadManager();

		Thread* createThread(char* debugName);
		Thread* createThread(char* debugName, int uid);
		void deleteThread(Thread* thread);

		void listThreadStatus();
		int getThreadCnt() { return (threadCnt); }

		int generateThreadID(); 

	private:
		List* threadList;	// List of all threads
		int threadCnt;		// Thread Counter
		int threadIDMap[THREAD_COUNT_MAX];	// Thread ID Map
};

#endif
