#include "copyright.h"
#include "EventBarrier.h"
#include "system.h"
#include "Elevator.h"
#include "synch.h"



//----------------------------------------------------------------------
// Elevator::Elevator
//----------------------------------------------------------------------
Elevator::Elevator(char *debugName, int numFloors, int myID)
{
    int i;
    char *s1,*s2,*s3;
    s1 = new char[20];
    s2 = new char[20];
    s3 = new char[20];
    topFloor = numFloors;
    occupancy = 0;
    currentfloor = 1;
    elevatorState = STAY; //means stay
    elevatorID = myID;
    capacity = 1;
    name = debugName;
    ElevatorLock = new Lock("ElevatorLock");
    HaveRequest = new Condition("HaveRequest");
    ElevatorNotFull = new Condition("ElevatorNotFull");
    for (i=1;i<=numFloors;i++)
    {
        sprintf( s1 , "upRequest-%d" , i );
        sprintf( s2 , "downRequest-%d" , i );
        sprintf( s3 , "outRequest-%d" , i );
        upRequest[i] = new EventBarrier(s1);
        downRequest[i] = new EventBarrier(s2);
        outRequest[i] = new EventBarrier(s3);
    }
}

//----------------------------------------------------------------------
// Elevator::~Elevator
//----------------------------------------------------------------------
Elevator::~Elevator()
{
    int i;
    for (i=1;i<=topFloor;i++)
    {
        delete upRequest[i];
        delete downRequest[i];
        delete outRequest[i];
    }
    delete ElevatorLock;
    delete HaveRequest;
    delete ElevatorNotFull;
}

//----------------------------------------------------------------------
// Elevator::OpenDoors
//----------------------------------------------------------------------
void Elevator::OpenDoors()
{
    alarms->Pause(TICK);
    printf("--Now elevator open door at %d floor with %d riders\n", currentfloor, occupancy);
    if(isOut[currentfloor])
        outRequest[currentfloor]->Signal();
    if(elevatorState == UP && isUp[currentfloor])
        upRequest[currentfloor]->Signal();
    if(elevatorState == DOWN && isDown[currentfloor])
        downRequest[currentfloor]->Signal();
    currentThread->Yield();
}

//----------------------------------------------------------------------
// Elevator::CloseDoors
//----------------------------------------------------------------------
void Elevator::CloseDoors()
{
    alarms->Pause(TICK);
    if( occupancy == 0 )
		printf("**** The Elevator is empty ****\n");
    printf("--Now elevator close door at %d floor with %d riders\n", currentfloor, occupancy);
    if (elevatorState == UP)
        isUp[currentfloor] = false;
    else if (elevatorState == DOWN)
        isDown[currentfloor] = false;
    isOut[currentfloor] = false;
}

//----------------------------------------------------------------------
// Elevator::VisitFloor
//----------------------------------------------------------------------
void Elevator::VisitFloor(int floor)
{
    ElevatorLock->Acquire();
    ElevatorNotFull->Broadcast(ElevatorLock);
    ElevatorLock->Release();
}

//----------------------------------------------------------------------
// Elevator::Enter
//----------------------------------------------------------------------
bool Elevator::Enter()
{
    if(occupancy < capacity)
    {
        occupancy++;
        alarms->Pause(TICK);
        if(elevatorState == UP)
        {
            upRequest[currentfloor]->Complete();
            isUp[currentfloor] = false;
        }
        else if(elevatorState == DOWN)
        {
            downRequest[currentfloor]->Complete();
            isDown[currentfloor] = false;
        }
        return true;
    }
    else
    {
        printf("The Elevator is full. Please wait for next turn.\n");
        if(elevatorState == UP)
        {
            upRequest[currentfloor]->Complete();
            isUp[currentfloor] = false;
        }
        else if(elevatorState == DOWN)
        {
            downRequest[currentfloor]->Complete();
            isDown[currentfloor] = false;
        }
        ElevatorLock->Acquire();
        ElevatorNotFull->Wait(ElevatorLock);
        ElevatorLock->Release();
        return false;
    }

}

//----------------------------------------------------------------------
// Elevator::Exit
//----------------------------------------------------------------------
void Elevator::Exit()
{
    alarms->Pause(TICK);
    occupancy--;
	outRequest[currentfloor]->Complete();
	isOut[currentfloor] = false;
/*	if( occupancy == 0 )
	{
		printf("**** The Elevator is empty ****\n");
		//elevatorState = STAY;
	}
*/
}

//----------------------------------------------------------------------
// Elevator::RequestFloor
//----------------------------------------------------------------------
void Elevator::RequestFloor(int floor)
{
    isOut[floor] = true;
    outRequest[floor]->Wait();
}

//----------------------------------------------------------------------
// Elevator::getRequest
//     Go through the EventBarrier and find a request.
//----------------------------------------------------------------------
int Elevator::getRequest()
{
    ElevatorLock->Acquire();
    int request = -1;
    int i;

    if( elevatorState == STAY )
    {
        for(i = 1; i <= topFloor; i++)
        {
            if(isUp[i] || isDown[i] || isOut[i])
            {
                request = i;
                break;
            }
        }
    }
    else if( elevatorState == UP )
    {
        for(i = topFloor; i >= currentfloor; i--)
        {
            if(isUp[i] || isDown[i] || isOut[i])
            {
                request = i;
                break;
            }
        }
    }
    else
    {
        for(i = 1; i <= currentfloor; i++)
        {
            if(isUp[i] || isDown[i] || isOut[i])
            {
                request = i;
                break;
            }
        }
    }
    ElevatorLock->Release();
    return request;
}


//----------------------------------------------------------------------
// Elevator::goUp
//----------------------------------------------------------------------
int Elevator::goUp()
{
    alarms->Pause(TICK);
    currentfloor++;
    return currentfloor;
}

//----------------------------------------------------------------------
// Elevator::goDown
//----------------------------------------------------------------------
int Elevator::goDown()
{
    alarms->Pause(TICK);
    currentfloor--;
	return currentfloor;
}

//----------------------------------------------------------------------
// Building::Building
//----------------------------------------------------------------------
Building::Building(char *debugname, int numFloors, int numElevators)
{
    name = debugname;
    topfloors = numFloors;
    elevator = new Elevator("elevator",numFloors,1);
}

//----------------------------------------------------------------------
// Building::~Building
//----------------------------------------------------------------------
Building::~Building()
{
    delete elevator;
}

//----------------------------------------------------------------------
// Building::CallUp
//----------------------------------------------------------------------
void Building::CallUp(int fromFloor)
{
    elevator->isUp[fromFloor] = true;
    elevator->ElevatorLock->Acquire();
    elevator->HaveRequest->Signal(elevator->ElevatorLock);
    elevator->ElevatorLock->Release();
}

//----------------------------------------------------------------------
// Building::CallDown
//----------------------------------------------------------------------
void Building::CallDown(int fromFloor)
{
    elevator->isDown[fromFloor] = true;
    elevator->ElevatorLock->Acquire();
    elevator->HaveRequest->Signal(elevator->ElevatorLock);
    elevator->ElevatorLock->Release();
}

//----------------------------------------------------------------------
// Building::AwaitUp
//----------------------------------------------------------------------
Elevator *Building::AwaitUp(int fromFloor)
{
    elevator->upRequest[fromFloor]->Wait();
    return elevator;
}

//----------------------------------------------------------------------
// Building::AwaitDown
//----------------------------------------------------------------------
Elevator *Building::AwaitDown(int fromFloor)
{
    elevator->downRequest[fromFloor]->Wait();
    return elevator;
}
