// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// testnum is set in main.cc
int testnum = 2;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadExercise1Test()
{
    DEBUG('t', "Entering Thread Exercise Test 1");

	Thread* t1 = threadManager->createThread("thread 1", 10);
	Thread* t2 = threadManager->createThread("thread 2", 20);
	Thread* t3 = threadManager->createThread("thread 3", 30);
	
	t1->Fork(SimpleThread, t1->getThreadID());
	t2->Fork(SimpleThread, t1->getThreadID());
	t3->Fork(SimpleThread, t3->getThreadID());
}

void
JustYield(int which)
{
	printf("Current running thread is %s\n", currentThread->getName());
	threadManager->listThreadStatus();
	currentThread->Yield();
	threadManager->listThreadStatus();
}

//----------------------------------------------------------------------
// ThreadExercise2Test
//	Set up 3 threads with different priority and then begin to schedule
//	with TS command to show the effect of priority in thread scheduling.
//----------------------------------------------------------------------

void 
ThreadExercise2Test()
{
    DEBUG('t', "Entering Thread Exercise Test 2");

	currentThread->setPriority(4);

	printf("Create 3 threads:\n");

	Thread* t1 = threadManager->createThread("thread 1", 10);
	Thread* t2 = threadManager->createThread("thread 2", 20);
	Thread* t3 = threadManager->createThread("thread 3", 30);

	t1->setPriority(8);
	t2->setPriority(0);
	t3->setPriority(5);

	threadManager->listThreadStatus();

	printf("After fork 3 threads:\n");

	t1->Fork(JustYield, t1->getThreadID());
	t2->Fork(JustYield, t1->getThreadID());
	t3->Fork(JustYield, t3->getThreadID());

	threadManager->listThreadStatus();

	JustYield(currentThread->getThreadID());
}

//----------------------------------------------------------------------
// ThreadExercise3Test
//	Simulate producer-customer problem by semaphore and condition value.
//----------------------------------------------------------------------

#define	USE_COND

void
ThreadExercise3Test()
{
#ifndef USE_COND
	// 1. set up semaphore of buffer
	// 2. fork producer and customer thread
	// 3. begin to schedule
#else
#endif
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum)
   	{
	// Exercise 1:Thread ID and user ID
    case 1:
		ThreadExercise1Test();
		break;
	// Exercise 2:Thread-Status command and priority scheduling
	case 2:
		ThreadExercise2Test();
		break;
	// Exercise 3:Producer-Customer problem with semaphore and condition value
	case 3:
		ThreadExercise3Test();
		break;
    default:
		printf("No test specified.\n");
		break;
    }
}

