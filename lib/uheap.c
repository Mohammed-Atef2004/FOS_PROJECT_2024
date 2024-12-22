#include <inc/lib.h>
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

uint32 checker[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE] = { 0 };//check allocated or not (it's size = number of pages )

struct information
{
	uint32 piece_allocated_size;  // allocated size
	uint32 piece_allocated_va;   //allocated virtual address
	uint32 sharedID;
};
// struct carrying the info about each made allocation
struct information arrx[(USER_HEAP_MAX -USER_HEAP_START)/ PAGE_SIZE]={};

// carrying the whole allocated size in block allocator
uint32 total_allocated_blocks = 0;

// a variable storing the number of already allocated blocks
int allocated = 0;

void* malloc(uint32 size)
{
	//check for the size if illegal
	if (size <= 0 || size > DYN_ALLOC_MAX_SIZE)
	{
	        return NULL;
	}
	// Step 1: Small allocations (block allocator)
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
	{
		return alloc_block_FF(size);
	}

	// it it gets here it means it's a page and we use page allocator
	size = ROUNDUP(size, PAGE_SIZE);

	// we update the page array with the new allocated page size
	arrx[allocated].piece_allocated_size = size;

	// intialize some variable we need for the ff strategy
	int accumualted_size = 0; // size of continus free pages in traverse
	int ptr = 0;
	int alloc=0;

    // we start the page allocation from the page allocator start (after block aloc by 4k)
    uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;

    // start traversing the heap looking for a suitable address
	for (uint32 i =page_allocator_start ; i <= USER_HEAP_MAX; i += PAGE_SIZE)
	{
		// if we arrived at the heap end then it means there is no suitsble address and we stop
		if (i == USER_HEAP_MAX)
		{
			ptr=0;
			break;
		}

		//then we check for free pages  (unmarked pages)
		if (checker[(i - page_allocator_start) / PAGE_SIZE] == 0)
		{
			// if we encountered a marked block we zero the size1 and begin again
			if (accumualted_size == 0)
			{
				// we set the corresponding va and update the array with it
				arrx[allocated].piece_allocated_va = i;
			}
			// as long as the page is free we keep incrementing with the page size
			// until the acumulated size equlas the wanted size
			accumualted_size += PAGE_SIZE;
			if (accumualted_size == size)
			{
				ptr = 1;
				break;
			}
		}
		else
		{
			// if we encountered an allocated or marked page we reset the size to start again
			accumualted_size = 0;
		}
	}
	// we found a suitable space for the allocation / we mark it in the array
	if (ptr != 0)
	{
		// here we mark the pages in the corresponding checker array
		// we set it to 1
		for (uint32 i = arrx[allocated].piece_allocated_va; i < (size + arrx[allocated].piece_allocated_va);i += PAGE_SIZE)
		{
			checker[(i - page_allocator_start) / PAGE_SIZE] = 1;
		}
		sys_allocate_user_mem((uint32) arrx[allocated].piece_allocated_va, size);
	}
	else//not marked , we failed to find a suitable area then we zero the array and return null
	{
		arrx[allocated].piece_allocated_va = 0;
		return NULL;
	}
	alloc = allocated;
	allocated++;
	return (void *) arrx[alloc].piece_allocated_va;
}
//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	uint32 va = (uint32)virtual_address;
	uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;
	uint32 uhhard_limit = (uint32)myEnv->uhhard_limit;

	// CHECK IF you are freeing a block
	if(va>= USER_HEAP_START && va< uhhard_limit)
	{
		 free_block(virtual_address);
		 return;
	}
	// you are freeing a page
	//check to find the desired page to free
	else if((va >=page_allocator_start&& va<USER_HEAP_MAX ))
	{
		uint32 size = 0;
		int isfound = 0;
		int allocindex = 0;
		for(int i = 0 ; i<allocated ; i++)
		{
			if(arrx[i].piece_allocated_va == va)
			{
				size = arrx[i].piece_allocated_size;
				isfound = 1;
				allocindex = i;
				break;
			}
		}
		// if the page was found
		if(isfound == 1)
		{
			// set these pages to be free (0) in the checker array
			for(uint32 addresses =va;addresses<va+size+PAGE_SIZE ;addresses+=PAGE_SIZE)
			{
				uint32 index = (addresses - page_allocator_start) / PAGE_SIZE;
			    checker[index] = 0;
			}
			//remove the mark and any data in mem
			sys_free_user_mem(va, size);

			// we update and store the arrx array
			arrx[allocindex].piece_allocated_va = 0;
			arrx[allocindex].piece_allocated_size = 0;

			for (int j = allocindex; j < allocated - 1; j++)
			{
				 arrx[j] = arrx[j + 1];
			}
			 allocated--;
		}
		// if the page not allocated in the first place we just panic
		else if(isfound == 0)
		{
			panic("free: Trying to free an Address which is not Allocated");
		}
	}

}
//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
//struct information
//{
//	uint32 piece_allocated_size;  // allocated size
//	uint32 piece_allocated_va;   //allocated virtual address
//	uint32 sharedID;
//};

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0){
		return NULL ;
	}
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	uint32 roundedSize = ROUNDUP(size, PAGE_SIZE);
	// we update the page array with the new allocated page size
	arrx[allocated].piece_allocated_size = roundedSize;
		// intialize some variable we need for the ff strategy
		int accumualted_size = 0; // size of continus free pages in traverse
		int ptr = 0;
	    int alloc=0;

	    // we start the page allocation from the page allocator start (after block aloc by 4k)
	    uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;

	    // start traversing the heap looking for a suitable address
		for (uint32 i =page_allocator_start ; i <= USER_HEAP_MAX; i += PAGE_SIZE)
		{
			// if we arrived at the heap end then it means there is no suitsble address and we stop
			if (i == USER_HEAP_MAX)
			{
				ptr=0;
				break;
			}

			//then we check for free pages  (unmarked pages)
			if (checker[(i - page_allocator_start) / PAGE_SIZE] == 0)
			{
				// if we encountered a marked block we zero the size1 and begin again
				if (accumualted_size == 0)
				{
					// we set the corresponding va and update the array with it
					arrx[allocated].piece_allocated_va = i;
				}
				// as long as the page is free we keep incrementing with the page size
				// until the acumulated size equlas the wanted size
				accumualted_size += PAGE_SIZE;
				if (accumualted_size == roundedSize)
				{
					ptr = 1;
					break;
				}
			}
			else
			{
				// if we encountered an allocated or marked page we reset the size to start again
				accumualted_size = 0;
			}
		}
		// we found a suitable space for the allocation / we mark it in the array
		if (ptr != 0)
		{
			int cheakAllocate = sys_createSharedObject(sharedVarName,size, isWritable ,(void*) arrx[allocated].piece_allocated_va);
			if(cheakAllocate!= E_NO_SHARE && cheakAllocate!= E_SHARED_MEM_EXISTS )
			{
				// here we mark the pages in the corresponding checker array
				// we set it to 1
				for (uint32 i = arrx[allocated].piece_allocated_va; i < (roundedSize + arrx[allocated].piece_allocated_va);i += PAGE_SIZE)
				{
					checker[(i - page_allocator_start) / PAGE_SIZE] = 1;
				}
				arrx[allocated].sharedID=cheakAllocate;
				alloc = allocated;
				allocated++;
				return (void *) arrx[alloc].piece_allocated_va;
			}
			else{
				arrx[allocated].piece_allocated_va=0;
				arrx[allocated].piece_allocated_size=0;
				return NULL;
			}

		}
		else//not marked , we failed to find a suitable area then we zero the array and return null
		{
			arrx[allocated].piece_allocated_va = 0;
			return NULL;
		}
}




//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
//	panic("sget() is not implemented yet...!!");
	int objectSize=sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if(objectSize==E_SHARED_MEM_NOT_EXISTS)
		return NULL;

	// we update the page array with the new allocated page size
		arrx[allocated].piece_allocated_size = objectSize;
		// intialize some variable we need for the ff strategy
		int accumualted_size = 0; // size of continus free pages in traverse
		int ptr = 0;
	    int alloc=0;

	    // we start the page allocation from the page allocator start (after block aloc by 4k)
	    uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;

	    // start traversing the heap looking for a suitable address
		for (uint32 i =page_allocator_start ; i <= USER_HEAP_MAX; i += PAGE_SIZE)
		{
			// if we arrived at the heap end then it means there is no suitsble address and we stop
			if (i == USER_HEAP_MAX)
			{
				ptr=0;
				break;
			}

			//then we check for free pages  (unmarked pages)
			if (checker[(i - page_allocator_start) / PAGE_SIZE] == 0)
			{
				// if we encountered a marked block we zero the size1 and begin again
				if (accumualted_size == 0)
				{
					// we set the corresponding va and update the array with it
					arrx[allocated].piece_allocated_va = i;
				}
				// as long as the page is free we keep incrementing with the page size
				// until the acumulated size equlas the wanted size
				accumualted_size += PAGE_SIZE;
				if (accumualted_size == ROUNDUP(objectSize,PAGE_SIZE))
				{
					ptr = 1;
					break;
				}
			}
			else
			{
				// if we encountered an allocated or marked page we reset the size to start again
				accumualted_size = 0;
			}
		}
		// we found a suitable space for the allocation / we mark it in the array
		if (ptr != 0)
		{
			int getShareID=sys_getSharedObject(ownerEnvID,sharedVarName,(void*)arrx[allocated].piece_allocated_va);
			if(getShareID==E_SHARED_MEM_NOT_EXISTS){
				arrx[allocated].piece_allocated_va = 0;
				arrx[allocated].piece_allocated_size = 0;
				return NULL;
				}
			else{
				// here we mark the pages in the corresponding checker array
				// we set it to 1
				for (uint32 i = arrx[allocated].piece_allocated_va; i < (ROUNDUP(objectSize,PAGE_SIZE) + arrx[allocated].piece_allocated_va);i += PAGE_SIZE)
				{
					checker[(i - page_allocator_start) / PAGE_SIZE] = 1;
				}
				arrx[allocated].sharedID= getShareID;
				alloc = allocated;
				allocated++;
				return (void*) arrx[alloc].piece_allocated_va;
			}
		}
		else//not marked , we failed to find a suitable area then we zero the array and return null
		{
			arrx[allocated].piece_allocated_va = 0;
			arrx[allocated].piece_allocated_size = 0;
			return NULL;
		}
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
//	panic("sfree() is not implemented yet...!!");
	uint32 va = (uint32)virtual_address;
	uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;
	uint32 uhhard_limit = (uint32)myEnv->uhhard_limit;
	// you are freeing a page
	//check to find the desired page to free
		uint32 size = 0;
		int isfound = 0;
		int allocindex = 0;
		for(int i = 0 ; i<allocated ; i++)
		{
			if(arrx[i].piece_allocated_va == va)
			{
				size = arrx[i].piece_allocated_size;
				isfound = 1;
				allocindex = i;
				break;
			}
		}
		// if the page was found
		if(isfound == 1)
		{
			// set these pages to be free (0) in the checker array
			for(uint32 addresses =va;addresses<va+size+PAGE_SIZE ;addresses+=PAGE_SIZE)
			{
				uint32 index = (addresses - page_allocator_start) / PAGE_SIZE;
			    checker[index] = 0;
			}
			// we update and store the arrx array
			arrx[allocindex].piece_allocated_va = 0;
			arrx[allocindex].piece_allocated_size = 0;
			int id=arrx[allocindex].sharedID;
			arrx[allocindex].sharedID=0;
			for (int j = allocindex; j < allocated - 1; j++)
			{
				 arrx[j] = arrx[j + 1];
				 arrx[j].sharedID = arrx[j + 1].sharedID;
			}
			 allocated--;
			uint32 returnedID= sys_freeSharedObject(id,virtual_address);
			if(returnedID==E_SHARED_MEM_NOT_EXISTS){
				panic("SharedObject could not be found");
			}
		}
		// if the page not allocated in the first place we just panic
		else if(isfound == 0)
		{
			panic("free: Trying to free an Address which is not Allocated");
		}
}
//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	//panic("realloc() is not implemented yet...!!");
	if((virtual_address==NULL&&new_size==0)||(uint32)virtual_address>KERNEL_HEAP_MAX||new_size>DYN_ALLOC_MAX_SIZE)
	{
		return NULL;
	}
	if(virtual_address !=NULL && new_size == 0)
	{
		free(virtual_address);
		return NULL;
	}
	else if(virtual_address==NULL && new_size!=0)
	{
		return malloc(new_size);
	}
	// The virtual address in Block Range
	if((uint32)virtual_address  >= KERNEL_HEAP_START && (uint32)virtual_address<(uint32)myEnv->uhhard_limit)
	{
		uint32 current_block_size=get_block_size(virtual_address);
		if(new_size==current_block_size)
		{
			return virtual_address;
		}
		else if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			// new size in Block range
			return realloc_block_FF(virtual_address , new_size);

		}
		else
		{    //you were a block but now you will be page
			// new size in Page Range
			//move the data
			void*distenation_address=malloc(new_size);
			if(distenation_address==NULL)
			{
				return NULL;
			}
			sys_move_user_mem((uint32)virtual_address,(uint32)distenation_address,new_size);
			free(virtual_address);
			return distenation_address;
		}
	}
	//you are now in page allocator range
	else if((uint32)virtual_address>= (uint32)myEnv->uhhard_limit+PAGE_SIZE&&(uint32)virtual_address<KERNEL_HEAP_MAX)
	{
		//you are in page allocator but your new size is in block allocator
		if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
		{   //you were page but now you will be a block
			// new size in Block range
			//you must move the data
			void*distenation_address=malloc(new_size);
			if(distenation_address==NULL)
			{
				return NULL;
			}
			sys_move_user_mem((uint32)virtual_address,(uint32)distenation_address,new_size);
			free(virtual_address);
			return distenation_address;

		}
         uint32 va=ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
         uint32 current_size=0;
		//get the current size of desired VA and the actual VA you must round it down
		for(int i=0;i<allocated;i++)
		{
			if(arrx[i].piece_allocated_va==va)
			{
				current_size=arrx[i].piece_allocated_size;
				break;
			}
		}
		//reallocate with same size ? no thing to do
		if(current_size==new_size||ROUNDUP(current_size,PAGE_SIZE)==ROUNDUP(new_size,PAGE_SIZE))
		{
			return virtual_address;
		}
		else if (new_size<current_size&&ROUNDUP(current_size,PAGE_SIZE)!=ROUNDUP(new_size,PAGE_SIZE))
		{
			//you need to unmape some pages from the old size
			//get the rounded size for search
			uint32 rounded_new_size=ROUNDUP(new_size,PAGE_SIZE);
			uint32 rounded_current_size=(ROUNDUP(current_size,PAGE_SIZE));

			//get the deallocated number of pages
			uint32 deallocated_size=current_size-new_size;
			deallocated_size=ROUNDUP(deallocated_size,PAGE_SIZE);
			uint32 num_of_deallocated_pages=deallocated_size/PAGE_SIZE;

			//get the start address to update data in the array
			uint32 address=va+rounded_new_size;
			// no need to move the data
			for(int i=address;i<num_of_deallocated_pages;i+=PAGE_SIZE)
			{
              checker[i]=0;
			}
			sys_free_user_mem(va+rounded_new_size, current_size-new_size);
			arrx[va].piece_allocated_size=new_size;
			sys_move_user_mem((uint32)virtual_address,(uint32)virtual_address,new_size);
			return virtual_address;

		}
		else if(new_size>current_size)
		{
			uint32 rounded_new_size=ROUNDUP(new_size,PAGE_SIZE);
			uint32 rounded_current_size=(ROUNDUP(current_size,PAGE_SIZE));
			int visited=0;
			for(int i=va+rounded_current_size;i<va+rounded_new_size;i+=PAGE_SIZE)
			{
				if(checker[i]!=0)
				{
					visited=1;
					break;
				}
			}
			if(visited)
			{
				void*distenation_address=malloc(new_size);
				if(distenation_address==NULL)
				{
					return NULL;
				}
				sys_move_user_mem((uint32)virtual_address,(uint32)distenation_address,new_size);
				free(virtual_address);
				return distenation_address;
			}
			else
			{
				for(int i=va+rounded_current_size;i<va+rounded_new_size;i+=PAGE_SIZE)
				{
					checker[i]=1;
				}
				sys_allocate_user_mem(va+rounded_current_size, new_size-current_size);
           }
		}

	}

	 return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
//#include <inc/lib.h>
////==================================================================================//
////============================ REQUIRED FUNCTIONS ==================================//
////==================================================================================//
//
////=============================================
//// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
////=============================================
///*2023*/
//void* sbrk(int increment)
//{
//	return (void*) sys_sbrk(increment);
//}
////=================================
//// [2] ALLOCATE SPACE IN USER HEAP:
////=================================
//void* malloc(uint32 size)
//{
//	//check for the size if illegal
//	if (size <= 0 || size > DYN_ALLOC_MAX_SIZE)
//	{
//	        return NULL;
//	}
//	// Step 1: Small allocations (block allocator)
//	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
//	{
//		return alloc_block_FF(size);
//	}
//	// it it gets here it means it's a page and we use page allocator
//	size = ROUNDUP(size, PAGE_SIZE);
//
//	// intialize some variable we need for the ff strategy
//	uint32 accumualted_size = 0; // size of continus free pages in traverse
//	int ptr = 0;
//	uint32 allocated_va;
//
//    // we start the page allocation from the page allocator start (after block aloc by 4k)
//    uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;
//
//    // start traversing the heap looking for a suitable address
//	for (uint32 i =page_allocator_start ; i < USER_HEAP_MAX; i += PAGE_SIZE)
//	{
//		// if we arrived at the heap end then it means there is no suitsble address and we stop
//		if (i == USER_HEAP_MAX)
//		{
//			ptr=0;
//			break;
//		}
//		//then we check for free pages  (unmarked pages)
//		if (sys_get_marked_bit(i)== 0)
//		{
//			// if we encountered a marked block we zero the size1 and begin again
//			if (accumualted_size == 0)
//			{
//				// we set the corresponding va and update the array with it
//				allocated_va = i;
//			}
//			// as long as the page is free we keep incrementing with the page size
//			// until the acumulated size equlas the wanted size
//			accumualted_size += PAGE_SIZE;
//			if (accumualted_size == size)
//			{
//				ptr = 1;
//				break;
//			}
//		}
//		else
//		{
//			// if we encountered an allocated or marked page we reset the size to start again
//			accumualted_size = 0;
//		}
//	}
//	// we found a suitable space for the allocation / we mark it in the array
//	if (ptr != 0)
//	{
//		// here we mark the pages
//		sys_allocate_user_mem((uint32)allocated_va,accumualted_size);
//	}
//	else//not marked , we failed to find a suitable area then we zero the array and return null
//	{
//		return NULL;
//	}
//	return (void *) allocated_va;
//}
////=================================
//// [3] FREE SPACE FROM USER HEAP:
////=================================
//void free(void* virtual_address)
//{
//	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
//	// Write your code here, remove the panic and write your code
//	//panic("free() is not implemented yet...!!");
//	uint32 va =(uint32)virtual_address;
//	va=ROUNDDOWN(va,PAGE_SIZE);
//	uint32 page_allocator_start =((uint32) myEnv->uhhard_limit)+PAGE_SIZE;
//	uint32 uhhard_limit = (uint32)myEnv->uhhard_limit;
//
//	// CHECK IF you are freeing a block
//	if(va>= USER_HEAP_START && va< uhhard_limit)
//	{
//		 free_block(virtual_address);
//		 return;
//	}
//	// you are freeing a page
//	//check to find the desired page to free
//	else if((va >=page_allocator_start&& va<USER_HEAP_MAX ))
//	{
//		if(sys_get_alloc_size(va) == 0)
//		{
//			panic("free: Trying to free an Address which is not Allocated");
//		}
//		else
//		{
//			//remove the mark and any data in mem
//			sys_free_user_mem(va, sys_get_alloc_size(va));
//		}
//	}
//}
////=================================
//// [4] ALLOCATE SHARED VARIABLE:
////=================================
//void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
//{
//	//==============================================================
//	//DON'T CHANGE THIS CODE========================================
//	if (size == 0)
//	{
//		cprintf("here1\n");
//		return NULL ;
//	}
//	//==============================================================
//	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
//	// Write your code here, remove the panic and write your code
//	//panic("smalloc() is not implemented yet...!!");
//	uint32 roundedSize = ROUNDUP(size, PAGE_SIZE);
//
//	// we update the page array with the new allocated page size
//	uint32 accumualted_size = 0; // size of continus free pages in traverse
//	int ptr = 0;
//	uint32 allocated_va;
//
//    // we start the page allocation from the page allocator start (after block aloc by 4k)
//    uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;
//
//    // start traversing the heap looking for a suitable address
//	for (uint32 i =page_allocator_start ; i < USER_HEAP_MAX; i += PAGE_SIZE)
//	{
//		// if we arrived at the heap end then it means there is no suitsble address and we stop
//		if (i == USER_HEAP_MAX)
//		{
//			ptr=0;
//			break;
//		}
//		//then we check for free pages  (unmarked pages)
//		if (sys_get_marked_bit(i)== 0)
//		{
//			// if we encountered a marked block we zero the size1 and begin again
//			if (accumualted_size == 0)
//			{
//				// we set the corresponding va and update the array with it
//				allocated_va = i;
//			}
//			// as long as the page is free we keep incrementing with the page size
//			// until the acumulated size equlas the wanted size
//			accumualted_size += PAGE_SIZE;
//			if (accumualted_size == roundedSize)
//			{
//				ptr = 1;
//				break;
//			}
//		}
//		else
//		{
//			// if we encountered an allocated or marked page we reset the size to start again
//			accumualted_size = 0;
//		}
//	}
//	// we found a suitable space for the allocation / we mark it in the array
//	if (ptr != 0)
//	{
//		int cheakAllocate = sys_createSharedObject(sharedVarName,size, isWritable ,(void*) allocated_va);
//		if(cheakAllocate!= E_NO_SHARE && cheakAllocate!= E_SHARED_MEM_EXISTS )
//		{
//			// here we mark the pages in the corresponding checker array
//			sys_allocate_user_mem((uint32)allocated_va,accumualted_size);
//			return (void *) allocated_va;
//		}
//		else
//		{
//			return NULL;
//		}
//
//	}
//	else//not marked , we failed to find a suitable area then we zero the array and return null
//	{
//		return NULL;
//	}
//}
////========================================
//// [5] SHARE ON ALLOCATED SHARED VARIABLE:
////========================================
//void* sget(int32 ownerEnvID, char *sharedVarName)
//{
//	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
//	// Write your code here, remove the panic and write your code
//	//	panic("sget() is not implemented yet...!!");
//
//	int objectSize=sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
//	if(objectSize==E_SHARED_MEM_NOT_EXISTS)
//	{
//		return NULL;
//	}
//	uint32 round_objectSize=ROUNDUP(objectSize, PAGE_SIZE);
//
//	uint32 accumualted_size = 0;
//	int ptr = 0;
//	uint32 allocated_va;
//
//    // we start the page allocation from the page allocator start (after block aloc by 4k)
//    uint32 page_allocator_start = ((uint32) myEnv->uhhard_limit)+PAGE_SIZE;
//
//    // start traversing the heap looking for a suitable address
//	for (uint32 i =page_allocator_start ; i < USER_HEAP_MAX; i += PAGE_SIZE)
//	{
//		// if we arrived at the heap end then it means there is no suitsble address and we stop
//		if (i == USER_HEAP_MAX)
//		{
//			ptr=0;
//			break;
//		}
//		if (sys_get_marked_bit(i)== 0)
//		{
//			// if we encountered a marked block we zero the size1 and begin again
//			if (accumualted_size == 0)
//			{
//				// we set the corresponding va and update the array with it
//				allocated_va = i;
//			}
//			// as long as the page is free we keep incrementing with the page size
//			// until the acumulated size equlas the wanted size
//			accumualted_size += PAGE_SIZE;
//			if (accumualted_size == round_objectSize)
//			{
//				ptr = 1;
//				break;
//			}
//		}
//		else
//		{
//			// if we encountered an allocated or marked page we reset the size to start again
//			accumualted_size = 0;
//		}
//	}
//	// we found a suitable space for the allocation / we mark it in the array
//	if (ptr != 0)
//	{
//		int getShareID=sys_getSharedObject(ownerEnvID,sharedVarName,(void*)allocated_va);
//		if(getShareID==E_SHARED_MEM_NOT_EXISTS)
//		{
//			return NULL;
//		}
//		else
//		{
//			// here we mark the pages in the corresponding checker array
//			// we set it to 1
//			sys_allocate_user_mem((uint32)allocated_va,accumualted_size);
//			return (void*) allocated_va;
//		}
//	}
//	else//not marked , we failed to find a suitable area then we zero the array and return null
//	{
//		return NULL;
//	}
//}
////==================================================================================//
////============================== BONUS FUNCTIONS ===================================//
////==================================================================================//
//
////=================================
//// FREE SHARED VARIABLE:
////=================================
////	This function frees the shared variable at the given virtual_address
////	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
////	from main memory then switch back to the user again.
////
////	use sys_freeSharedObject(...); which switches to the kernel mode,
////	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
////	the freeSharedObject() function is empty, make sure to implement it.
//void sfree(void* virtual_address)
//{
//	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
//	// Write your code here, remove the panic and write your code
//	//	panic("sfree() is not implemented yet...!!");
//	uint32 va = (uint32)virtual_address;
//	// you are freeing a page
//	if(sys_get_alloc_size(va) == 0)
//	{
//		panic("free: Trying to free an Address which is not Allocated");
//	}
//	else
//	{
//		//remove the mark and any data in mem
//		uint32 id=sys_getSharedObjectID(va);
//		uint32 returnedID= sys_freeSharedObject(id,virtual_address);
//		if(returnedID==E_SHARED_MEM_NOT_EXISTS)
//		{
//			panic("SharedObject could not be found");
//		}
//	}
//}
///*
////=================================
//// REALLOC USER SPACE:
////=================================
////	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
////	possibly moving it in the heap.
////	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
////	On failure, returns a null pointer, and the old virtual_address remains valid.
//
////	A call with virtual_address = null is equivalent to malloc().
////	A call with new_size = zero is equivalent to free().
//
////  Hint: you may need to use the sys_move_user_mem(...)
////	which switches to the kernel mode, calls move_user_mem(...)
////	in "kern/mem/chunk_operations.c", then switch back to the user mode here
////	the move_user_mem() function is empty, make sure to implement it.
//void *realloc(void *virtual_address, uint32 new_size)
//{
//	//[PROJECT]
//	// Write your code here, remove the panic and write your code
//	//panic("realloc() is not implemented yet...!!");
//	if((virtual_address==NULL&&new_size==0)||(uint32)virtual_address>KERNEL_HEAP_MAX||new_size>DYN_ALLOC_MAX_SIZE)
//	{
//		return NULL;
//	}
//	if(virtual_address !=NULL && new_size == 0)
//	{
//		free(virtual_address);
//		return NULL;
//	}
//	else if(virtual_address==NULL && new_size!=0)
//	{
//		return malloc(new_size);
//	}
//	// The virtual address in Block Range
//	if((uint32)virtual_address  >= KERNEL_HEAP_START && (uint32)virtual_address<(uint32)myEnv->uhhard_limit)
//	{
//		uint32 current_block_size=get_block_size(virtual_address);
//		if(new_size==current_block_size)
//		{
//			return virtual_address;
//		}
//		else if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
//		{
//			// new size in Block range
//			return realloc_block_FF(virtual_address , new_size);
//		}
//		else
//		{    //you were a block but now you will be page
//			// new size in Page Range
//			//move the data
//			void*distenation_address=malloc(new_size);
//			if(distenation_address==NULL)
//			{
//				return NULL;
//			}
//			sys_move_user_mem((uint32)virtual_address,(uint32)distenation_address,new_size);
//			free(virtual_address);
//			return distenation_address;
//		}
//	}
//	//you are now in page allocator range
//	else if((uint32)virtual_address>= (uint32)myEnv->uhhard_limit+PAGE_SIZE&&(uint32)virtual_address<KERNEL_HEAP_MAX)
//	{
//		//you are in page allocator but your new size is in block allocator
//		if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
//		{   //you were page but now you will be a block
//			// new size in Block range
//			//you must move the data
//			void*distenation_address=malloc(new_size);
//			if(distenation_address==NULL)
//			{
//				return NULL;
//			}
//			sys_move_user_mem((uint32)virtual_address,(uint32)distenation_address,new_size);
//			free(virtual_address);
//			return distenation_address;
//		}
//         uint32 va=ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
//         uint32 current_size=0;
//		//get the current size of desired VA and the actual VA you must round it down
//		for(int i=0;i<allocated;i++)
//		{
//			if(arrx[i].piece_allocated_va==va)
//			{
//				current_size=arrx[i].piece_allocated_size;
//				break;
//			}
//		}
//		//reallocate with same size ? no thing to do
//		if(current_size==new_size||ROUNDUP(current_size,PAGE_SIZE)==ROUNDUP(new_size,PAGE_SIZE))
//		{
//			return virtual_address;
//		}
//		else if (new_size<current_size&&ROUNDUP(current_size,PAGE_SIZE)!=ROUNDUP(new_size,PAGE_SIZE))
//		{
//			//you need to unmape some pages from the old size
//			//get the rounded size for search
//			uint32 rounded_new_size=ROUNDUP(new_size,PAGE_SIZE);
//			uint32 rounded_current_size=(ROUNDUP(current_size,PAGE_SIZE));
//
//			//get the deallocated number of pages
//			uint32 deallocated_size=current_size-new_size;
//			deallocated_size=ROUNDUP(deallocated_size,PAGE_SIZE);
//			uint32 num_of_deallocated_pages=deallocated_size/PAGE_SIZE;
//
//			//get the start address to update data in the array
//			uint32 address=va+rounded_new_size;
//			// no need to move the data
//			sys_free_user_mem(va+rounded_new_size, current_size-new_size);
//			arrx[va].piece_allocated_size=new_size;
//			sys_move_user_mem((uint32)virtual_address,(uint32)virtual_address,new_size);
//			return virtual_address;
//		}
//		else if(new_size>current_size)
//		{
//			uint32 rounded_new_size=ROUNDUP(new_size,PAGE_SIZE);
//			uint32 rounded_current_size=(ROUNDUP(current_size,PAGE_SIZE));
//			int visited=0;
//			for(int i=va+rounded_current_size;i<va+rounded_new_size;i+=PAGE_SIZE)
//			{
//				if(sys_get_marked_bit(i)!=0)
//				{
//					visited=1;
//					break;
//				}
//			}
//			if(visited)
//			{
//				void*distenation_address=malloc(new_size);
//				if(distenation_address==NULL)
//				{
//					return NULL;
//				}
//				sys_move_user_mem((uint32)virtual_address,(uint32)distenation_address,new_size);
//				free(virtual_address);
//				return distenation_address;
//			}
//			else
//			{
//				sys_allocate_user_mem(va+rounded_current_size, new_size-current_size);
//           }
//		}
//
//	}
//
//	 return NULL;
//
//}
//*/
////==================================================================================//
////========================== MODIFICATION FUNCTIONS ================================//
////==================================================================================//
//
//void expand(uint32 newSize)
//{
//	panic("Not Implemented");
//
//}
//void shrink(uint32 newSize)
//{
//	panic("Not Implemented");
//
//}
//void freeHeap(void* virtual_address)
//{
//	panic("Not Implemented");
//
//}
