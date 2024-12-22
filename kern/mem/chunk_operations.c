/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/proc/user_environment.h>
#include "kheap.h"
#include "memory_manager.h"
#include <inc/queue.h>

//extern void inctst();

/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va on the given page_directory
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, cut-paste the number of pages and return 0
//	ALL 12 permission bits of the destination should be TYPICAL to those of the source
//	The given addresses may be not aligned on 4 KB
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	//[PROJECT] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	panic("cut_paste_pages() is not implemented yet...!!");
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	If ANY of the destination pages exists with READ ONLY permission, deny the entire process and return -1.
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages doesn't exist, create it with the following permissions then copy.
//	Otherwise, just copy!
//		1. WRITABLE permission
//		2. USER/SUPERVISOR permission must be SAME as the one of the source
//	The given range(s) may be not aligned on 4 KB
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//[PROJECT] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the //panic and write your code
	panic("copy_paste_chunk() is not implemented yet...!!");
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	It should set the permissions of the second range by the given perms
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, share the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	The given range(s) may be not aligned on 4 KB
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//[PROJECT] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the //panic and write your code
	panic("share_chunk() is not implemented yet...!!");
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate the given virtual range [<va>, <va> + <size>) in the given address space  <page_directory> with the given permissions <perms>.
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, allocate the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	Allocation should be aligned on page boundary. However, the given range may be not aligned.
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	//[PROJECT] [CHUNK OPERATIONS] allocate_chunk
	// Write your code here, remove the //panic and write your code
	panic("allocate_chunk() is not implemented yet...!!");
}

//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//[PROJECT] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	panic("calculate_allocated_space() is not implemented yet...!!");
}

//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
//This function should calculate the required number of pages for allocating and mapping the given range [start va, start va + size) (either for the pages themselves or for the page tables required for mapping)
//	Pages and/or page tables that are already exist in the range SHOULD NOT be counted.
//	The given range(s) may be not aligned on 4 KB
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//[PROJECT] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
/* DYNAMIC ALLOCATOR SYSTEM CALLS */
//=====================================
void* sys_sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the current user program to increase the size of its heap
	 * 				by the given number of pages. You should allocate NOTHING,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) As in real OS, allocate pages lazily. While sbrk moves the segment break, pages are not allocated
	 * 		until the user program actually tries to access data in its heap (i.e. will be allocated via the fault handler).
	 * 	2) Allocating additional pages for a process’ heap will fail if, for example, the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sys_sbrk fails, the net effect should
	 * 		be that sys_sbrk returns (void*) -1 and that the segment break and the process heap are unaffected.
	 * 		You might have to undo any operations you have done so far in this case.
	 */

	//TODO: [PROJECT'24.MS2 - #11] [3] USER HEAP - sys_sbrk
	/*====================================*/
	/*Remove this line before start coding*/
	//return (void*)-1 ;
	/*====================================*/
	struct Env* env = get_cpu_proc(); //the current running Environment to adjust its break limit
	// to check for invalid entries or that the physical frame are exhausted
	// this leads to sys_sbrk failure and so we return (void*) -1
	if(numOfPages < 0 )
	{
		return (void*)-1;
	}
	else if(numOfPages == 0)
	{
		return env->uhsegmant_break; // if pgnum = 0 then return the break address
	}
	else
	{
		uint32 increaseSize = numOfPages*(4*1024);
		uint32* newAddress = (uint32*)((uint32)env->uhsegmant_break+increaseSize);
		uint32* oldBreak=env->uhsegmant_break;

		// we check the hardlimit , mark pages and assign the new break if all good
		if(newAddress <= env->uhhard_limit)
		{
			allocate_user_mem(env,(uint32)oldBreak,increaseSize);
			env->uhsegmant_break = newAddress;
			return oldBreak;
		}
		else
		{
			return (void*)-1;
		}
	}
}
//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	/*====================================*/
	/*Remove this line before start coding*/
	// inctst();
	// return;
	/*====================================*/
	//TODO: [PROJECT'24.MS2 - #13] [3] USER HEAP [KERNEL SIDE] - allocate_user_mem()
	// Write your code here, remove the panic and write your code
	// panic("allocate_user_mem() is not implemented yet...!!");
	uint32 num_pages = ROUNDUP(size, PAGE_SIZE)/ PAGE_SIZE;
	uint32*table;
	for(int i=0;i<num_pages;i++)
	{
		table=NULL;
		int x=get_page_table(e->env_page_directory,virtual_address,&table);
		if(table==NULL)
		{
			table= create_page_table(e->env_page_directory,virtual_address);
		}
		pt_set_page_permissions(e->env_page_directory,virtual_address,PERM_MARKED,0);
		// IF it's the last page in the alloc set the end bit to 1
		if(i == (num_pages - 1))
		{
			pt_set_page_permissions(e->env_page_directory,virtual_address,PERM_ALLOC_END,0);
		}
		virtual_address+=PAGE_SIZE;
	}
}
//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	/*====================================*/
	/*Remove this line before start coding*/
	//	inctst();
	//	return;
	/*====================================*/
	//TODO: [PROJECT'24.MS2 - #15] [3] USER HEAP [KERNEL SIDE] - free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");
	struct WorkingSetElement*last=e->page_last_WS_element;
	struct WorkingSetElement*first=e->page_WS_list.lh_first;
	struct WorkingSetElement*last_list=e->page_WS_list.lh_last;
	if(last!=NULL){
		if(last!=e->page_WS_list.lh_first)
		{
			e->page_WS_list.lh_first=last;
			e->page_WS_list.lh_first->prev_next_info.le_prev=NULL;
			e->page_WS_list.lh_last=last->prev_next_info.le_prev;
			e->page_WS_list.lh_last->prev_next_info.le_next=NULL;
			if(last==last_list){
				e->page_WS_list.lh_first->prev_next_info.le_next=first;
			}
		}
	}
	// get number of pages we should free
	uint32 num_pages =  size / PAGE_SIZE;
	uint32* table;
	struct FrameInfo* frame;
	for(int i=0;i<num_pages;i++)
	{
		table=NULL;
    	//unmark the page
    	pt_set_page_permissions(e->env_page_directory,virtual_address,0,PERM_MARKED);
		if(i == num_pages - 1 )
		{
			pt_set_page_permissions(e->env_page_directory,virtual_address,0,PERM_ALLOC_END);
		}
    	// check if the page exist in disk
    	frame=get_frame_info(e->disk_env_pgdir,virtual_address,&table);
    	if(frame!=NULL)
    		//remove the page from disk
    			pf_remove_env_page(e,virtual_address);
    	//ckeck if the page in memory and if exist remove it
    	env_page_ws_invalidate(e,virtual_address);
      virtual_address+=PAGE_SIZE;
	}
		//TODO: [PROJECT'24.MS2 - BONUS#3] [3] USER HEAP [KERNEL SIDE] - O(1) free_user_mem
}
//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
    uint32 num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
    uint32* src_table = NULL;
    uint32* dst_table = NULL;
    struct FrameInfo* frame = NULL;

    for (int i = 0; i < num_pages; i++)
    {
        // Check and map source
        frame = get_frame_info(e->env_page_directory, src_virtual_address, &src_table);

        // Proceed only if the source page exists
        if (frame != NULL)
        {
            // Ensure destination memory is allocated
            int dst_exists = get_page_table(e->env_page_directory, dst_virtual_address, &dst_table);
            if (!dst_exists || dst_table == NULL)
            {
                allocate_user_mem(e, dst_virtual_address, PAGE_SIZE);
            }
            // Map the destination to the source frame
            map_frame(e->env_page_directory, frame, dst_virtual_address, pt_get_page_permissions(e->env_page_directory, src_virtual_address));

            // unmap the source page
            unmap_frame(e->env_page_directory, src_virtual_address);
        }
        // move to the next page
        src_virtual_address += PAGE_SIZE;
        dst_virtual_address += PAGE_SIZE;
    }
    // free the source memory
    free_user_mem(e, src_virtual_address - (num_pages * PAGE_SIZE), size);
}
//=====================================
// 5)  Get allocation size :
//=====================================
uint32 get_alloc_size(struct Env* e, uint32 virtual_address)
{
	uint32 accumlated_size = 0 ;
	uint32 *table=NULL;
	int x=get_page_table(e->env_page_directory,virtual_address,&table);
	if(table == NULL)
	{
		return accumlated_size;
	}
	// keep track of accumulated size
	uint32 page_permissions = pt_get_page_permissions(e->env_page_directory,virtual_address);
	// we loop over the addresses untill we find the end page
	for(uint32 i =virtual_address; page_permissions & PERM_MARKED ;)
	{
		accumlated_size += PAGE_SIZE;
		if(page_permissions & PERM_ALLOC_END)
		{
			return (uint32)accumlated_size;
		}
		i +=PAGE_SIZE;
		// here we get the permission to check the perms (marked / alloc_end)
		page_permissions = pt_get_page_permissions(e->env_page_directory,i);
	}
	return accumlated_size;
}
//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

