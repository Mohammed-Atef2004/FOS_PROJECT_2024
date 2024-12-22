// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	struct semaphore sem;
	sem.semdata =(struct __semdata*)smalloc(semaphoreName,sizeof(struct __semdata),1);
	if(sem.semdata!=NULL)
	{
		sem.semdata->count =value;
		strcpy(sem.semdata->name,semaphoreName);
		LIST_INIT(&sem.semdata->queue);
		sem.semdata->lock =0;
	}
	return sem;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	struct semaphore sem;
	sem.semdata= (struct __semdata*)sget(ownerEnvID,semaphoreName);
	return sem;
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	uint32 key=1;
	while(xchg(&(sem.semdata->lock), key) != 0);
	sem.semdata->count--;
	if(semaphore_count(sem)<0)
	{
		sys_semaphore_operation(&sem.semdata->queue,&sem.semdata->lock,2);
	}
	sem.semdata->lock=0;
}

void signal_semaphore(struct semaphore sem)
{
	// TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("signal_semaphore is not implemented yet");
	// Your Code is Here...
	uint32 key=1;
	while(xchg(&(sem.semdata->lock), key) != 0) ;

	sem.semdata->count++;
	if(semaphore_count(sem) <= 0)
	{
		sys_semaphore_operation(&sem.semdata->queue,&sem.semdata->lock,1);
	}
	sem.semdata->lock=0;

}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
