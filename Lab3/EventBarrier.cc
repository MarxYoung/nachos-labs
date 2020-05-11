// EventBarrier.cc 
//
// Copyright (c) 2020 MarxYoung.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "EventBarrier.h"

//----------------------------------------------------------------------
// EventBarrier::EventBarrier
//----------------------------------------------------------------------

EventBarrier::EventBarrier(char *debugName)
{
    waitersCnt = 0;
    state = false;
    signalLock = new Lock(debugName);
    signalCond = new Condition(debugName);
    completeLock = new Lock(debugName);
    completeCond = new Condition(debugName);
}

//----------------------------------------------------------------------
// EventBarrier::~EventBarrier
//----------------------------------------------------------------------

EventBarrier::~EventBarrier()
{
    delete signalLock;
    delete signalCond;
    delete completeLock;
    delete completeCond;
}

//----------------------------------------------------------------------
// EventBarrier::EventBarrier
//----------------------------------------------------------------------

void
EventBarrier::Wait()
{
    waitersCnt++;
    if (state)
        return;
    signalLock->Acquire();
    signalCond->Wait(signalLock);
    signalLock->Release();
}

//----------------------------------------------------------------------
// EventBarrier::EventBarrier
//----------------------------------------------------------------------

void
EventBarrier::Signal()
{
    // signalLock->Acquire();
    signalLock->Acquire();
    state = true;       // set EventBarrier to signaled state
    signalCond->Broadcast(signalLock);

    completeLock->Acquire();
    completeCond->Wait(completeLock);
    completeLock->Release();

    state = false;      // EventBarrier reverts to unsignaled state
    signalLock->Release();
    // signalLock->Release();
}

//----------------------------------------------------------------------
// EventBarrier::EventBarrier
//----------------------------------------------------------------------

void
EventBarrier::Complete()
{
    if (--waitersCnt != 0) 
    {
        completeLock->Acquire();
        completeCond->Wait(completeLock);
        completeLock->Release();
    }
    else 
    {
        completeLock->Acquire();
        completeCond->Broacast(completeLock);
        completeLock->Release();
    }
}

//----------------------------------------------------------------------
// EventBarrier::EventBarrier
//----------------------------------------------------------------------

int
EventBarrier::Waiters()
{
    return waitersCnt;
}