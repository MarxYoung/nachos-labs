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
#include "synch-sleep.h"
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
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!

//----------------------------------------------------------------------
// Lock::Lock
// 	Initialize a Lock.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Lock::Lock(char* debugName)
{
    name = debugName;
    isBusy = false;     //lock is free
    queue = new List;
    owner = NULL;
}

//----------------------------------------------------------------------
// Lock::Lock
// 	De-allocate Lock, when no longer needed.
//----------------------------------------------------------------------

Lock::~Lock()
{
    delete queue;
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
// true if the current thread holds this lock.  Useful for
// checking in Release, and in Condition variable ops below.
//----------------------------------------------------------------------

bool Lock::isHeldByCurrentThread()
{
    if (owner == currentThread && isBusy)
        return true;
    else
        return false;
}

//----------------------------------------------------------------------
// Lock::Acquire
// wait until the lock is FREE, then set it to BUSY
//----------------------------------------------------------------------

void Lock::Acquire()
{
    ASSERT(!isHeldByCurrentThread());   // prevent lock holder from 
                                  // acquiring the lock a second time
    
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    while(isBusy) //if Lock is busy
    {
        queue->Append((void *) currentThread);  //so go to sleep
        currentThread->Sleep();
    }
    isBusy = true;  //set the Lock to busy
    owner = currentThread;

    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Lock::Release
// release the lock
// if any threads is waiting the Lock and current thread is the
// owner of the Lock ,then wake up the first thread
//----------------------------------------------------------------------

void Lock::Release()
{
    Thread* thread;
    ASSERT(isHeldByCurrentThread());    //ensure current thread
                                        //is the owner of the Lock
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    thread = (Thread *)queue->Remove();
    if(thread != NULL)  //wake up a thread
    {
        scheduler->ReadyToRun(thread);
    }
    isBusy = false;
    owner = NULL;

    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Condition::Condition
// 	Initialize a Condition
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Condition::Condition(char* debugName)
{
    name = debugName;
    queue = new List;
}

//----------------------------------------------------------------------
// Condition::Condition
//  De-allocate Condition, when no longer needed.
//----------------------------------------------------------------------

Condition::~Condition()
{
    delete queue;
}

//----------------------------------------------------------------------
// Condition::Wait
//  if thread is waiting condition variable, then release the Lock
//  and reacquire the Lock
//----------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock)
{
    if (mutex == NULL)
        mutex = conditionLock;
    else
        ASSERT(mutex == conditionLock);

    ASSERT(conditionLock->isHeldByCurrentThread());
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    conditionLock->Release();
    queue->Append((void *)currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();

    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Condition::Signal
//  if queue is not empty, then wake up the next thread
//----------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock)
{
    if (mutex == NULL)
        mutex = conditionLock;
    else
        ASSERT(mutex == conditionLock);

    Thread* thread;
    ASSERT(conditionLock->isHeldByCurrentThread());
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    thread = (Thread *)queue->Remove();
    if(thread != NULL)  //wake up the next thread
        scheduler->ReadyToRun(thread);

    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Condition::Broadcast
//  wake up all threads
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock)
{
    while (!queue->IsEmpty()) {
        Signal(conditionLock);
    }
}
