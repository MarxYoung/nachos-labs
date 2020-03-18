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
int T, N, E;
DLList *list;

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

//----------------------------------------------------------------------
// ConcurrentError2
// 	switch threads before setting list->first when inserting an item
//  into an empty list
//----------------------------------------------------------------------

void
ConcurrentError2(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
}

//----------------------------------------------------------------------
// ConcurrentError3
// 	switch threads after setting list->first when inserting an item
//  into an empty list
//----------------------------------------------------------------------

void
ConcurrentError3(int which)
{
    int key[] = {1,2};  // 2rd thread's item's key > 1st thread's item's key
                        // so that a segment fault will occur in
                        // "else if (sortKey >= last->key)"(DLList::SortedInsert)
    printf("*** thread %d is going to insert an item with key: %d\n", which, key[which % 2]);
    list->SortedInsert(NULL, key[which % 2]);
    printf("*** thread %d\n", which);
    RemoveN(1, list);
}

//----------------------------------------------------------------------
// ConcurrentError4
//
//----------------------------------------------------------------------

void
ConcurrentError4(int which)
{
    ConcurrentError2(which);
}

//----------------------------------------------------------------------
// ConcurrentError5
//
//----------------------------------------------------------------------

void
ConcurrentError5(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(1, list);
}

//----------------------------------------------------------------------
// ConcurrentError6
//   switch threads before setting first = element if item
//   which inserted is the smallest
//----------------------------------------------------------------------
void
ConcurrentError6(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
}

//----------------------------------------------------------------------
// ConcurrentError7
//   switch threads before setting last = element if item
//   which inserted is the biggest
//---------------------------------
//----------------------------------------------------------------------
void
ConcurrentError7(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
}

//----------------------------------------------------------------------
// ConcurrentError8
//
//----------------------------------------------------------------------
void
ConcurrentError8(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
}

//----------------------------------------------------------------------
// ConcurrentError9
//
//----------------------------------------------------------------------
void
ConcurrentError9(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
}

//----------------------------------------------------------------------
// ConcurrentError10
//
//----------------------------------------------------------------------
void
ConcurrentError10(int which)
{
    printf("*** thread %d\n", which);
    GenerateN(N, list);
    currentThread->Yield();
    printf("*** thread %d\n", which);
    RemoveN(N, list);
}

const int error_num = 10;    // total number of concurrent errors
typedef void (*func) (int);
func ConcurrentErrors[error_num] = {ConcurrentError1, ConcurrentError2, ConcurrentError3,
                                    ConcurrentError4, ConcurrentError5, ConcurrentError6,
                                    ConcurrentError7, ConcurrentError8, ConcurrentError9,
                                    ConcurrentError10};

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
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");

    // problem with parameter E
    if (E > error_num) {
        printf("No concurrent error specified.\n");
        return;
    }

    list = new DLList(E);
    int i;

    for (i = 1; i < T; i++) {
        Thread *t = new Thread("forked thread");
        t->Fork(ConcurrentErrors[E - 1], i);
    }
    ConcurrentErrors[E - 1](0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest(int t, int n, int e)
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
    T = t;
    N = n;
    E = e;
    ThreadTest2();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

