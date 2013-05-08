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

// A dummy handler function of user program's Fork/Exec. The main purpose of this
// function is to run virtual machine in nachos(machine->Run()). Before do that,
// Fork need to restore virtual machine's registers and Exec need to init virtual
// machine's registers and pageTable.
static void ThreadFuncForUserProg(int arg)
{
    switch (arg)
    {
        case 0: // Fork
            // Fork just restore registers.
            currentThread->RestoreUserState();
            break;
        case 1: // Exec
            if (currentThread->space != NULL)
            {
                // Exec should initialize registers and restore address space.
                currentThread->space->InitRegisters();
                currentThread->space->RestoreState();
            }
            break;
        default:
            break;
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

    // Get the executable file name from user space.
	do
	{
		machine->ReadMem(arg + i, 1, (int*)&fileName[i]);
	}while(fileName[i++] != '\0');

    // Open the executabel file.
	OpenFile* executable = fileSystem->Open(fileName);
	if (executable != NULL)	
	{
        // Set up a new thread and alloc address space for it.
		Thread* thread = threadManager->createThread(fileName);
		thread->space = memoryManager->createAddrSpace(thread->getThreadID(), executable);

        // Return the new thread id.
		machine->WriteRegister(2, thread->getThreadID());

		DEBUG('a', "Exec from thread %d -> executable %s\n", 
				currentThread->getThreadID(), fileName);
		thread->Fork(ThreadFuncForUserProg, 1);
	}
	else
	{
        // Can't open executable file, so return -1.
		machine->WriteRegister(2, -1);
	}
	
	machine->PCForward();
}

static void SysCallExitHandler()
{
    int exitStatus = machine->ReadRegister(4);
	int threadId = currentThread->getThreadID();

    // Delete thread' address space.
	memoryManager->deleteAddrSpace(threadId);

    // Set thread's exit status.
    currentThread->setExitStatus(exitStatus);

    // Wake up parent if parent is sleeping.
    if (currentThread->getParent()->getStatus() == BLOCKED)
    {
        scheduler->ReadyToRun(currentThread->getParent());
    }

    // Thread finished.
	currentThread->Finish();
}

static void SysCallJoinHandler()
{
    Thread *childThread;
    int exitStatus = 0;
    int childThreadId = machine->ReadRegister(4);
    
    // Check the waiting child thread whether in the exited child list or not.
    childThread = currentThread->removeExitedChild(childThreadId);
    while (childThread == NULL)
    {
        // If the child thread is not in the exited child list, current thread sleep.
        currentThread->Sleep();
        childThread = currentThread->removeExitedChild(childThreadId);
    }

    // Get child thread's exit status.
    exitStatus = childThread->getExitStatus();

    // Clean up resources of child thread and destroy it.
    childThread->cleanUpBeforeDestroy();
    threadManager->deleteThread(childThread);

    // Return the child thread's exit status.
    machine->WriteRegister(2, exitStatus);

	machine->PCForward();
}

static void SysCallForkHandler()
{
	Thread* thread = threadManager->createThread("UserProg");
	thread->space = memoryManager->shareAddrSpace(currentThread->getThreadID(),
												  thread->getThreadID());
	int userFunc = machine->ReadRegister(4);

    // Copy machine registers of current thread to new thread
    thread->SaveUserState(); 

    // Modify PC/SP register of new thread
	thread->SetUserRegister(PCReg, userFunc);
	thread->SetUserRegister(NextPCReg, userFunc + 4);
    // Every thread has its own private stack space
	thread->SetUserRegister(StackReg, thread->space->getThreadStackTop(thread->getThreadID()));

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
    // 1. Get the TLB miss address and calculate which page it belong to.
    int addr = machine->ReadRegister(BadVAddrReg);
    int vpn = (unsigned) addr / PageSize;

    // 2. Handle whether this virtual page in memory or not.
    // After this step, this virtual page is in the physical memory.
    memoryManager->process(vpn);

    if (machine->tlb != NULL)
    {
        // 3. Cache this page in TLB.
        machine->tlb->cacheOnePageEntry(vpn);
    }

    // 4. Page fault statistics.
    stats->numPageFaults++;
}
