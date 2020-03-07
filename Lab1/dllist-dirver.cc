// dllist-dirver.cc 
//	Simple test case for concurrent errors of doubly-linked list.
//
//  TypeⅠ error:
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//  TypeⅡ error:
//  It's the same as the first, except for the timing of
//  context switch.
// 
// Copyright (c) 2020 Marx Young. All rights reserved.

#include "dllist.h"
#include "system.h"

//----------------------------------------------------------------------
// GenerateN
// 	generates N items with random keys and inserts them into a 
//  doubly-linked list
//----------------------------------------------------------------------

void 
GenerateN(int N, DLList *list) {
    while (N--) {
        int key = Random() % 2001 - 1000;   // here we limit the range  
                                // of random numbers to [ -1000, 1000 ]
                                // just for the convenience of demonstration
        list->SortedInsert(NULL, key);
        printf("Insert an item which key is %d\n", key);
    }
}

//----------------------------------------------------------------------
// RemoveN
// 	removes N items starting from the head of the list and 
//  prints out the removeditems to the console
//----------------------------------------------------------------------

void 
RemoveN(int N, DLList *list) {
    int key;
    while (N--) {
        list->Remove(&key);
        printf("Remove an item which key is %d\n", key);
    }
}

