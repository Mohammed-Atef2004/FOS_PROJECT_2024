#include "kheap.h"
#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
#include <inc/environment_definitions.h>
#include <inc/assert.h>
#include <inc/string.h>

struct node_info
{
	uint32 startee_va;
	uint32 endee_va;
	// the size after rounding up
	uint32 nodeee_size;
};
struct spinlock kerneloo_lock;
struct node_info free_nodes[(KERNEL_HEAP_MAX - (KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE)) / (PAGE_SIZE *2) ];
struct node_info allocated_nodes[(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];

//==================================
//INIT kheap dynamic ALLOCATOR:
//==================================
//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//[PROJECT'24.MS2] [USER HEAP - KERNEL SIDE] initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code

	init_spinlock(&kerneloo_lock, "lock");
	bool lock_already_held = holding_spinlock(&kerneloo_lock);
	if (!lock_already_held)
	{
		acquire_spinlock(&kerneloo_lock);
	}
	start=(uint32*)daStart;
	segbreak=(uint32*)(daStart+initSizeToAllocate);
	rlimit=(uint32*)daLimit;

	if(segbreak > rlimit)
	{
		if (!lock_already_held)
		{
			release_spinlock(&kerneloo_lock);
		}
		panic("Initial Size exceed The Given Limit");
	}
	else
	{
		int num_of_frames=(ROUNDUP(initSizeToAllocate,PAGE_SIZE))/(PAGE_SIZE);
		struct FrameInfo *ptr;
		uint32 virtual_address=daStart;
		int alloc_ret;
		int map_ret;
		for(int i=0;i<num_of_frames;i++)
		{
			ptr=NULL;
			alloc_ret= allocate_frame(&ptr);
			if(alloc_ret==0)
			{
				map_ret=map_frame(ptr_page_directory,ptr,virtual_address,PERM_PRESENT|PERM_WRITEABLE);
				if(map_ret!=0)
				{
					uint32* table=NULL;
					free_frame(ptr);
					virtual_address-=PAGE_SIZE;
					for(int j=0;j<i-1;j++)
					{
						ptr=get_frame_info(ptr_page_directory,virtual_address,&table);
						ptr->va=0;
						unmap_frame(ptr_page_directory,virtual_address);
						virtual_address-=PAGE_SIZE;
					}
					if (!lock_already_held)
					{
						release_spinlock(&kerneloo_lock);
					}
					panic("Frame could not be mapped\n");
				}
				else
				{
					ptr->va = virtual_address;
				}
			}
			else
			{
				uint32* table=NULL;
				virtual_address-=PAGE_SIZE;
				for(int j=0;j<i-1;j++){
					ptr=get_frame_info(ptr_page_directory,virtual_address,&table);
					ptr->va=0;
					unmap_frame(ptr_page_directory,virtual_address);
					virtual_address-=PAGE_SIZE;
				}
				if (!lock_already_held)
				{
					release_spinlock(&kerneloo_lock);
				}
				panic("No Physical Memory Available");
			}
			virtual_address += PAGE_SIZE;
		}

		// the starting node
		free_nodes[0].startee_va= daLimit+ PAGE_SIZE;
		free_nodes[0].endee_va= KERNEL_HEAP_MAX;
		free_nodes[0].nodeee_size= KERNEL_HEAP_MAX - (daLimit + PAGE_SIZE);

		// the guarding node
		free_nodes[1].startee_va= -1;
		free_nodes[1].endee_va= -1;
		free_nodes[1].nodeee_size= -1;

		// the first node in the allocated array
		allocated_nodes[0].startee_va= -1;
		allocated_nodes[0].endee_va= -1;
		allocated_nodes[0].nodeee_size= -1;

		initialize_dynamic_allocator(daStart,initSizeToAllocate);
		if (!lock_already_held)
		{
			release_spinlock(&kerneloo_lock);
		}
		return 0;
	}
}
//==================================
//             sbrk
//==================================
void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//	return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//	panic("sbrk() is not implemented yet...!!");
	bool lock_already_held = holding_spinlock(&kerneloo_lock);
	    if (!lock_already_held)
	    {
	        acquire_spinlock(&kerneloo_lock);
	    }
	if(numOfPages<0)
	{
		if (!lock_already_held)
		{
			release_spinlock(&kerneloo_lock);
		}
		return (void*)-1;
	}
	else if(numOfPages==0)
	{
		if (!lock_already_held)
		{
			release_spinlock(&kerneloo_lock);
		}
		return (void*)segbreak;
	}
	else
	{
		//allocate and map the new pages
		uint32 increasedSize = numOfPages*(PAGE_SIZE);
		uint32* newAddress = (uint32*)((uint32)segbreak+increasedSize);
		uint32* oldBreak=segbreak;
		uint32 itr_va = (uint32)segbreak;
		if(newAddress <= rlimit)
		{
			struct FrameInfo *newframe;
			for(int i=0;i<numOfPages;i++)
			{
				newframe=NULL;
				int allocResult= allocate_frame(&newframe);
				if(allocResult==0)
				{
					int mapResult=map_frame(ptr_page_directory,newframe,(uint32)itr_va ,PERM_PRESENT|PERM_WRITEABLE);
					if(mapResult!=0)
					{
						uint32* table=NULL;
						free_frame(newframe);
						itr_va-=PAGE_SIZE;
						for(int j=0;j<i-1;j++){
							newframe=get_frame_info(ptr_page_directory,itr_va,&table);
							newframe->va=0;
							unmap_frame(ptr_page_directory,itr_va);
							itr_va-=PAGE_SIZE;
						}
						if (!lock_already_held)
						{
							release_spinlock(&kerneloo_lock);
						}
						return (void*) -1;
					}
					else
					{
						newframe->va = itr_va;
					}
				}
				else
				{
					uint32* table=NULL;
					itr_va-=PAGE_SIZE;
					for(int j=0;j<i-1;j++){
						newframe=get_frame_info(ptr_page_directory,itr_va,&table);
						newframe->va=0;
						unmap_frame(ptr_page_directory,itr_va);
						itr_va-=PAGE_SIZE;
					}
					if (!lock_already_held)
					{
						release_spinlock(&kerneloo_lock);
					}
					return (void*)-1;
				}
				itr_va+=PAGE_SIZE;
			}
			segbreak=newAddress;
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return (void*)oldBreak;
		}
		else
		{
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return (void*)-1;
		}
	}

}
//==================================
//            KMALLOC
//==================================
// guarding node index
uint32 end_node_index = 1;
void* kmalloc(unsigned int size)
{
	bool lock_already_held = holding_spinlock(&kerneloo_lock);
	if (!lock_already_held)
	{
		acquire_spinlock(&kerneloo_lock);
	}
    if (size <= 0 )//|| size > DYN_ALLOC_MAX_SIZE)
    {
    	if (!lock_already_held)
    			{
    				release_spinlock(&kerneloo_lock);
    			}
        return NULL;
    }
    // Step 1: Small allocations (block allocator)
    if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
    {
        void* block_address = alloc_block_FF(size);
        if (!lock_already_held)
        		{
        			release_spinlock(&kerneloo_lock);
        		}
        return block_address;
    }
    else if((KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE + size) <= KERNEL_HEAP_MAX )
    {
    // Step 2: Large allocations (page allocator)
    uint32 required_size = ROUNDUP(size, PAGE_SIZE);
    uint32 num_pages = required_size / PAGE_SIZE;
    uint32 required_size_address = 0;

    // Search for available space in free_nodes
    for (int i = 0; i < end_node_index; i++)
    {
        if (free_nodes[i].nodeee_size >= required_size)
        {
            required_size_address = free_nodes[i].startee_va;

            // update the allocated nodes array by adding the nodes to it
    		allocated_nodes[(required_size_address-(uint32)rlimit)/PAGE_SIZE].startee_va= required_size_address;
    		allocated_nodes[(required_size_address-(uint32)rlimit)/PAGE_SIZE].endee_va= required_size_address + required_size;
    		allocated_nodes[(required_size_address-(uint32)rlimit)/PAGE_SIZE].nodeee_size= required_size;

            // Adjust the free node after allocation
            free_nodes[i].startee_va += required_size;
            free_nodes[i].nodeee_size -= required_size;

            // Remove the node if its empty
            if (free_nodes[i].nodeee_size == 0)
            {
                for (int j = i; j < end_node_index - 1; j++)
                {
                    free_nodes[j] = free_nodes[j + 1];
                }
                end_node_index--;
            }
            break;
        }
    }
    if (required_size_address == 0)
    {
    	if (!lock_already_held)
    			{
    				release_spinlock(&kerneloo_lock);
    			}
        return NULL; // No suitable free block found
    }
    // Allocate frames and map them to the virtual address
    uint32 allocPtr = required_size_address;
    uint32 allocated_pages = 0; // Track how many pages were successfully mapped
    uint32* page_table;
    struct FrameInfo* frame_ptr;

    for (uint32 i = 0; i < num_pages; i++)
    {
        struct FrameInfo* frame_info = NULL;
        if (allocate_frame(&frame_info) != 0 || frame_info == NULL)
        {
        	for (uint32 j = 0; j < allocated_pages; j++)
				{
				uint32 cleanup_va = required_size_address + j * PAGE_SIZE;
				frame_ptr = get_frame_info(ptr_page_directory, cleanup_va, &page_table);
				if (frame_ptr != NULL)
				{
					free_frame(frame_ptr);
					unmap_frame(ptr_page_directory, cleanup_va);
					frame_ptr->va = 0;
				}
		}
	if (!lock_already_held)
	{
		release_spinlock(&kerneloo_lock);
	}
            return NULL; // Allocation failed
        }
        if (map_frame(ptr_page_directory, frame_info, allocPtr, PERM_WRITEABLE | PERM_PRESENT) != 0)
        {
        	for (uint32 j = 0; j < allocated_pages; j++)
			{
				uint32 cleanup_va = required_size_address + j * PAGE_SIZE;
				frame_ptr = get_frame_info(ptr_page_directory, cleanup_va, &page_table);
				if (frame_ptr != NULL)
				{
					free_frame(frame_ptr);
					unmap_frame(ptr_page_directory, cleanup_va);
					frame_ptr->va = 0;
				}
			}
            if (!lock_already_held)
            		{
            			release_spinlock(&kerneloo_lock);
            		}
            return NULL; // Mapping failed
        }
        frame_info->va=allocPtr;
        allocPtr += PAGE_SIZE;
        allocated_pages++;
    }
    if (!lock_already_held)
    		{
    			release_spinlock(&kerneloo_lock);
    		}
    return (void*)required_size_address;
    }
    return NULL ;
}
//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator
//==================================
//            KFREE
//==================================
void kfree(void* virtual_address)
{
	bool lock_already_held = holding_spinlock(&kerneloo_lock);
	if (!lock_already_held)
	{
		acquire_spinlock(&kerneloo_lock);
	}
    uint32 va = (uint32)virtual_address;
    // If the address belongs to the block allocator range
    if (va >= KERNEL_HEAP_START && va < (KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE))
    {
        free_block(virtual_address);
        if (!lock_already_held)
        		{
        			release_spinlock(&kerneloo_lock);
        		}
        return;
    }
    // If the address belongs to the page allocator range
    else if (va >= KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE && va < KERNEL_HEAP_MAX)
    {
        uint32 size =allocated_nodes[(va-(uint32)rlimit)/PAGE_SIZE].nodeee_size;
        if (size == 0)
        {
            panic("kfree: invalid address or size not found");
            if (!lock_already_held)
            		{
            			release_spinlock(&kerneloo_lock);
            		}
            return;
        }
        size = ROUNDUP(size, PAGE_SIZE);
		// prepare the metadata for the new free node
		struct node_info new_free_node ;
		new_free_node.startee_va = va;
		new_free_node.nodeee_size= size;
		new_free_node.endee_va = va + size ;

        // Find the position to insert the new node
        int i;
        for (i = 0; i < end_node_index; i++)
        {
            if (new_free_node.startee_va < free_nodes[i].startee_va)
                break;
        }
        // Insert the node at position i and shift the array
        for (int j = end_node_index; j > i; j--)
        {
            free_nodes[j] = free_nodes[j - 1];
        }
        free_nodes[i] = new_free_node;
        end_node_index++;
        // Check for merging with previous node
        if (i > 0 && free_nodes[i - 1].endee_va == free_nodes[i].startee_va)
        {
            free_nodes[i - 1].endee_va = free_nodes[i].endee_va;
            free_nodes[i - 1].nodeee_size += free_nodes[i].nodeee_size;

            // Shift the array to remove the merged node
            for (int j = i; j < end_node_index - 1; j++)
            {
                free_nodes[j] = free_nodes[j + 1];
            }
            end_node_index--;
            i--;
        }
        // Check for merging with next node
        if (i < end_node_index - 1 && free_nodes[i].endee_va == free_nodes[i + 1].startee_va)
        {
            free_nodes[i].endee_va = free_nodes[i + 1].endee_va;
            free_nodes[i].nodeee_size += free_nodes[i + 1].nodeee_size;

            // Shift the array to remove the merged node
            for (int j = i + 1; j < end_node_index - 1; j++)
            {
                free_nodes[j] = free_nodes[j + 1];
            }
            end_node_index--;
        }
        // here we update the allocated array by zeroing every attributes
		allocated_nodes[(va-(uint32)rlimit)/PAGE_SIZE].startee_va= 0;
		allocated_nodes[(va-(uint32)rlimit)/PAGE_SIZE].endee_va= 0;
		allocated_nodes[(va-(uint32)rlimit)/PAGE_SIZE].nodeee_size= 0;

		//free and unmap frames
	    uint32* page_table;
	    struct FrameInfo* frame_ptr;
        uint32 num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
        for (uint32 i = 0; i < num_pages; i++)
        {
        	frame_ptr = get_frame_info(ptr_page_directory, va, &page_table);
        	if (frame_ptr != NULL)
			{
				free_frame(frame_ptr);
				unmap_frame(ptr_page_directory, va);
				frame_ptr->va = 0;
			}
            va += PAGE_SIZE;
        }
        if (!lock_already_held)
        		{
        			release_spinlock(&kerneloo_lock);
        		}
        return;
    }
    panic("kfree: invalid address");
}
//==================================
// KHEAP PHYSICAL ADDRESS
//==================================
unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//	panic("kheap_physical_address() is not implemented yet...!!");
	//
	//	//return the physical address corresponding to given virtual_address
	//	//refer to the project presentation and documentation for details
	//	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
		uint32* table=NULL;
		uint32 pa;
		int x =get_page_table(ptr_page_directory,(uint32)virtual_address, &table);
		if(x == TABLE_IN_MEMORY)
		{
			uint32 pageNumber = table[PTX((uint32)virtual_address)];
			if( (pageNumber & ~0xFFF) != 0)
			{
				pa= (pageNumber& 0xFFFFF000)+(virtual_address & 0x00000FFF);
				return (unsigned int)pa;
			}
		}
		return 0;
}
//==================================
// KHEAP virtual ADDRESS
//==================================
unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	struct FrameInfo *current_Frame = to_frame_info((uint32)physical_address);
	uint32 offset = ((uint32)physical_address & 0x00000FFF);
	uint32 virtual_address = current_Frame->va +  offset;
	if(current_Frame->references == 0)
	{
		return 0;
	}
	else if (virtual_address < KERNEL_HEAP_START || virtual_address >= KERNEL_HEAP_MAX)
    {
    	return 0;
    }
	else

	{
		return (unsigned int)(virtual_address);
	}
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}
void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	//return NULL;
	//panic("krealloc() is not implemented yet...!!");
	bool lock_already_held = holding_spinlock(&kerneloo_lock);
		if (!lock_already_held)
		{
			acquire_spinlock(&kerneloo_lock);
		}
	if((virtual_address==NULL&&new_size==0)||(uint32)virtual_address>KERNEL_HEAP_MAX||new_size>DYN_ALLOC_MAX_SIZE)
	{
		if (!lock_already_held)
		{
			release_spinlock(&kerneloo_lock);
		}
		return NULL;
	}
	if(virtual_address !=NULL && new_size == 0)
	{
		kfree(virtual_address);
		if (!lock_already_held)
		{
			release_spinlock(&kerneloo_lock);
		}
		return NULL;
	}
	else if(virtual_address==NULL && new_size!=0)
	{
		if (!lock_already_held)
		{
			release_spinlock(&kerneloo_lock);
		}
		return kmalloc(new_size);
	}
	// The virtual address in Block Range
	if((uint32)virtual_address  >= KERNEL_HEAP_START && (uint32)virtual_address  <(uint32)rlimit)
	{
		uint32 current_block_size=get_block_size(virtual_address);
		if(new_size==current_block_size)
		{
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return virtual_address;
		}
		else if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			// new size in Block range
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return realloc_block_FF(virtual_address , new_size);
		}
		else
		{    //you were a block but now you will be page
			// new size in Page Range
			//move the data
			void*distenation_address=kmalloc(new_size);
			if(distenation_address==NULL)
		    {
				if (!lock_already_held)
				{
					release_spinlock(&kerneloo_lock);
				}
				return NULL;
		    }
			memmove(distenation_address,virtual_address,new_size);
			kfree(virtual_address);
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return distenation_address;
		}
	}
	//you are now in page allocator range
	else if((uint32)virtual_address  >= (uint32)rlimit+PAGE_SIZE&&(uint32)virtual_address<KERNEL_HEAP_MAX)
	{
		//you are in page allocator but your new size is in block allocator
		if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
		{   //you were page but now you will be a block
			// new size in Block range
			//you must move the data
			void*distenation_address=kmalloc(new_size);
			if(distenation_address==NULL)
			{
				if (!lock_already_held)
				{
					release_spinlock(&kerneloo_lock);
				}
				return NULL;
			}
			memmove(distenation_address,virtual_address,new_size);
			kfree(virtual_address);
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return distenation_address;
		}
		//get the current size of desired VA and the actual VA you must round it down
		uint32 va_to_search=ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
		uint32 current_size = allocated_nodes[(va_to_search-(uint32)rlimit)/PAGE_SIZE].nodeee_size;
		//reallocate with same size ? no thing to do
		if(current_size==new_size||ROUNDUP(current_size,PAGE_SIZE)==ROUNDUP(new_size,PAGE_SIZE))
		{
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return virtual_address;
		}
		else if (new_size<current_size)
		{
			//you need to unmap some pages from the old size
			//get the rounded size for search
			uint32 rounded_new_size=ROUNDUP(new_size,PAGE_SIZE);
			uint32 rounded_current_size=(ROUNDUP(current_size,PAGE_SIZE));

			//get the deallocated number of pages
			uint32 deallocated_size=current_size-new_size;
			deallocated_size=ROUNDUP(deallocated_size,PAGE_SIZE);
			uint32 num_of_deallocated_pages=deallocated_size/PAGE_SIZE;

			//get the start address to update data in the array
			uint32 address=va_to_search+rounded_new_size;
			uint32 new_node_start_va = address;
			// start unmap frame
			// no need to move the data
			//free and unmap frames
			uint32* page_table;
			struct FrameInfo* frame_ptr;
			for (uint32 i = 0; i < num_of_deallocated_pages; i++)
			{
				frame_ptr = get_frame_info(ptr_page_directory, address, &page_table);
				if (frame_ptr != NULL)
				{
					free_frame(frame_ptr);
					unmap_frame(ptr_page_directory, address);
					frame_ptr->va = 0;
				}
				address += PAGE_SIZE;
			}
			memmove(virtual_address,virtual_address,new_size);
			// here we need to make this update in the array ALLOCATED the free node
			// we do it by making a new allocated block for (deallocated pages)
			// then we call the kfree for the new (deallocated size)
			allocated_nodes[(new_node_start_va-(uint32)rlimit)/PAGE_SIZE].startee_va= new_node_start_va;
			allocated_nodes[(new_node_start_va-(uint32)rlimit)/PAGE_SIZE].endee_va= new_node_start_va +(num_of_deallocated_pages * PAGE_SIZE);
			allocated_nodes[(new_node_start_va-(uint32)rlimit)/PAGE_SIZE].nodeee_size= num_of_deallocated_pages * PAGE_SIZE;

			// here we update the remaining reallocated size in the allocated array
			allocated_nodes[(va_to_search-(uint32)rlimit)/PAGE_SIZE].startee_va= va_to_search;
			allocated_nodes[(va_to_search-(uint32)rlimit)/PAGE_SIZE].endee_va= new_node_start_va;
			allocated_nodes[(va_to_search-(uint32)rlimit)/PAGE_SIZE].nodeee_size= new_node_start_va - va_to_search;

			// then we update the freenodes array by calling the kfree for the deallocated node
			kfree((void*)new_node_start_va);
			if (!lock_already_held)
			{
				release_spinlock(&kerneloo_lock);
			}
			return virtual_address;
		}
		else if(new_size>current_size)
		{
			uint32 rounded_new_size=ROUNDUP(new_size,PAGE_SIZE);
			uint32 rounded_current_size=(ROUNDUP(current_size,PAGE_SIZE));
			uint32 extra_pages_required=rounded_current_size-rounded_new_size/PAGE_SIZE;
			uint32 check=0;
		   // Search for available space in free_nodes
			for (int i = 0; i < end_node_index; i++)
			{
				if (free_nodes[i].startee_va == va_to_search +rounded_current_size)
				{
					if(free_nodes[i].nodeee_size >= extra_pages_required*PAGE_SIZE )
					{
						if(check == 0)
						{
							check = 1;
						}
						// update the allocated nodes array by adding the nodes to it
						allocated_nodes[(va_to_search-(uint32)rlimit)/PAGE_SIZE].startee_va= va_to_search;
						allocated_nodes[(va_to_search-(uint32)rlimit)/PAGE_SIZE].endee_va= va_to_search + rounded_new_size;
						allocated_nodes[(va_to_search-(uint32)rlimit)/PAGE_SIZE].nodeee_size= rounded_new_size;

						// Adjust the free node after allocation
						free_nodes[i].startee_va += (extra_pages_required*PAGE_SIZE);
						free_nodes[i].nodeee_size -= (extra_pages_required*PAGE_SIZE);

						// Remove the node if its empty
						if (free_nodes[i].nodeee_size == 0)
						{
							for (int j = i; j < end_node_index - 1; j++)
							{
								free_nodes[j] = free_nodes[j + 1];
							}
							end_node_index--;
						}
				// Allocate frames and map them to the virtual address
				uint32 allocPtr = va_to_search +rounded_current_size;
				uint32 allocated_pages = 0; // Track how many pages were successfully mapped
				uint32* page_table;
				struct FrameInfo* frame_ptr;
				for (uint32 i = 0; i < extra_pages_required; i++)
				{
					struct FrameInfo* frame_info = NULL;
					if (allocate_frame(&frame_info) != 0 || frame_info == NULL)
					{
						for (uint32 j = 0; j < allocated_pages; j++)
						{
							uint32 cleanup_va = (va_to_search +rounded_current_size) + j * PAGE_SIZE;
							frame_ptr = get_frame_info(ptr_page_directory, cleanup_va, &page_table);
							if (frame_ptr != NULL)
							{
								free_frame(frame_ptr);
								unmap_frame(ptr_page_directory, cleanup_va);
								frame_ptr->va = 0;
							}
						}
						if (!lock_already_held)
						{
							release_spinlock(&kerneloo_lock);
						}
								return NULL; // Allocation failed
							}
							if (map_frame(ptr_page_directory, frame_info, allocPtr, PERM_WRITEABLE | PERM_PRESENT) != 0)
							{
								for (uint32 j = 0; j < allocated_pages; j++)
								{
									uint32 cleanup_va = (va_to_search +rounded_current_size) + j * PAGE_SIZE;
									frame_ptr = get_frame_info(ptr_page_directory, cleanup_va, &page_table);
									if (frame_ptr != NULL)
									{
										free_frame(frame_ptr);
										unmap_frame(ptr_page_directory, cleanup_va);
										frame_ptr->va = 0;
									}
								}
								if (!lock_already_held)
								{
									release_spinlock(&kerneloo_lock);
								}
								return NULL; // Mapping failed
							}
							frame_info->va=allocPtr;
							allocPtr += PAGE_SIZE;
							allocated_pages++;
						}
						if (!lock_already_held)
						{
							release_spinlock(&kerneloo_lock);
						}
						break;
					}
				}
			}
			// i found enough pages in the position
			if(check == 1)
			{
				if (!lock_already_held)
				{
					release_spinlock(&kerneloo_lock);
				}
				return (void*) va_to_search ;
			}
			// here we need to find a new locatoin
			else if(check==0)
			{
				//you must move the data
				void*distenation_address=kmalloc(new_size);
				if(distenation_address==NULL)
				{
					if (!lock_already_held)
					{
						release_spinlock(&kerneloo_lock);
					}
					return NULL;
				}
				memmove(distenation_address,virtual_address,new_size);
				kfree(virtual_address);
				if (!lock_already_held)
				{
					release_spinlock(&kerneloo_lock);
				}
				return distenation_address;
			}
		}
	}
	if (!lock_already_held)
	{
		release_spinlock(&kerneloo_lock);
	}
	 return NULL;
}
