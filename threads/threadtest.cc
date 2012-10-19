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

void
JustYield(int which)
{
	printf("Current running thread is %s\n", currentThread->getName());
	threadManager->listThreadStatus();
	currentThread->Yield();
	threadManager->listThreadStatus();
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

//----------------------------------------------------------------------
// ThreadExercise2Test
//----------------------------------------------------------------------
void 
ThreadExercise2Test()
{
    DEBUG('t', "Entering Thread Exercise Test 2");

	printf("Create 3 threads:\n");

	Thread* t1 = threadManager->createThread("thread 1", 10);
	Thread* t2 = threadManager->createThread("thread 2", 20);
	Thread* t3 = threadManager->createThread("thread 3", 30);

	threadManager->listThreadStatus();

	printf("After fork 3 threads:\n");

	t1->Fork(JustYield, t1->getThreadID());
	t2->Fork(JustYield, t1->getThreadID());
	t3->Fork(JustYield, t3->getThreadID());

	threadManager->listThreadStatus();
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadExercise1Test();
	break;
	case 2:
	ThreadExercise2Test();
	break;
	case 3:
//	ThreadExercise3Test();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

