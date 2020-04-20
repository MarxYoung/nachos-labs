#include "BoundedBuffer.h"
#include "system.h"


//----------------------------------------------------------------------
// BoundedBuffer::BoundedBuffer
//	    Initialize a BoundedBuffer.
//----------------------------------------------------------------------
BoundedBuffer::BoundedBuffer(int max_size)
{
    maxsize = max_size;
    buffer = new int[max_size] ;
    in = out = 0;
    count = 0;
    lock = new Lock("bufferLock");
    WriteFull = new  Condition("WriteFull");
    ReadEmpty = new Condition("ReadEmpty");
    
}


//----------------------------------------------------------------------
// BoundedBuffer::~BoundedBuffer
//	   Destroy a BoundedBuffer.
//----------------------------------------------------------------------
BoundedBuffer::~BoundedBuffer()
{
    delete buffer;
    delete lock;
    delete WriteFull;
    delete ReadEmpty;
}


//----------------------------------------------------------------------
//BoundedBuffer::IsEmpty
//	Determine whether the buffer is empty
//----------------------------------------------------------------------
bool BoundedBuffer::IsEmpty()
{
	return (count == 0);
}


//----------------------------------------------------------------------
//BoundedBuffer::IsFull
//	Determine whether the buffer is full
//----------------------------------------------------------------------
bool BoundedBuffer::IsFull()
{
	return (count == maxsize);
}


//----------------------------------------------------------------------
//BoundedBuffer::Read
//	Read 'size' bytes from buffer and add them to '*data' .
//      Read one character at a time into Space starting from *data 
//      with locks and condition variables.
//      'out' is the location of the first character in the current buffer
//----------------------------------------------------------------------
void  BoundedBuffer::Read(void *data, int size)
{
    int i,j;
    int flag = 0;
    int *s = (int *)data;
    lock -> Acquire();
    for(i = 0; i < size; i++)
    {
        if ( IsEmpty() )
            {
                PrintBuffer();
                printf("%s's data is:",currentThread -> getName());
                for(j = 0; j < i ; j++)
                    printf("%d ",*(s + j));
                printf("\n\n");
            }
        while( IsEmpty() )
            {
                ReadEmpty -> Wait(lock);
                flag = 1;
            }
        if ( flag == 1 )	
            {
                printf("Context switch to  %s \n", currentThread -> getName());
                flag = 0;
            }
        *(s + i) = *(buffer + out);
        out = (out + 1) % maxsize;
        count--;
        WriteFull -> Signal(lock);
    }
    lock -> Release();
}



//----------------------------------------------------------------------
//BoundedBuffer::Write
//	Write 'size' bytes from '*data' to buffer,
//      write one character at a time into the buffer 
//      with locks and condition variables
//       'in' is the location of the last character in the current buffer
//----------------------------------------------------------------------
void BoundedBuffer::Write(void *data, int size)
{
     int *s = (int *)data;
     int i;
     int flag = 0;
     lock -> Acquire();
     for(i = 0; i < size; i++)
    {
        if ( IsFull() )
            {
                PrintBuffer();
            }
        while( IsFull() )
            {
                WriteFull -> Wait(lock);
                flag = 1;
            }
        if ( flag == 1 )	
            {
                printf("Context switch to  %s \n", currentThread -> getName());
                flag = 0;
            }
        *(buffer + in) = *(s + i);
        in = (in + 1) % maxsize;
        count++;
        ReadEmpty -> Signal(lock);
    }
    lock -> Release();
}



//----------------------------------------------------------------------
//BoundedBuffer::PrintBuffer
//	Print the contents of the current buffer.
//      Just use for debugging.
//----------------------------------------------------------------------
void BoundedBuffer::PrintBuffer()
{
    int i = out;
    int num =count;
    printf("Current buffer is :");
    while(num--)
    {
        printf("%d ", *(buffer+i));
        i = (i + 1) % maxsize;
    }
    printf("\n\n");
}


