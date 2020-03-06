// dllist.h 
//	Data structures of ordered doubly linked list.  
//
// Copyright (c) 2020 Marx Young. All rights reserved.

#ifndef DLLIST_H
#define DLLIST_H

#include "copyright.h"

class DLLElement;

class DLList {
public:
  DLList(); // initialize the list
  ~DLList(); // de-allocate the list

  void Prepend(void *item); // add to head of list (set key = min_key-1)
  void Append(void *item); // add to tail of list (set key = max_key+1)
  void *Remove(int *keyPtr); // remove from head of list
                              // set *keyPtr to key of the removed item
                              // return item (or NULL if list is empty)

  bool IsEmpty(); // return true if list has elements

  // routines to put/get items on/off list in order (sorted by key)
  void SortedInsert(void *item, int sortKey);
  void *SortedRemove(int sortKey); // remove first item with key==sortKey
                                    // return NULL if no such item exists

private:
  DLLElement *first; // head of the list, NULL if empty
  DLLElement *last; // last element of the list, NULL if empty
};

#endif // DLLIST_H
