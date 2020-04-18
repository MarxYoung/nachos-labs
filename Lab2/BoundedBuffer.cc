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
void  BoundedBuffer::Read(void *data,int size)
{
    int i;
    int *s = (int *)data;
    lock -> Acquire();
    for(i =0;i<size;i++)
        {
	while(IsEmpty())
		ReadEmpty -> Wait(lock);
	*(s + i) = *(buffer +out);
	out = (out+1)%maxsize;
	count--;
	WriteFull -> Signal(lock);
        }
    *(s + i) = '\0'; 
    lock -> Release();
}



//----------------------------------------------------------------------
//BoundedBuffer::Write
//	Write 'size' bytes from '*data' to buffer,
//      write one character at a time into the buffer 
//      with locks and condition variables
//       'in' is the location of the last character in the current buffer
//----------------------------------------------------------------------
void BoundedBuffer::Write(void *data,int size)
{
     int *s = (int *)data;
     int i;
     lock -> Acquire();
     for(i=0;i<size;i++)
        {
	while(IsFull())
		WriteFull -> Wait(lock);
	*(buffer +in) = *(s + i);
	in = (in+1)%maxsize;
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
    printf("\nCurrent buffer is :");
    while(num--)
    {
	printf("%d ",*(buffer+i));
	i = (i + 1)%maxsize;
    }
    printf("\n");
}


