// dllist.cc
//      Routines to manage a doubly-linked list.
//
// 	    A "DLLElement" is allocated for each item to be put on the
//	    list; it is de-allocated when the item is removed.
//
// Copyright (c) 2020 Marx Young. All rights reserved.

// extern "C" {
// #include <assert.h>

// #define ASSERT(expression)  assert(expression)
// }

#include "copyright.h"
#include "dllist.h"
#include "system.h"

// const int NULL = 0;

// The following class defines a "list element" -- which is
// used to keep track of one item on a list.
//
// Class defined in dllist.cc, because only the DLList class
// can be allocating and accessing DLLElements.

class DLLElement
{
public:
    DLLElement(void *itemPtr, int sortKey); // initialize a list element

    DLLElement *next; // next element on list
                      // NULL if this is the last
    DLLElement *prev; // previous element on list
                      // NULL if this is the first
    int key;          // priority, for a sorted list
    void *item;       // pointer to item on the list
};

//----------------------------------------------------------------------
// DLLElement::DLLElement
//	    Initialize an element.
//----------------------------------------------------------------------

DLLElement::DLLElement(void *itemPtr, int sortKey)
{
    next = prev = NULL;
    item = itemPtr;
    key = sortKey;
}

//----------------------------------------------------------------------
// DLList::DLList
//	    Initialize a list, empty to start with.
//	    Elements can now be added to the list.
//----------------------------------------------------------------------

DLList::DLList()
{
    first = last = NULL;
    err_type = -1;
    lock = new Lock("list lock");
    listEmpty = new Condition("list empty cond");
}

DLList::DLList(int err_type)
{
    first = last = NULL;
    this->err_type = err_type;
    lock = new Lock("list lock");
    listEmpty = new Condition("list empty cond");
}

//----------------------------------------------------------------------
// DLList::~DLList
//	    Prepare a list for deallocation. If the list still contains any
//	    DLLElements, de-allocate them.
//----------------------------------------------------------------------

DLList::~DLList()
{
    while (!IsEmpty())
        Remove(NULL); // delete all the list elements
    delete lock;
    delete listEmpty;
}

//----------------------------------------------------------------------
// DLList::Prepend
//      Put item at the the head of the list.
//
//	    Allocate a DLLElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//
//	    "value" is the pointer of the item to be put on the list.
//----------------------------------------------------------------------

void
DLList::Prepend(void *value)
{
    lock->Acquire();    //enforce mutual exclusive access to the list
    if (IsEmpty())
    { // list is empty, set key = 0
        DLLElement *element = new DLLElement(value, 0);
        first = element;
        last = element;
    }
    else
    { // else add to head of list (set key = min_key-1)
        DLLElement *element = new DLLElement(value, first->key - 1);
        element->next = first;
        element->prev = NULL;
        first->prev = element;
        first = element;
    }
    listEmpty->Signal(lock);    // wake up a waiter, if any
    lock->Release();
}

//----------------------------------------------------------------------
// DLList::Append
//      Put item at the the end of the list.
//
//	    Allocate a DLLElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//
//	    "value" is the pointer of the item to be put on the list.
//----------------------------------------------------------------------

void
DLList::Append(void *value)
{
    lock->Acquire();    //enforce mutual exclusive access to the list
    if (IsEmpty())
    { // list is empty, set key = 0
        DLLElement *element = new DLLElement(value, 0);
        first = element;
        last = element;
    }
    else
    { // else add to tail of list (set key = max_key+1)
        DLLElement *element = new DLLElement(value, last->key + 1);
        element->next = NULL;
        element->prev = last;
        last->next = element;
        last = element;
    }
    listEmpty->Signal(lock);    // wake up a waiter, if any
    lock->Release();
}

//----------------------------------------------------------------------
// DLList::Remove
//      Remove an item from head of list.
//      Set *keyPtr to key of the removed item.
//
// Returns:
//      item (or NULL if list is empty)
//----------------------------------------------------------------------

void *
DLList::Remove(int *keyPtr)
{
    lock->Acquire();    //enforce mutual exclusive access to the list
    while (IsEmpty())
        listEmpty->Wait(lock);

    DLLElement *element = first;
    if (err_type == 4)
        currentThread->Yield();
    if (keyPtr)
        *keyPtr = element->key;
    void *item = element->item;
    ASSERT(item != NULL);
    first = first->next;
    if (first == NULL) {
        last = NULL;
    }

    delete element; // deallocate list element -- no longer needed
    lock->Release();
    return item;
}

//----------------------------------------------------------------------
// DLList::IsEmpty
//      Returns TRUE if the list is empty (has no items).
//----------------------------------------------------------------------

bool DLList::IsEmpty()
{
    return (first == NULL);
}

//----------------------------------------------------------------------
// DLList::SortedInsert
//      Put items on list in order (sorted by key).
//----------------------------------------------------------------------

void DLList::SortedInsert(void *item, int sortKey)
{
    DLLElement *element = new DLLElement(item, sortKey);

    lock->Acquire();    //enforce mutual exclusive access to the list
    if (IsEmpty())
    { // list is empty
        if (err_type == 2)
            currentThread->Yield();
        first = element;
        if (err_type == 3)
            currentThread->Yield();
        last = element;
    }
    else
    { // else put it at the correct position
        if (sortKey <= first->key)
        {                          // new key is the smallest of all
            element->next = first; // put it before first
            element->prev = NULL;
            first->prev = element;
            if (err_type == 5)
                currentThread->Yield();
            first = element;
        }
        else if (sortKey >= last->key)
        {                         // new key is the biggest of all
            element->next = NULL; // put it after last
            element->prev = last;
            last->next = element;
            if (err_type == 5)
                currentThread->Yield();
            last = element;
        }
        else
        { // neither the first nor the last
            DLLElement *e = first->next;
            while (element->key > e->key) // loop till e points to element
                e = e->next;              // following the correct position
            if (err_type == 6)
                currentThread->Yield();
            e->prev->next = element;
            element->prev = e->prev;
            element->next = e;
            e->prev = element;
        }
    }
    listEmpty->Signal(lock);    // wake up a waiter, if any
    lock->Release();
}

//----------------------------------------------------------------------
// DLList::SortedRemove
//      Remove first item with key == sortKey.
// Returns:
//	    item (or NULL if no such item exists)
//----------------------------------------------------------------------

void *
DLList::SortedRemove(int sortKey)
{
    lock->Acquire();    //enforce mutual exclusive access to the list
    while (IsEmpty())
        listEmpty->Wait(lock);

    DLLElement *element = first;
    while (element && sortKey > element->key)
        element = element->next;
    if (element && sortKey == element->key)
    {
        element->prev->next = element->next;
        element->next->prev = element->prev;
        void *item = element->item;
        ASSERT(item != NULL);
        delete element;
        lock->Release();
        return item;
    }
    lock->Release();
    return NULL;
}

void
DLList::PrintList()
{
    if(IsEmpty())
        return;
    DLLElement *element = first;
    printf("-----------List-----------\n");
    while(element)
    {
        printf("%d ",element->key);
        element = element->next;
    }
    printf("\n--------------------------\n");
}
