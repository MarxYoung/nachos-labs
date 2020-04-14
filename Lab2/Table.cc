# include "Table.h"

//----------------------------------------------------------------------
// Table::Table
// 	Initialize a Table
//----------------------------------------------------------------------
Table::Table(int size)
{
	this->size = size;
    elem = new void *[this->size + 1]();
	lock = new Lock("TableLock");
	
}

Table::~Table()
{
	delete elem;
    delete lock;
}

int Table::Alloc(void* object)
{
    int index = -1;

    if(object == NULL)
        return index;
	lock->Acquire();
	for(int i = 0; i < size; i++)
	{
		if(elem[i] == NULL)
		{
			elem[i] = object;
			index= i;
			break;
		}
	}
	lock->Release();
	return index;
}

void* Table::Get(int index)
{
	ASSERT(index >= 0 && index < size);
	return elem[index];
}

void Table::Release(int index)
{
	ASSERT(index >= 0 && index < size);
	elem[index] = NULL;
}
