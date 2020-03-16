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
#include "dllist.h"

extern void GenerateN(int N, DLList *list);
extern void RemoveN(int N, DLList *list);

// testnum is set in main.cc
int testnum = 1;
int N;
DLList *list = new DLList();

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
// ConcurrentError1
// 	insert N items -- switch threads -- remove N items -- switch threads
//----------------------------------------------------------------------

void
ConcurrentError1(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
    currentThread->Yield();
}

const int error_num = 1;    // total number of concurrent errors
typedef void (*func) (int);
func ConcurrentErrors[error_num] = {ConcurrentError1};

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
// ThreadTest2
// 	
//----------------------------------------------------------------------

void
ThreadTest2(int T, int E)
{
    DEBUG('t', "Entering ThreadTest2");

    // problem with parameter E
    if (E >= error_num) {
        printf("No concurrent error specified.\n");
        return;
    }

    int i;
    
    for (i = 1; i < T; i++) {
        Thread *t = new Thread("forked thread");
        t->Fork(ConcurrentErrors[E], i);
    }
    ConcurrentErrors[E](0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest(int T, int n, int E)
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
    N = n;
    ThreadTest2(T, E);
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

