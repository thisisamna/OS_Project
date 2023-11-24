#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"


int virtual_addresses_sizes[((KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE)] = {0};

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM


	/*Requirement 1: Initialization*/
	start = daStart;
	segment_break = daStart +initSizeToAllocate;
	hard_limit = daLimit;


	//handle: "if no memory found" ???
	if(initSizeToAllocate > daLimit - daStart )
		return E_NO_MEM;

	/*Requirement 2: All pages within space should be allocated and mapped*/
	uint32 page;
	struct FrameInfo *frame;
	for(page = daStart; page<segment_break;page+=PAGE_SIZE)
	{
		allocate_frame(&frame);
		map_frame(ptr_page_directory, frame,  page, PERM_PRESENT | PERM_USER | PERM_WRITEABLE);
		frame->va = page;
	}


	/*Requirement 3: Call initialize_dynamic_allocator*/
	initialize_dynamic_allocator(daStart, initSizeToAllocate);

	return 0;


}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
	struct FrameInfo* frame =NULL;
	uint32 *ptr_page_table = NULL;
	uint32 old_segment_break= segment_break;
	uint32 va = segment_break;


	if(increment==0)
	{
		return (void*) old_segment_break;
	}
	else if(increment>0)
	{
		increment= ROUNDUP(increment, PAGE_SIZE);

		if(va+increment>hard_limit)
		{
			panic("Exceeded limit");
		}
		for(int i=0; i<increment/PAGE_SIZE;i++)
		{
			allocate_frame(&frame);
			map_frame(ptr_page_directory, frame,  va, PERM_PRESENT | PERM_WRITEABLE);
			frame->va = va;
			va +=PAGE_SIZE;
		}
		segment_break=va;
		return (void*) old_segment_break;

	}
	else
	{
		if(segment_break-increment < start)// dynamic allocator region is negative size
		{
			panic("Cannot shrink space beyond size zero!");
		}
		segment_break-=increment;
		increment= ROUNDDOWN(increment, PAGE_SIZE)*-1;

		for(int i=0; i<increment/PAGE_SIZE;i++)
		{
			va -=PAGE_SIZE;
			frame = get_frame_info(ptr_page_directory,va,&ptr_page_table);
			unmap_frame(ptr_page_directory,va);
			free_frame(frame);
		}

		return (void*)segment_break;;
	}

	//panic("not implemented yet");
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	int numOfPages = (ROUNDUP(size,PAGE_SIZE))/PAGE_SIZE;
	int numOfPagesFound = 0;
	uint32 *ptr_page_table = NULL;
	uint32 va = 0;
	struct FrameInfo *frame =NULL;
	void* allocated= NULL;

	if(!isKHeapPlacementStrategyFIRSTFIT())
		return NULL;

	//BLOCK ALLOCATOR
	if(size<=DYN_ALLOC_MAX_BLOCK_SIZE)
		return alloc_block_FF(size);

	//check if there is sufficient space
	for(uint32 page = (hard_limit + PAGE_SIZE); page <KERNEL_HEAP_MAX; page = (page + PAGE_SIZE))
	{
		ptr_page_table = NULL;
		//if the page is not mapped
		if(get_frame_info(ptr_page_directory, page, &ptr_page_table) == 0)
		{
			if(numOfPagesFound==0)
				va=page;
			numOfPagesFound++;
			if(numOfPagesFound == numOfPages)
			{
				//va = (page - ((numOfPages)*PAGE_SIZE) + PAGE_SIZE);
				allocated = (void*) va;
				break;
			}
		}

		else
		{
			va=0;
			numOfPagesFound = 0;
		}
	}

	if(numOfPagesFound != numOfPages)
	{
		return NULL;
	}
	if(numOfPagesFound > LIST_SIZE(&free_frame_list))
	{
		return NULL;
	}
	int index = ((va-KERNEL_HEAP_START)/PAGE_SIZE);
	virtual_addresses_sizes[index] = numOfPages;
	//allocate and map then return va
	//virtual_addresses_sizes[(int)va] = numOfPages;
	for(uint32 i = 0; i<numOfPages; i++)
	{
		frame = NULL;
		allocate_frame(&frame);
		map_frame(ptr_page_directory, frame,  (va + (PAGE_SIZE * i)), PERM_PRESENT | PERM_WRITEABLE);
		frame->va = (va + (PAGE_SIZE * i));
	}
	return allocated;
}



void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code


	uint32 va = (uint32) virtual_address;
	  //If virtual address inside the [PAGE ALLOCATOR] range
	   //FREE the space of the given address from RAM
	 if (va < KERNEL_HEAP_MAX && va>= hard_limit+PAGE_SIZE )
	 {
		 va=ROUNDDOWN(va,PAGE_SIZE);
			///from virtual to physical
		 	 uint32 index = (va-KERNEL_HEAP_START)/PAGE_SIZE;
		 	 uint32 numOfPages = virtual_addresses_sizes[index];
		 	 for(int i= 0; i< numOfPages; i++)
		 	 {
		 		 unmap_frame(ptr_page_directory,va);
		 		 va+=PAGE_SIZE;
		 	 }

		 	virtual_addresses_sizes[index]=0;
	 }

	  //If virtual address inside the [BLOCK ALLOCATOR] range
	   //Use dynamic allocator to free the given address
	 else if(va >= KERNEL_HEAP_START &&  va< hard_limit )
	 {

		free_block(virtual_address);
	 }

	  //if outside the [PAGE ALLOCATOR] range and the [BLOCK ALLOCATOR] range
	   // should panic(…)
	 else {
		 panic("invalid address");
	 }

    //panic("kfree() is not implemented yet...!!");

}


unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code

//	uint32 offset= physical_address%PAGE_SIZE;
//	physical_address-=offset;
//	physical_address=physical_address << PGSHIFT;
	//change this "return" according to your answer

	unsigned int offset = PGOFF(physical_address);
	struct FrameInfo* frame = to_frame_info(physical_address);
	if(frame == NULL)
		return 0;
	if(frame->references == 0)
		return 0;
	else
	{
		return frame->va + offset;
	}

	return 0;

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
/*
	uint32 *ptr_page_table = NULL;b
	uint32 page_table_dir = PDX(virtual_address);
	unsigned int page_table_index = PTX(virtual_address);
	unsigned int offset = PGOFF(virtual_address);

	unsigned int physical_address = 0;

	get_page_table(ptr_page_directory, (uint32)virtual_address, &ptr_page_table);
	if(ptr_page_table != NULL)
	{
		physical_address = (ptr_page_table[page_table_index]&(0xFFFFF000))+offset;
	}
	return to_physical_address((void*)physical_address);

*/
	struct FrameInfo* frame = NULL;
	uint32 *ptr_page_table = NULL;
	unsigned int offset = PGOFF(virtual_address);
	virtual_address =ROUNDDOWN(virtual_address, PAGE_SIZE);
	frame = get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table);

	if(frame!=(void*)0)
	{
		return (unsigned int)to_physical_address(frame)+offset;
	}
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	uint32 VA =(uint32)virtual_address;
	if (VA <=  KERNEL_HEAP_MAX && VA>=hard_limit+PAGE_SIZE ){
		int index = ((VA-KERNEL_HEAP_START)/PAGE_SIZE);
		uint32 numofpagesofVS=(uint32)virtual_addresses_sizes[index];
		uint32 cureent_size=(numofpagesofVS * PAGE_SIZE);

		if(new_size==cureent_size){
			return virtual_address;
		}

		if(new_size<cureent_size){
			uint32 lenthfree=cureent_size-new_size;
			//uint32 NUM_Pagesfree=lenthfree/PAGE_SIZE;
				kfree(virtual_address);
				return kmalloc(new_size);
				}

		if(new_size>cureent_size){

			uint32 *ptr_page_table =NULL;
			uint32 numOfPagesFound =0;

			uint32 lenthalloac=new_size-cureent_size;
			uint32 NUM_Pagesalloc=lenthalloac/PAGE_SIZE;

			for(uint32 page = VA + cureent_size; page <KERNEL_HEAP_MAX; page = (page + PAGE_SIZE))
			{
				ptr_page_table = NULL;
				//if the page is not mapped
				if(get_frame_info(ptr_page_directory, page, &ptr_page_table) == 0)
				{
					numOfPagesFound++;
					if(numOfPagesFound == NUM_Pagesalloc)
						{
						//allocate and map
						return (void*)VA;
						}
				}

				else //no space in front of me
				{
					break;
				}
				//if there is space behind me, i need to chekc this before i free and kmalloc there
				//there is no space around me, i need to kmalloc somewehere else and if it works ill free
			}

			/*
			if(free_frame_list.size>=NUM_Pagesalloc){
			for(int i=0; i<NUM_Pagesalloc;i++)
					{
				frame = NULL;
				allocate_frame(&frame);
				map_frame(ptr_page_directory, frame,  VA, PERM_PRESENT | PERM_WRITEABLE);
				VA +=PAGE_SIZE;
					}
		}
			return (void*)VA;
			return	kmalloc(lenthalloac);
		       }
			*/
		  }


	//(1)handling the special cases
    if (virtual_address == NULL)
    {
    	return kmalloc(new_size); //alloc_FF(n) in case of realloc_block_FF(null, new_size), and size=0 handled in alloc
    }
    if (new_size == 0)
    {
       kfree(virtual_address); //to free(va) in case of realloc_block_FF(va,0)
        return NULL;
    }

	}
	//If virtual address inside the [BLOCK ALLOCATOR] range
	       //Use dynamic allocator to realloc new size
	if(VA >=KERNEL_HEAP_START &&  VA<= hard_limit )
	     {
		realloc_block_FF(virtual_address,  new_size);
	     }
	return NULL;
	//panic("krealloc() is not implemented yet...!!");
}
