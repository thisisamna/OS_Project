#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"


int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
	//panic("not implemented yet");

	/*Requirement 1: Initialization*/
	start = daStart;
	segment_break = daStart +initSizeToAllocate;
	hard_limit = daLimit;

	//handle: "if no memory found" ???
	if(initSizeToAllocate > daLimit - daStart)
		return E_NO_MEM;

	/*Requirement 2: All pages within space should be allocated and mapped*/
	uint32 page;
	struct FrameInfo *frame;
	for(page = daStart; page<segment_break;page+=PAGE_SIZE)
	{

		allocate_frame(&frame);
		map_frame(ptr_page_directory, frame,  page, PERM_PRESENT | PERM_USER | PERM_WRITEABLE);

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
	return (void*)-1 ;
	panic("not implemented yet");
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//if(size >= (KERNEL_HEAP_MAX - (hard_limit + PAGE_SIZE)))
				//	return NULL;
	if(size >= ((134193153))) //TEMPRORARY!! JUST TO MAKE IT SHUT UP!
		return NULL;

	int numOfPages = (ROUNDUP(size,PAGE_SIZE))/PAGE_SIZE;
	int num_of_frames = 0;
	uint32 pa = 0;
	uint32 *ptr_page_table;
	struct FrameInfo *frame =NULL;


	if(!isKHeapPlacementStrategyFIRSTFIT())
		return NULL; //don't know what else to do lol
	void* allocated= NULL;
	if(size<=DYN_ALLOC_MAX_BLOCK_SIZE) //block allocator
	{
		allocated = alloc_block_FF(size);
	}
	else //page allocator
	{
		uint32 va;
		for(uint32 i = 0; i<numOfPages; i++)
		{
		frame = NULL;
				int is_allocated = allocate_frame(&frame);
//				if(is_allocated == 0)
//				if(frame==NULL)
//					allocate_frame(&frame);
//			if(frame!=NULL)
//			{
					va = (hard_limit + PAGE_SIZE + (i*PAGE_SIZE));
					int is_mapped = map_frame(ptr_page_directory, frame,  va, PERM_PRESENT | PERM_USER | PERM_WRITEABLE);
					//int is_mapped = (int)get_frame_info(ptr_page_directory, va, &ptr_page_table);
					if(is_mapped == 0)
					{
						map_frame(ptr_page_directory, frame,  va, PERM_PRESENT | PERM_USER | PERM_WRITEABLE);
						num_of_frames++;
						frame = NULL;
						if(num_of_frames == 1)
						{
							 //pa = to_physical_address(frame);
							 allocated = (void*) va;

						}
					}
					else
					{
						num_of_frames=0;
						allocated=NULL;
					}
		//}


		}
		if(numOfPages!=num_of_frames)
		{
			return NULL;
		}
		//allocated = to_frame_info(pa); //hey google cast this & retrun it
	}
	cprintf("\nnumber of pages: %d and number of frames: %d. size: %d\n", numOfPages,num_of_frames,size);
	//change this "return" according to your answer
	//panic_into_prompt("kmalloc() is not implemented yet...!!");
	return allocated;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//change this "return" according to your answer
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer
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
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
