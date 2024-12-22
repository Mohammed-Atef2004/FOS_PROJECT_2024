/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
		//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//	panic("initialize_dynamic_allocator is not implemented yet");
		//Your Code is Here...

	LIST_INIT(&freeBlocksList);
		 dyaBegin = (uint32*) daStart ;
		 dyaEnd = (uint32*) (daStart +  initSizeOfAllocatedSpace - sizeof(int));
		uint32* blockHeader = (uint32*) (daStart + sizeof(int));
		uint32* blockFooter = (uint32*) (daStart +  initSizeOfAllocatedSpace - 2*sizeof(int));
		*dyaBegin = 1;
		*dyaEnd = 1;
		*blockHeader = initSizeOfAllocatedSpace - 2*sizeof(int);
		*blockFooter = initSizeOfAllocatedSpace - 2*sizeof(int);
		LIST_FIRST(&freeBlocksList)=LIST_LAST(&freeBlocksList)= (struct BlockElement*) (daStart + 2*sizeof(int));
		LIST_FIRST(&freeBlocksList)->prev_next_info.le_next=NULL;
		LIST_FIRST(&freeBlocksList)->prev_next_info.le_prev=NULL;
		LIST_LAST(&freeBlocksList)->prev_next_info.le_next=NULL;
		LIST_LAST(&freeBlocksList)->prev_next_info.le_prev=NULL;
		LIST_SIZE(&freeBlocksList) = 1;
}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	 uint32* header = ((uint32*)va) -1;
	 uint32* footer = (uint32*)(((uint32)va) + (totalSize-(2*sizeof(int))));
	 if(isAllocated){
	  totalSize= totalSize | (0x00000001);
	 }
	 else{
	  totalSize= totalSize & (0xFFFFFFFE);
	 }
	 *header = totalSize;
	 *footer = totalSize;
}


//=========================================
void *alloc_block_FF(uint32 size)
 {
    if (size ==0)
    return NULL;
 //==================================================================================
 //DON'T CHANGE THESE LINES==========================================================
 //==================================================================================
 {
 if (size % 2 != 0) size++; //ensure that the size is even (to use LSB as allocation flag)
 if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
 size = DYN_ALLOC_MIN_BLOCK_SIZE ;
 if (!is_initialized)
 {
 uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
 uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
 uint32 da_break = (uint32)sbrk(0);
 initialize_dynamic_allocator(da_start, da_break - da_start);
 }
 }
 //==================================================================================
 //==================================================================================

 //TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
 //COMMENT THE FOLLOWING LINE BEFORE START CODING

 //panic("alloc_block_BF is not implemented yet");
       uint32 required_size = size + 2*sizeof(int);
       struct BlockElement* cur;
       cur = LIST_FIRST(&freeBlocksList);
       while(cur != NULL)
       {
             uint32 block_size =get_block_size(cur);
             if(required_size <= block_size)
             {
                  if(block_size - required_size <16)
                  {
                      set_block_data(cur,block_size,1);
                      LIST_REMOVE(&freeBlocksList,cur);
                      return cur;
                  }
                  else
                  {
                      set_block_data(cur,required_size,1);
                      uint32 splited_block_size = block_size - required_size;
                      struct BlockElement* newElement = (struct BlockElement *)((uint32)cur +size + 2 *sizeof(int) );
                      set_block_data(newElement,splited_block_size,0);
                      if(LIST_FIRST(&freeBlocksList)==cur){
                          LIST_REMOVE(&freeBlocksList,cur);
                          LIST_INSERT_HEAD(&freeBlocksList,newElement);
                      }
                      else if(LIST_LAST(&freeBlocksList)==cur){
                    	  LIST_REMOVE(&freeBlocksList,cur);
                    	  LIST_INSERT_TAIL(&freeBlocksList,newElement);
                      }
                      else{
                    	  LIST_INSERT_AFTER(&freeBlocksList,cur,newElement);
                    	  LIST_REMOVE(&freeBlocksList,cur);
                      }
                      return cur;
                  }
               }
          cur=cur->prev_next_info.le_next;
      }
      uint32 numberOfPages=(ROUNDUP(required_size,4*1024))/(4*1024);
      void*sbrkreturn= sbrk(numberOfPages);
      if(sbrkreturn==(void*)-1){
          return NULL;
      }
      else{
		  uint32* previousEnd=dyaEnd;
		  dyaEnd=(uint32*)((numberOfPages*PAGE_SIZE)+((uint32)previousEnd));
		   *dyaEnd=1;
		   struct BlockElement* increasedElement;
    	  if(LIST_LAST(&freeBlocksList)==NULL&&LIST_FIRST(&freeBlocksList)==NULL){
    		  increasedElement = (struct BlockElement*)sbrkreturn;
    		  set_block_data(increasedElement,numberOfPages*(4*1024),0);
    		  LIST_FIRST(&freeBlocksList)=LIST_LAST(&freeBlocksList)=increasedElement;
    		  LIST_FIRST(&freeBlocksList)->prev_next_info.le_next=NULL;
    		  LIST_FIRST(&freeBlocksList)->prev_next_info.le_prev=NULL;
    		  LIST_LAST(&freeBlocksList)->prev_next_info.le_next=NULL;
    		  LIST_LAST(&freeBlocksList)->prev_next_info.le_prev=NULL;

    	  }
    	  else{
    		      	   struct BlockElement* last = LIST_LAST(&freeBlocksList);
    		      	   uint32 sizeLast =get_block_size(last);
    		      	   if((uint32*)(((uint32)last)+sizeLast-4) == previousEnd)
    		      	   {
    		      		   LIST_REMOVE(&freeBlocksList,last);
    		      		    increasedElement = (struct BlockElement *)(last);
    		      	       set_block_data(increasedElement,sizeLast+(numberOfPages*(4*1024)) ,0);
    		      	       LIST_INSERT_TAIL(&freeBlocksList,increasedElement);
    		      	   }
    		      	   else
    		      	   {
    		      		    increasedElement = (struct BlockElement*)sbrkreturn;
    		      	       set_block_data(increasedElement,numberOfPages*(4*1024),0);
    		      	       LIST_INSERT_TAIL(&freeBlocksList,increasedElement);
    		      	   }
    	  }

    	   return alloc_block_FF(size);
      }
 }
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
//COMMENT THE FOLLOWING LINE BEFORE START CODING

//Your Code is Here...
	if(size==0)
		return NULL;
	 {
	 if (size % 2 != 0) size++; //ensure that the size is even (to use LSB as allocation flag)
	 if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
	 size = DYN_ALLOC_MIN_BLOCK_SIZE ;
	 if (!is_initialized)
	 {
	 uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
	 uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
	 uint32 da_break = (uint32)sbrk(0);
	 initialize_dynamic_allocator(da_start, da_break - da_start);
	 }
	 }
uint32 required_size = size + 2*sizeof(int);
      struct BlockElement* cur;
      struct BlockElement* Best=NULL;
      cur = LIST_FIRST(&freeBlocksList);
      uint32 bestSize;
      int check=0;;
      while(cur != NULL)
      {
            uint32 block_size = get_block_size(cur);
            if(required_size <= block_size)
            {
            	if(!check){
            		bestSize=get_block_size(cur);
            		check=1;
            	}
            if(block_size <= bestSize)
            {
            bestSize = block_size;
            Best =cur;
            }
            }
            cur=cur->prev_next_info.le_next;
       }
       if(Best == NULL)
       {
        sbrk(required_size);
           return NULL;
       }
       if( bestSize - required_size  <16)
           {
              set_block_data(Best,get_block_size(Best),1);
              LIST_REMOVE(&freeBlocksList,Best);
              return Best;
           }
       else
           {
              uint32 splited_block_size = get_block_size(Best) - required_size;
              struct BlockElement* newElement = (struct BlockElement *)((uint32)Best +size + 2 *sizeof(int) );
              set_block_data(Best,required_size,1);
              if(LIST_FIRST(&freeBlocksList)==Best)
              {
                 LIST_REMOVE(&freeBlocksList,Best);
                 LIST_INSERT_HEAD(&freeBlocksList,newElement);
              }
              else if(LIST_LAST(&freeBlocksList)==Best){
                 LIST_REMOVE(&freeBlocksList,Best);
                 LIST_INSERT_TAIL(&freeBlocksList,newElement);
              }
              else{
              LIST_INSERT_AFTER(&freeBlocksList,Best,newElement);
              LIST_REMOVE(&freeBlocksList,Best);
              }
              set_block_data(newElement,splited_block_size,0);
        return Best;
        }
}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...

	// NULL of free block so we won't do anything
	if(va==((void*)0)||is_free_block(va)==(int8)1)
	{
		return ;
	}

	//not a NULL block or a free block
	uint32 block_size = get_block_size(va);
	uint32 *current_block_header = ((uint32*)va) - 1; //get the header address
	uint32 *current_block_footer = (uint32*)(((uint32)va) + (block_size- (2* sizeof(int)))); //get the footer address

	set_block_data(va,block_size,(bool)0);
	//get the block element
	struct BlockElement *current_block=(struct BlockElement*)va;

	//iterate on the list to insert the Block at a sorted position
	int checker=0;//to check whether the condition is done or not

	//initialize iterator to iterate on the list
	struct BlockElement *IteratorOnTheList=LIST_FIRST(&freeBlocksList);
	LIST_FOREACH(IteratorOnTheList, &freeBlocksList)
	{
		if(current_block<IteratorOnTheList)
		{
			LIST_INSERT_BEFORE(&freeBlocksList,IteratorOnTheList,current_block);
			checker=1;
			break;
		}
	}
	//there are no blocks has a pointer greater than the block
	if(checker==0)
	{
		LIST_INSERT_TAIL(&freeBlocksList,current_block);
	}

	// at this moment the block should be already in the empty list and pointing to
	// next and previous free block if any.
	//checking for merging

	// first case : the block is the first in the empty list.
	// check for the next free block as there is no previous free blocks
	if( current_block == LIST_FIRST(&freeBlocksList))
	{
		// prepare the meta data for the next block
		uint32 *next_block_header = (uint32*)(((uint32)va) + (block_size- (1 * sizeof(int)))); // the header address for the next block
		uint32 next_block_size = (uint32)((*next_block_header) & ~(0x1));
		// check if the block isn't at the edge
		if(next_block_size ==(uint32)0)
		{
			set_block_data(va,block_size,(bool)0);
			return;
		}
		uint32 *next_block_address = (uint32*)(next_block_header+1);
		void *next_va = (void*)next_block_address;
		// make a block element to compare
		struct BlockElement *next_physicalblock=(struct BlockElement*)next_va;
		// check for the merging
		// first step  check if the next physical block is free
		if(is_free_block(next_va)==(int8)1)
		{
			struct BlockElement *next_list_block=LIST_NEXT(current_block);
			if(next_list_block == next_physicalblock)
			{
				// get the total size
				block_size +=next_block_size;
				// remove from the list
				LIST_REMOVE(&freeBlocksList,next_list_block);
				//LIST_REMOVE(&freeBlocksList,next_physicalblock);
				// set the new block
				set_block_data(va,block_size,(bool)0);

				return;
			}
		}
	}
	// second case : the block is the last in the empty list.
	// check for the previous free block as there is no previous free blocks
	else if( current_block ==LIST_LAST(&freeBlocksList))
	{
		// prepare the meta data for the PREVIOUS block
		uint32 *previous_block_footer = (current_block_header- 1); //get the footer address for previous block
		uint32  previous_block_size = (uint32)((* previous_block_footer) & ~(0x1));
		// check if the block isn't at the edge
		if( previous_block_size ==(uint32)0)
		{

			//set_block_data(va,block_size,(bool)0);
			return;
		}
		uint32 *previous_block_address = (uint32*)(((uint32)va) - (previous_block_size));
		void *previous_va = (void*)previous_block_address;

		// make a block element to compare
		struct BlockElement *previous_physicalblock=(struct BlockElement*)previous_va;

		// check for the merging
		// first step  check if the previous physical block is free
		if(is_free_block(previous_va)==(int8)1)
		{
			struct BlockElement *previous_list_block=LIST_PREV(current_block);
			if(previous_list_block == previous_physicalblock)
			{
				// get the total size
			LIST_REMOVE(&freeBlocksList,current_block);
			set_block_data(previous_va,previous_block_size + block_size,0);
				return;
			}
		}
	}
	else // to check the cases if the block isn't a head or a tail
	{
		// prepare the meta data for the PREVIOUS block
		uint32 *previous_block_footer =(uint32*) (current_block_header- 1); //get the footer address for previous block
		uint32  previous_block_size =(uint32) ((* previous_block_footer) & ~(0x1));
		uint32 *previous_block_address = (uint32*)(((uint32)va) - (previous_block_size));
		void *previous_va = (void*)previous_block_address;
		// make a block element to compare
		struct BlockElement *previous_physicalblock=(struct BlockElement*)previous_va;

		// prepare the meta data for the next block
		uint32 *next_block_header = (uint32*)(((uint32)va) + (block_size- (1 * sizeof(int)))); // the header address for the next block
		uint32 next_block_size = (uint32)((*next_block_header) & ~(0x1));
		uint32 *next_block_address = (uint32*)(next_block_header+1);
		void *next_va = (void*)next_block_address;
		// make a block element to compare
		struct BlockElement *next_physicalblock=(struct BlockElement*)next_va;

		// check if the next and previous blocks are free and if they are merge them
		// case 1 : the two are free
	    if((is_free_block(next_va) == (int8)1)&&(is_free_block(previous_va)== (int8)1))
		{
			// get the total size
			va =previous_va;
			block_size += (next_block_size + previous_block_size);
			// remove the blocks
			LIST_REMOVE(&freeBlocksList,next_physicalblock);
			LIST_REMOVE(&freeBlocksList,current_block);
			// set the new block

			set_block_data(va,block_size,(bool)0);
			return;
		}


		//case 2 : the previous is free , the next isn't
		else if((is_free_block(next_va) != (int8)1)&&(is_free_block(previous_va)== (int8)1))
		{
			// get the total size
			block_size +=previous_block_size;
			va =previous_va;
			//remove the block
			LIST_REMOVE(&freeBlocksList,current_block);
			// set the new block

			set_block_data(va,block_size,(bool)0);
			return;
		}
		//case 3: the next is free , the previous isn't
		else if((is_free_block(next_va) == (int8)1)&&(is_free_block(previous_va)!=(int8)1))
		{
			// get the total size
			block_size =block_size+next_block_size;
			// remove the block
			LIST_REMOVE(&freeBlocksList,next_physicalblock);
			// set the new block
			set_block_data(va,block_size,(bool)0);
			return;
		}
		else // the blocks around it are all full
		{
			// set the block
			set_block_data(va,block_size,(bool)0);
		}
	}

}
//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...
	if(new_size %2 !=0){
		  new_size+=1;
		 };
		  uint32 required_size=new_size + 2*sizeof(int);
		  uint32 block_size = get_block_size(va);
		  uint32 splited_block_size;
		 if(va!= NULL && new_size !=0){
				// First condition (required size less than block size)
		  if(required_size < block_size){
		    splited_block_size=block_size-required_size;
		   if(splited_block_size >= 16){
			    set_block_data(va,required_size,1);
		    struct BlockElement* newBlock = (struct BlockElement*)((uint32)va + required_size );
		    struct BlockElement* nextCurruntBlock = (struct BlockElement*)((uint32)va + block_size);
			if(is_free_block(nextCurruntBlock)){
				uint32 nextCurruntBlock_size =get_block_size(nextCurruntBlock);
				uint32 all_size =splited_block_size+nextCurruntBlock_size;
				if(LIST_FIRST(&freeBlocksList)==nextCurruntBlock){
					LIST_REMOVE(&freeBlocksList,nextCurruntBlock);
					LIST_INSERT_HEAD(&freeBlocksList,newBlock);
				}
				else if(LIST_LAST(&freeBlocksList)==nextCurruntBlock){
					LIST_REMOVE(&freeBlocksList,nextCurruntBlock);
					LIST_INSERT_TAIL(&freeBlocksList,newBlock);
				}
				else{
					LIST_INSERT_BEFORE(&freeBlocksList,nextCurruntBlock,newBlock);
					LIST_REMOVE(&freeBlocksList,nextCurruntBlock);
				}
				set_block_data(newBlock,all_size,0);
	}
			else{
				set_block_data(newBlock,splited_block_size,0);
				struct BlockElement* search= newBlock;
				// if it allocated it will return 0 if not return 1
				if(LIST_LAST(&freeBlocksList)>newBlock){
					do{
						search = (struct BlockElement*)((uint32)search + get_block_size(search));
					}while(is_free_block(search)==0);
					if(search->prev_next_info.le_prev == NULL){
						LIST_INSERT_HEAD(&freeBlocksList,newBlock);
					}
					else{
						LIST_INSERT_BEFORE(&freeBlocksList,search, newBlock);
					}
				}
				else{
					LIST_INSERT_TAIL(&freeBlocksList,newBlock);
				}
			}
		   }
		   else{
		    cprintf("Size can not be changed (internal fragmentation).\n");
		   }
		   return va;
		  }
		  // Second condition (required size greater than block size)
		  else if(required_size > block_size){
			  struct BlockElement* next_block_address = (struct BlockElement*)((uint32)va+block_size);
			  uint32 next_block_size=get_block_size(next_block_address);
			  uint32 all_size=block_size+next_block_size;
			  if(is_free_block(next_block_address)==1 && (required_size<=all_size)){
				  splited_block_size=all_size-required_size;
				  if(splited_block_size>=16){
					  set_block_data(va,all_size-splited_block_size,1);
					  struct BlockElement* splited_block_address=(struct BlockElement*)((uint32)va + required_size);
					  set_block_data(splited_block_address,splited_block_size,0);
						  if(LIST_LAST(&freeBlocksList)==next_block_address){
						LIST_REMOVE(&freeBlocksList,next_block_address);
						LIST_INSERT_TAIL(&freeBlocksList,splited_block_address);
					  }
					  else if(LIST_FIRST(&freeBlocksList)==next_block_address){
							LIST_REMOVE(&freeBlocksList,next_block_address);
							LIST_INSERT_HEAD(&freeBlocksList,splited_block_address);
					  }
					  else{
							LIST_INSERT_AFTER(&freeBlocksList,next_block_address,splited_block_address);
							LIST_REMOVE(&freeBlocksList,next_block_address);
					  }
				  }
				  else{
					LIST_REMOVE(&freeBlocksList,next_block_address);
					  set_block_data(va,all_size,1);
				  }
				  return va;
			  }
			  else{
				  // apply FF strategy
				  void*returned_block_add=alloc_block_FF(new_size);
				  if(returned_block_add==NULL){
					  return va;
				  }
				  short*returned=(short*)returned_block_add;
				  short*alternative=(short*)va;
				  for(int i=0;i<((block_size-8)/2);i++){
					  *returned=*alternative;
					  returned++;
					  alternative++;
				  }
				  free_block(va);
					  return returned_block_add;
			  }
		  }
		  else{
			  return va;
		  }
		 }
		 else if(va == NULL && new_size !=0){
			 return alloc_block_FF(new_size);
		 }
		 else if (new_size==0 && va!=NULL){
			 free_block(va);
			 return NULL;
		 }
		 else if (new_size==0 && va==NULL){
			 return alloc_block_FF(new_size);
		 }
		 else {
			 return NULL;
		 }
}
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
