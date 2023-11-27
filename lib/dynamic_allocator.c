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
		oldBlock->size=allocatedSize;
    }//else leave block size as it is, which includes the small free bit (internal frag)
    	oldBlock->is_free=0;

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
bool is_initialized =0;

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return;
	is_initialized=1;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	LIST_INIT(&block_list);

	struct BlockMetaData* initBlock = (struct BlockMetaData *) daStart;
	initBlock->size= initSizeOfAllocatedSpace;;
	initBlock->is_free=1;

	LIST_INSERT_HEAD(&block_list, initBlock);

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
    if (!is_initialized)
    {
    uint32 required_size = size + sizeOfMetaData();
    uint32 da_start = (uint32)sbrk(required_size);
    //get new break since it's page aligned! thus, the size can be more than the required one
    uint32 da_break = (uint32)sbrk(0);
    initialize_dynamic_allocator(da_start, da_break - da_start);
    }

    struct BlockMetaData* blockInList;
    size += sizeOfMetaData();
    LIST_FOREACH(blockInList, &block_list)
    {
        if(blockInList->is_free)
        {
            if(size > blockInList->size)
            {
                continue;
            }
            else if(size < blockInList->size)
            {
            	blockInList->is_free=0;

            	shrink_block(blockInList, size);

                return ++blockInList;
            }

            else if(size == blockInList->size)
                {
            	blockInList->is_free=0;
                return ++blockInList;

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
            old_sbrk->size= ROUNDUP(size, PAGE_SIZE);
            old_sbrk->is_free=0;
            LIST_INSERT_TAIL(&block_list, old_sbrk);
            shrink_block(old_sbrk, size);
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
	struct BlockMetaData* blockInList;
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
     LIST_FOREACH(blockInList, &block_list){
	 if(blockInList->size >= size && blockInList->is_free){
		 if (((blockInList->size)-size)<mindiff){
			 point=blockInList;
			 mindiff=(blockInList->size)-size;
			 if(mindiff==0){
				 blockInList->is_free=0;
				return ++blockInList;
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
             old_sbrk->size= ROUNDUP(size, PAGE_SIZE);
             old_sbrk->is_free=0;
             LIST_INSERT_AFTER(&block_list, blockInList, old_sbrk);
             shrink_block(old_sbrk, size);
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
		    struct BlockMetaData* blockToFree = ((struct BlockMetaData *) va-1);
		    blockToFree->is_free =1;

			struct BlockMetaData *blockBefore = LIST_PREV(blockToFree);
			struct BlockMetaData *blockAfter = LIST_NEXT(blockToFree);
			if (blockAfter!=NULL && blockAfter->is_free)   // if next block is free
			{
				blockToFree->size += blockAfter->size;
				blockAfter->size=0;
				blockAfter->is_free=0;

				LIST_REMOVE(&block_list, blockAfter);

				//next=block;
			}
			if (blockBefore!=NULL && blockBefore->is_free)   // if prev block is free
			{
				cprintf("IM here \n");
				blockBefore->size += blockToFree->size;
				blockToFree->size=0;
				blockToFree->is_free=0;
				LIST_REMOVE(&block_list, blockToFree);
			}

}



//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================

void *realloc_block_FF(void* va, uint32 new_size)
{
    //TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()

	//please delete this comment -- i hate the sizeofmetadata >_<

    //if(va >= (void*)USER_LIMIT)
    	//return (void*) new_size;

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
	struct BlockMetaData *blockToReallocate = ((struct BlockMetaData *)va - 1);


	//(2) Check if the new size is smaller than the current size
	if (new_size <= blockToReallocate->size)
	{
	   // Update the  block's size
	   shrink_block(blockToReallocate, new_size+sizeOfMetaData());
	   struct BlockMetaData *freeMiniBlock = LIST_NEXT(blockToReallocate);
	   free_block(freeMiniBlock); //so that it merges if there is a free block after it
	   return va;
	}

	//(3) Check if the new size is larger than current
	//NEW size without meta
	uint32 additional_size = new_size - blockToReallocate->size;// - sizeOfMetaData();

	struct BlockMetaData *blockAfter = LIST_NEXT(blockToReallocate);
	if (blockAfter != NULL && blockAfter->is_free){
		//(3.1)Check if there's sufficient free space right in front of block
		//the size of metadata is needed to save next info we <<<CANT>>> get rid of it
		if(blockAfter->size - sizeOfMetaData() >= additional_size)
		{
			shrink_block(blockAfter, additional_size);
			blockToReallocate->size = new_size+sizeOfMetaData();
			blockAfter->size=0;
			blockAfter->is_free=0;
			//free the other split of the next block
			free_block(LIST_NEXT(blockAfter)); //newly freed block
			//remove the next block from the block_list, but its still somewhere in the memory
			LIST_REMOVE(&block_list, blockAfter);
			// Resize the current block
			return va;
		}
	}

	//(3.2)Check if there's a sufficient free block anywhere in the list
	return alloc_block_FF(new_size);
}

