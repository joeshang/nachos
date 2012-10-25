// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;

	currentThread->Yield();	// it's time to schedule

    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!

//----------------------------------------------------------------------
// Lock::Lock
// 	Initialize a Lock, so that it can be used for mutex.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Lock::Lock(char* debugName)
{
	semaphore = new Semaphore("lock", 1);
	name = debugName;
	lockHolder = NULL;
}

//----------------------------------------------------------------------
// Lock::~Lock
// 	De-allocate lock, when no longer needed.  Assume no one
//	is still waiting on the lock!
//----------------------------------------------------------------------

Lock::~Lock()
{
	delete semaphore;
}

void Lock::Acquire()
{
	// 1. close interrupt
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	// 2. semaphore operate P and update lockHolder
	semaphore->P();
	lockHolder = currentThread;

	// 3. resume interrupt
	(void)interrupt->SetLevel(oldLevel);
}

void Lock::Release()
{
	// 1. disable interrupt
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

	// 2. assert only the holder thread can release the lock
	ASSERT(isHeldByCurrentThread());

	// 3. reset lockHolder and semaphore operate V
	lockHolder = NULL;
	semaphore->V();

	// 4. resume interrupt
	(void)interrupt->SetLevel(oldLevel);
}

bool Lock::isHeldByCurrentThread()
{
	return lockHolder == currentThread;
}

Condition::Condition(char* debugName)
{
	name = debugName;
	waitingList = new List();
}

Condition::~Condition()
{
	delete waitingList;
}

void Condition::Wait(Lock* conditionLock)
{
	// 1. disable interrupt
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	// 2. assert the condition lock belong to current thread
	ASSERT(conditionLock->isHeldByCurrentThread());

	// 3. release condition lock because condition segment outside must be locked
	conditionLock->Release();

	// 4. add thread in waiting list and sleeping for condition matching
	waitingList->Append((void*)currentThread);
	currentThread->Sleep();

	// 5. condition matching, so it's time to acquire the condition lock 
	conditionLock->Acquire();
	
	// 6. resume interrupt
	(void)interrupt->SetLevel(oldLevel);
}

void Condition::Signal(Lock* conditionLock)
{
	// 1. disable interrupt
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	// 2. assert the condition lock belong to current thread
	ASSERT(conditionLock->isHeldByCurrentThread());

	// 3. condition matching, so find a thread to schedule in waiting list
	if (!waitingList->IsEmpty())
	{
		Thread* waitingThread = (Thread*)waitingList->Remove();
		scheduler->ReadyToRun(waitingThread);
	}

	// 4. resume interrupt
	(void)interrupt->SetLevel(oldLevel);
}

void Condition::Broadcast(Lock* conditionLock)
{
	// 1. disable interrupt
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	// 2. assert the condition lock belong to current thread
	ASSERT(conditionLock->isHeldByCurrentThread());

	// 3. signal all threads waiting in list
	while (!waitingList->IsEmpty())
	{
		Signal(conditionLock);
	}

	// 4. resume interrupt
	(void)interrupt->SetLevel(oldLevel);

}
