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
				block->is_free=0;
				uint32 free_block_size=block->size-size;
				block->size=size;
				if(free_block_size>=sizeOfMetaData())
				{
				struct BlockMetaData* free_block = (struct BlockMetaData*) &block[(uint32)block->size];
					free_block->size= free_block_size;
					free_block->is_free=1;
					LIST_INSERT_AFTER(&block_list, block, free_block);
				}
				return ++block;
			}

			else if(size == block->size)
				{
					block->is_free=0;
					return ++block;

				}

		}

	}
	if(sbrk(size)==(void*)-1)
		return NULL;
	else
		//returns old sbreak, go from here
		return NULL;
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
	panic("free_block is not implemented yet");
}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	panic("realloc_block_FF is not implemented yet");

/*
//the current block meta data size
uint32 original_size = get_block_size(va);


// handle the case where size is less than the original size.
//then if the size is larger than the original size
if (new_size <= original_size) {
original_size = new_size;
   return 0;
}
else if(new_size > original_size) {


struct BlockMetaData *nextBlkMetaData = LIST_NEXT(curBlkMetaData);
uint32 next_size = 0;
if (nextBlkMetaData != NULL) {
   next_size = nextBlkMetaData->size;
}

else if (nextBlkMetaData != NULL &&  free_block(va) && (original_size + next_size >= new_size)) {
get_block_size(va) = new_size;
   nextBlkMetaData->size -= (new_size - original_size);
   return 0;


}

}



//handling the special cases
if (va == NULL) {
      if (new_size == 0) {
          return NULL;  //alloc_FF(0) in case of realloc_block_FF(null,0)
      }
      else {
          return alloc_FF(new_size);  //alloc_FF(n) in case of realloc_block_FF(null,new_size)
      }
 }
else if (new_size == 0) {
      free_block(va);  // to free(va) in case of realloc_block_FF(va,0)
      return NULL;
  }




panic("realloc_block_FF is not implemented yet");
return NULL;
*/
}

