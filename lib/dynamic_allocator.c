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
//============================== HELPER FUNCTIONS ===================================//
//==================================================================================//
void shrink_block(struct BlockMetaData* oldBlock, uint32 allocatedSize) // including metadata
{
    uint32 free_block_size=oldBlock->size-allocatedSize;
    if(free_block_size>=sizeOfMetaData())
    {
    struct BlockMetaData* free_block = oldBlock;
    char* free_block_char=(char*)free_block;
    free_block_char+=allocatedSize;
    free_block=(struct BlockMetaData*)free_block_char;
        free_block->size= free_block_size;
        free_block->is_free=1;
        LIST_INSERT_AFTER(&block_list, oldBlock, free_block);
    }
    oldBlock->is_free=0;
    oldBlock->size=allocatedSize;
}

uint32* findMinimum(uint32* arr[], int size) {
	 uint32* min = arr[0];  // Assume the first element is the minimum

    for (int i = 1; i < size; i++) {
        if (*arr[i] < *min) {
            min = arr[i];  // Update min if a smaller element is found
        }
    }

    return min;
}
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
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
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	LIST_INIT(&block_list);

	struct BlockMetaData* block = (struct BlockMetaData *) daStart;
	block->size= initSizeOfAllocatedSpace;;
	block->is_free=1;

	LIST_INSERT_HEAD(&block_list, block);

}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
    //TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
    //panic("alloc_block_FF is not implemented yet");
    if(size==0)
        return NULL;
    struct BlockMetaData* block;
    size += sizeOfMetaData();
    LIST_FOREACH(block, &block_list)
    {
        if(block->is_free)
        {
            if(size > block->size)
            {
                continue;
            }
            else if(size < block->size)
            {
            	shrink_block(block, size);

                return ++block;
            }

            else if(size == block->size)
                {
                    block->is_free=0;
                    return ++block;

                }

        }

    }
    //if no blocks were found:
    struct BlockMetaData* old_sbrk=sbrk(size);
    if(old_sbrk==(void*)-1)
        return NULL;
    else
    {
        //returns old sbreak, add block there
        old_sbrk->size= size;
        old_sbrk->is_free=1;
        LIST_INSERT_AFTER(&block_list, block, old_sbrk);
        return ++old_sbrk;
    }
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	//panic("alloc_block_BF is not implemented yet");
	if(size==0)
	return NULL;
	size += sizeOfMetaData();
	struct BlockMetaData* block;
	//LIST_FOREACH(block, &block_list){
	//In Case the size = block.size exactly
		//if(size==block->size){
		//	 block->is_free=0;
		   //   return ++block;
		//}
		//else
		//	continue;
	//}
	//int BlkListsize = LIST_SIZE(&block_list);
	//uint32* arr[BlkListsize];
	uint32 mindiff=UINT_MAX;
	struct BlockMetaData* point = NULL;
     LIST_FOREACH(block, &block_list){
	 if(block->size >= size && block->is_free){
		 if (((block->size)-size)<mindiff){
			 point=block;
			 mindiff=(block->size)-size;
			 if(mindiff==0){
				 block->is_free=0;
				return ++block;
			 }

		 }

		 //for (int i = 0; i < BlkListsize; i++) {
			// arr[i]=(uint32*)block;
			// *arr[i]=((block->size)-size);
		// }
		// block=(struct BlockMetaData*)findMinimum(arr,BlkListsize)
	 }
	// block->is_free=0;
	// return ++block;
       }
     if(point!=NULL){
    shrink_block(point, size);
     return ++point;
     }
     //if no blocks were found:
         struct BlockMetaData* old_sbrk=sbrk(size);
         if(old_sbrk==(void*)-1)
             return NULL;
         else
         {
             //returns old sbreak, add block there
             old_sbrk->size= size;
             old_sbrk->is_free=1;
             LIST_INSERT_AFTER(&block_list, block, old_sbrk);
             return ++old_sbrk;
         }
}
//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{

	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	//panic("free_block is not implemented yet");
	if(va ==NULL)   // if given address is pointing to null
		    	return;
		    struct BlockMetaData* block = ((struct BlockMetaData *) va-1);
			if(block->is_free)   // if block is free already
				return;
			else// if block is occupied
				 block->is_free =1;

			struct BlockMetaData *prev = LIST_PREV(block);
			struct BlockMetaData *next = LIST_NEXT(block);
			if (next!=NULL && next->is_free)   // if next block is free
			{
				block->size += next->size;
				next->size=0;
				next->is_free=0;

				LIST_REMOVE(&block_list, next);

				//next=block;
			}
			if (prev!=NULL && prev->is_free)   // if prev block is free
			{
				prev->size += block->size;
				block->size=0;
				block->is_free=0;
				LIST_REMOVE(&block_list, block);
			}

}



//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================

void *realloc_block_FF(void* va, uint32 new_size)
{
    //TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()

    uint32 original_size = get_block_size(va);

    //(1)handling the special cases
    if (va == NULL)
    {
    	return alloc_block_FF(new_size); //alloc_FF(n) in case of realloc_block_FF(null, new_size), and size=0 handled in alloc

     }
    if (new_size == 0)
    {
       free_block(va); //to free(va) in case of realloc_block_FF(va,0)
        return NULL;
    }



	// Get the current block's metadata

	struct BlockMetaData *block = ((struct BlockMetaData *)va - 1);


	//(2) Check if the new size is smaller than the current size

	if (new_size <= block->size)
	{
		// Update the  block's size

	   shrink_block(block, new_size+sizeOfMetaData());
		struct BlockMetaData *next = LIST_NEXT(block);
		free_block(next);
		/*if (next!=NULL && next->is_free)   // if next block is free
		{
			//block->size += next->size;
			next->size=0;
			next->is_free=0;

			//LIST_REMOVE(&block_list, next);

			//next=block;
		}*/


	   return va;
	}

	//(3) Check if the new size is larger than current
	//NEW size without meta
	uint32 additional_size = new_size - block->size - sizeOfMetaData();

	struct BlockMetaData *next = LIST_NEXT(block);
	if (next != NULL && next->is_free){
		//(3.1)Check if there's sufficient free space right in front of block
		if(next->size +sizeOfMetaData() >= additional_size)
		{
			shrink_block(next, additional_size);
			block->size = new_size+sizeOfMetaData();
			next -> size=0;
			next->is_free=0;
			LIST_REMOVE(&block_list, next);
			free_block(LIST_NEXT(next));
			// Resize the current block
			return va;
		}

	}
	//(3.2)Check if there's a sufficient free block anywhere in the list
	return alloc_block_FF(new_size);
}

