#include "synch.h"

class BoundedBuffer {
   public:
     // create a bounded buffer with a limit of 'maxsize' bytes
     BoundedBuffer(int max_size);

     //De-allocate BoundedBuffer, when no longer needed.  Assume no        
     //one is still waiting on the semaphore!
     ~BoundedBuffer();

     // read 'size' bytes from the bounded buffer, storing into 'data'.
     // ('size' may be greater than 'maxsize')
     void Read(void *data, int size);
     
     // write 'size' bytes from 'data' into the bounded buffer.
     // ('size' may be greater than 'maxsize')
     void Write(void *data, int size);

     // Determine whether the buffer is empty
     bool IsEmpty();

     // Determine whether the buffer is full
     bool IsFull();

     // Use for debug,print  the contents of the buffer
     void PrintBuffer(); 


   private:
     int   in, out, maxsize; 
     int  *buffer;
     int count; // Record the number of numbers in the buffer

     Lock  *lock;
     Condition *WriteFull, *ReadEmpty; 
     
};

