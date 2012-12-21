// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

static void ThreadFuncForUserProg(int arg);

static void SysCallHaltHandler();
static void SysCallExecHandler();
static void SysCallExitHandler();
static void SysCallJoinHandler();
static void SysCallForkHandler();
static void SysCallYieldHandler();
static void SysCallPrintHandler();

static void ExceptionPageFaultHanlder();


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

	switch (which)
	{
		case SyscallException:
			switch (type)
			{
				case SC_Halt:
					SysCallHaltHandler();
					break;

				case SC_Exec:
					SysCallExecHandler();
					break;

				case SC_Exit:
					SysCallExitHandler();
					break;

				case SC_Join:
					SysCallJoinHandler();
					break;

				case SC_Fork:
					SysCallForkHandler();
					break;

				case SC_Yield:
					SysCallYieldHandler();
					break;

				case SC_Print:
					SysCallPrintHandler();
					break;

				default:
					break;
			}
			break;

		case PageFaultException:
			ExceptionPageFaultHanlder();
			break;

		default:
			printf("Unexpected user mode exception %d %d\n", which, type);
			break;
	}
}

static void ThreadFuncForUserProg(int arg)
{
	currentThread->RestoreUserState();
	// TODO: Need to modify 3 registers: pc, next pc, sp
	if (arg && currentThread->space != NULL)
	{
		// Exec should initialize registers and restore address space.
		currentThread->space->InitRegisters();
		currentThread->space->RestoreState();
	}

	machine->Run();
}

static void SysCallHaltHandler()
{
	DEBUG('a', "Shutdown, initiated by user program.\n");
	interrupt->Halt();
}

static void SysCallExecHandler()
{
	char fileName[100];
	int arg = machine->ReadRegister(4);
	int i = 0;
	do
	{
		machine->ReadMem(arg + i, 1, (int*)&fileName[i]);
	}while(fileName[i++] != '\0');

	OpenFile* executable = fileSystem->Open(fileName);
	if (executable != NULL)	
	{
		Thread* thread = threadManager->createThread(fileName);
		thread->space = memoryManager->createAddrSpace(thread->getThreadID(), executable);
		machine->WriteRegister(2, thread->getThreadID());

		DEBUG('a', "Exec from thread %d -> executable %s\n", 
				currentThread->getThreadID(), fileName);
		thread->Fork(ThreadFuncForUserProg, 1);
	}
	else
	{
		machine->WriteRegister(2, -1);
	}
	
	machine->PCForward();
}

static void SysCallExitHandler()
{
	int threadId = currentThread->getThreadID();
	memoryManager->deleteAddrSpace(threadId);
	currentThread->Finish();

	machine->PCForward();
}

static void SysCallJoinHandler()
{
}

static void SysCallForkHandler()
{
	Thread* thread = threadManager->createThread("UserProg");
	thread->space = memoryManager->shareAddrSpace(currentThread->getThreadID(),
												  thread->getThreadID());
	int userFunc = machine->ReadRegister(4);
	thread->SetUserRegister(PCReg, userFunc);
	thread->SetUserRegister(NextPCReg, userFunc + 4);
	thread->SetUserRegister(StackReg, machine->ReadRegister(StackReg) - 4);

	DEBUG('a', "Fork from thread %d -> thread %d\n", 
			currentThread->getThreadID(),
			thread->getThreadID());

	thread->Fork(ThreadFuncForUserProg, 0);

	machine->PCForward();
}

static void SysCallYieldHandler()
{
	currentThread->Yield();

	machine->PCForward();
}

static void SysCallPrintHandler()
{
	int msg = machine->ReadRegister(4);
	int size = machine->ReadRegister(5);
	char* buf = new char[size + 5];

	int i = 0;
	do
	{
		machine->ReadMem(msg + i, 1, (int*)&buf[i]);
	}while(buf[i++] != '\0');

	printf("%s\n", buf);

	delete buf;

	machine->PCForward();
}

static void ExceptionPageFaultHanlder()
{
	if (machine->tlb != NULL)
	{
		// 1. Get the TLB miss address and calculate which page it belong to.
		int addr = machine->ReadRegister(BadVAddrReg);
		int vpn = (unsigned) addr / PageSize;

		// 2. Handle whether this virtual page in memory or not.
		// After this step, this virtual page is in the physical memory.
		memoryManager->process(vpn);

		// 3. Cache this page in TLB.
		machine->tlb->cacheOnePageEntry(vpn);

		// 4. Page fault statistics.
		stats->numPageFaults++;
	}
}
