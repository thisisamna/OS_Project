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
void shrink_block(struct BlockMetaData* oldBlock, uint32 allocatedSize)
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
	panic("alloc_block_BF is not implemented yet");
	return NULL;
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
    if(va == NULL)   // if given address is pointing to null
    	return;

    struct BlockMetaData *block = ((struct BlockMetaData *)va - 1);
    if(block->is_free)   // if block is free already
    	return;
    else   // if block is occupied
    	block->is_free =1;


    struct BlockMetaData *prev = LIST_PREV(block);
    struct BlockMetaData *next = LIST_NEXT(block);

            if(prev == NULL && next == NULL)
                return;


            if((prev->is_free && next->is_free)) // if previous & next blocks are both free
                    {
                        (prev->size)+=(get_block_size(block)+get_block_size(next));
                        block->size=0;
                        (next)->size=0;
                        va = prev;
                    }
                    else if(prev->is_free) // if previous block is free
                    {
                        (prev->size)+=get_block_size(block);
                        block->size=0;
                        va = prev;
                    }
                    else if (next->is_free)   // if next block is free
                    {
                        block->size+=get_block_size(next);
                        (next->size)=0;
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

	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1);


	//(2) Check if the new size is smaller than the current size

	if (new_size <= get_block_size(curBlkMetaData))
	{
		// Update the  block's size
	   curBlkMetaData->size = new_size;
	   return va;
	}

	// Calculate the required additional size

	uint32 additional_size = new_size - curBlkMetaData->size;

	//(3)Check if there's sufficient free block in front of the current block

	struct BlockMetaData *nextBlkMetaData = LIST_NEXT(curBlkMetaData);
	if (nextBlkMetaData != NULL && nextBlkMetaData->is_free && get_block_size(nextBlkMetaData) >= additional_size)
	{
		// Resize the current block
		curBlkMetaData->size = new_size;
		nextBlkMetaData->is_free = 0;
		return va;
	}
	return NULL;
}

