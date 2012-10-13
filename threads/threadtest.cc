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
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadExercise1Test
//	Print threadID and userID of current thread
//----------------------------------------------------------------------

void
PrintThreadInfo(int arg)
{
	printf("--> I'm thread %u which created by user %u\n",
			currentThread->getThreadID(),
			currentThread->getUserID());

	currentThread->Yield();
}

//----------------------------------------------------------------------
// ThreadExercise1Test
//	Set up three threads with UserID, every thread call PrintThreadInfo.
//----------------------------------------------------------------------

void
ThreadExercise1Test()
{
	DEBUG('t', "Enter Thread Exercise 1 Test");

	Thread* t1 = new Thread("thread 1", 10);
	t1->Fork(PrintThreadInfo, 1);
	
	Thread* t2 = new Thread("thread 2", 20);
	t2->Fork(PrintThreadInfo, 1);

	Thread* t3 = new Thread("thread 3", 30);
	t3->Fork(PrintThreadInfo, 1);

	PrintThreadInfo(0);
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
	ThreadTest1();
	break;
	case 2:
	ThreadExercise1Test();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

