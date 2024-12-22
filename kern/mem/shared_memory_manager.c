#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//	panic("create_frames_storage is not implemented yet");
	//Your Code is Here...
	struct FrameInfo** frames_storage=(struct FrameInfo** )kmalloc(numOfFrames*sizeof(struct FrameInfo*));
	if(frames_storage!=NULL){
		for(int i=0;i<numOfFrames;i++)
		{
			frames_storage[i]=0;
		}
		return frames_storage;
	}
	else
	{
		return NULL;
	}
}
//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//	panic("create_share is not implemented yet");
	//Your Code is Here...

	struct Share* share= (struct Share*)kmalloc(sizeof(struct Share));
	uint32 va=(uint32)share ;
	if((uint32*)va==NULL)
		return NULL;
	int num_of_frames=(ROUNDUP(size,PAGE_SIZE))/(PAGE_SIZE);
	struct FrameInfo** frames_storage =create_frames_storage(num_of_frames);
	// check if frames_storage and allocate returned address or not
	if(frames_storage!=NULL)
	{
		share->ID=va & 0x7FFFFFFF; //mask the MSB of va to make it +ve
		share->ownerID=ownerID;
		strcpy(share->name,shareName);
		share->isWritable=isWritable;
		share->references=1;
		share->framesStorage=frames_storage;
		share->size=size;
		share->prev_next_info.le_prev=NULL;
		share->prev_next_info.le_next=NULL;
		return share;
	}
	else
	{
		//undo any allocation
		kfree((uint32*)va);
		return NULL;
	}
}
//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("get_share is not implemented yet");
	//Your Code is Here...
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held)
	{
		acquire_spinlock(&AllShares.shareslock);
	}
	struct Share* requiredObject;
	int found =0;
	LIST_FOREACH(requiredObject,&AllShares.shares_list)
	{
		if((requiredObject->ownerID==ownerID) && (strcmp(requiredObject->name,name)==0))
		{
			found=1;
			break;
		}
	}
	if (!lock_already_held)
	{
		release_spinlock(&AllShares.shareslock);
	}
	if(found){
		return requiredObject;
	}
	else{
		return NULL;
	}
}
//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//	panic("createSharedObject is not implemented yet");
	//Your Code is Here...
	struct Env* myenv = get_cpu_proc(); //The calling environment
	struct Share* newObject= get_share(ownerID,shareName);
	// check if the object exists before
	if(newObject!=NULL)
	{
		return E_SHARED_MEM_EXISTS;
	}
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held)
	{
		acquire_spinlock(&AllShares.shareslock);
	}
	newObject= create_share(ownerID,shareName,size,isWritable);
	if(newObject==NULL)
	{
		if (!lock_already_held)
		{
			release_spinlock(&AllShares.shareslock);
		}
		return E_NO_SHARE;
	}
	LIST_INSERT_TAIL(&AllShares.shares_list,newObject);
	uint32 numberOfPages=ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	struct FrameInfo *newframe;
	uint32 va=(uint32)virtual_address;
	for(int i=0;i<numberOfPages;i++)
	{
		newframe=NULL;
		int allocResult= allocate_frame(&newframe);
		if(allocResult==0)
		{
			int mapResult=map_frame(myenv->env_page_directory,newframe,(uint32)va,PERM_PRESENT|PERM_WRITEABLE|PERM_USER);
			if(mapResult!=0)
			{
				free_frame(newframe);
				va-=PAGE_SIZE;
				for(int j=0;j<i-1;j++)
				{
					unmap_frame(myenv->env_page_directory,va);
					va-=PAGE_SIZE;
				}
				if (!lock_already_held)
					{
						release_spinlock(&AllShares.shareslock);
					}
				return E_NO_SHARE;
			}
			newframe->Shared_id=newObject->ID;
		}
		else
		{
			va-=PAGE_SIZE;
			for(int j=0;j<i-1;j++)
			{
				unmap_frame(myenv->env_page_directory,va);
				va-=PAGE_SIZE;
			}
			if (!lock_already_held)
			{
				release_spinlock(&AllShares.shareslock);
			}
			return E_NO_SHARE;
		}
		newObject->framesStorage[i]=newframe;
		va+=PAGE_SIZE;
	}
	int id=newObject->ID;
	if (!lock_already_held)
	{
		release_spinlock(&AllShares.shareslock);
	}
	return id;
}
//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//	panic("getSharedObject is not implemented yet");
	//Your Code is Here...
	struct Env* myenv = get_cpu_proc(); //The calling environment
	struct Share* shareObject=get_share(ownerID,shareName);
	if(shareObject==NULL)
	{
		return E_SHARED_MEM_NOT_EXISTS;
	}
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held)
	{
		acquire_spinlock(&AllShares.shareslock);
	}
	uint32 numberOfPages=ROUNDUP((shareObject->size),PAGE_SIZE)/PAGE_SIZE;
	uint32 va=(uint32)virtual_address;
	uint32 permission=0;
	struct FrameInfo *newframe;

	if(shareObject->isWritable)
	{
		permission|=PERM_PRESENT|PERM_WRITEABLE|PERM_USER;
	}
	else
	{
		permission|=PERM_PRESENT|PERM_USER;
	}
	for(int i=0;i<numberOfPages;i++)
	{
		newframe=shareObject->framesStorage[i];
		int mapResult=map_frame(myenv->env_page_directory,newframe,(uint32)va,permission);
		va+=PAGE_SIZE;
	}
	shareObject->references+=1;
	int id=shareObject->ID;
	if (!lock_already_held)
	{
		release_spinlock(&AllShares.shareslock);
	}
	return id;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//
//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_share is not implemented yet");
	//Your Code is Here...
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held)
	{
		acquire_spinlock(&AllShares.shareslock);
	}
	struct FrameInfo** frames=ptrShare->framesStorage;
	LIST_REMOVE(&AllShares.shares_list,ptrShare);
	if (!lock_already_held)
	{
		release_spinlock(&AllShares.shareslock);
	}
	kfree((void*)ptrShare);
	kfree((void*)frames);
}

//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
    //panic("freeSharedObject is not implemented yet");
	//Your Code is Here...
	struct Env* myenv = get_cpu_proc();
	struct Share* requiredObject;
	int found;
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held)
	{
		acquire_spinlock(&AllShares.shareslock);
	}
	LIST_FOREACH(requiredObject,&AllShares.shares_list)
	{
		if(sharedObjectID==requiredObject->ID)
		{
			found=1;
			break;
		}
	}
	if(found!=1)
	{
		if (!lock_already_held)
		{
			release_spinlock(&AllShares.shareslock);
		}
		return E_SHARED_MEM_NOT_EXISTS;
	}
	uint32 numOfPages= ROUNDUP(requiredObject->size,PAGE_SIZE)/PAGE_SIZE;
	uint32*table;
	uint32 ret;
	uint32 isEmpty=1;
	uint32 va=(uint32)startVA;
	for(int i=0;i<numOfPages;i++)
	{
		unmap_frame(myenv->env_page_directory,va);
		table=NULL;
		ret=get_page_table(myenv->env_page_directory,va,&table);
		if(table!=NULL)
		{
			for(int i=0;i<1024;i++)
			{
				if((table[i]&PERM_PRESENT))
				{
					isEmpty=0;
					break;
				}
			}
			if(isEmpty)
			{
				unmap_frame(myenv->env_page_directory,(uint32)table);//not sure
				pd_clear_page_dir_entry(myenv->env_page_directory,va);
			}
		}
		else{

			cprintf("table does not exist for virtual_address:%X\n",va);
		}
		isEmpty=1;
		va+=PAGE_SIZE;
	}
	requiredObject->references-=1;
	if(requiredObject->references==0)
	{
		free_share(requiredObject);
	}
	if (!lock_already_held)
	{
		release_spinlock(&AllShares.shareslock);
	}
	tlbflush();
	return 0;
}

