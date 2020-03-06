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

// 'Cause <stdlib.h> conflicts with nachos, we use this sequence
int rand_sequence[100] = {-27,9,-51,-87,43,15,31,-91,-87,-94,-46,-61,-35,-17,4,41,48,65,43,45,-18,90,-36,-64,0,-18,-68,43,-97,-51,37,-69,4,-11,23,10,-35,48,91,-85,81,95,25,-48,-61,-24,55,44,-14,18,96,-9,54,16,-31,2,-87,-47,25,-90,78,9,-46,-45,84,-50,0,85,78,-14,-58,-39,70,1,-42,-62,-56,-44,79,15,-83,23,57,-14,-16,-9,5,3,-4,-19,-4,-4,47,-8,25,14,9,-64,-77,-24};
int rand_index = 0;

//----------------------------------------------------------------------
// Rand
// 	return a "random number" in [-100, 100]
//----------------------------------------------------------------------

int 
Rand() {
    rand_index = rand_index % 100;
    return rand_sequence[rand_index++];
}

//----------------------------------------------------------------------
// GenerateN
// 	generates N items with random keys and inserts them into a 
//  doubly-linked list
//----------------------------------------------------------------------

void 
GenerateN(int N, DLList *list) {
    while (N--) {
        int key = Rand();
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

