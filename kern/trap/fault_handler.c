/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif
	if(isPageReplacmentAlgorithmFIFO())
	{
		if(wsSize < (curenv->page_WS_max_size))
		{
			//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
			void* va= (void*)fault_va;
			struct FrameInfo *ptr_frame_info=NULL;
			allocate_frame(&ptr_frame_info);
			map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_AVAILABLE | PERM_PRESENT|PERM_USER|PERM_WRITEABLE);
			int ret = pf_read_env_page(curenv,va);
			if (ret == E_PAGE_NOT_EXIST_IN_PF)
			{
				if ((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) || (fault_va >= USTACKBOTTOM && fault_va < USTACKTOP))
				{
				}
				else
				{
					sched_kill_env(curenv->env_id);
					return;
				}
			}
			struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);
			LIST_INSERT_TAIL(&(curenv->page_WS_list), newElement);
			if (LIST_SIZE(&(curenv->page_WS_list)) == curenv->page_WS_max_size)
			{
				curenv->page_last_WS_element = LIST_FIRST(&(curenv->page_WS_list));

			}
			else
			{
				curenv->page_last_WS_element = NULL;
			}
		}
		else
		{
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			void* va= (void*)fault_va;
			struct FrameInfo *ptr_frame_info=NULL;
			struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);

			/////////////part 1 take the victim to the page file//////////////

			//victim awl element ethat ally hya 3nd el head list first return pointer (ally hoa head )
			struct WorkingSetElement *victim = LIST_FIRST(&(curenv->page_WS_list));

			 //pointer to the oldest after the victim
			struct WorkingSetElement * nextelement = LIST_NEXT(victim);

			//check if victim is modified
			uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory,(uint32)victim->virtual_address);
			//is modified
			 if(page_permissions & PERM_MODIFIED)
			 {
				//save it to the page file
				//struct FrameInfo *victimFrameInfo = get_frame_info(curenv->env_page_directory, (uint32)victim->virtual_address , curenv->ptr_page_table);
				//int ret = pf_update_env_page(curenv, victim, victimFrameInfo);

				LIST_REMOVE(&(curenv->page_WS_list),victim);
				env_page_ws_invalidate(curenv, victim->virtual_address);

			 }
			else
			{
				LIST_REMOVE(&(curenv->page_WS_list),victim);
				env_page_ws_invalidate(curenv, victim->virtual_address);
			}



			///////my placement change //////
			//// LIST_INSERT_BEFORE(&(curenv->page_WS_list),nextelement, newElement);



			/////////////placement///////////////

			//(1)Allocate space for the faulted page
			allocate_frame(&ptr_frame_info);
			map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_AVAILABLE | PERM_PRESENT|PERM_USER|PERM_WRITEABLE);

			//(2)Read the faulted page from page file to memory
			int ret = pf_read_env_page(curenv,va);

			////(3)If the page does not exist on page file, then
			if (ret == E_PAGE_NOT_EXIST_IN_PF)
			{
				  //(3.1)If it is a stack or a heap page, then, it’s OK.
				if ((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) || (fault_va >= USTACKBOTTOM && fault_va < USTACKTOP))
				{
				}
				else
				{
				  //(3.2)Else, it must be rejected without harm to the kernel or other running processes, by killing the process.
					sched_kill_env(curenv->env_id);
					return;
				}
			}
			//6)Reflect the changes in the page working set list (i.e. add new element to list & update its last one)
			//6.1) add new element to list
			LIST_INSERT_BEFORE(&(curenv->page_WS_list),nextelement, newElement);
			//6.2)update its last one
			if (LIST_SIZE(&(curenv->page_WS_list)) == curenv->page_WS_max_size)
			{
				curenv->page_last_WS_element = LIST_FIRST(&(curenv->page_WS_list));
			}
			else
			{
				curenv->page_last_WS_element = NULL;
			}



		}
	}


	if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
		// Write your code here, remove the panic and write your code
		//panic("page_fault_handler() LRU Replacement is not implemented yet...!!");
		int ActiveSize=LIST_SIZE(&(curenv->ActiveList)); //not max just its self
		int SecondSize=LIST_SIZE(&(curenv->SecondList));  //not max just its self
		if((ActiveSize + SecondSize) < (curenv->page_WS_max_size))
	   {
		  //TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER – LRU Placement
		 //if there's space in active list
		 if((ActiveSize) < (curenv->ActiveListSize))  // its self < its max
		 {
			struct WorkingSetElement *element;
			struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);
			LIST_FOREACH(element,&(curenv->SecondList))
			{
				if(element == newElement)
				{
					//pt_set_page_permissions(curenv->SecondList,fault_va,1,PERM_PRESENT);
					LIST_REMOVE(&(curenv->SecondList),newElement);
					break;
				}
			}

			LIST_INSERT_HEAD(&(curenv->ActiveList), newElement);
			pt_set_page_permissions(curenv->env_page_directory,fault_va,1,PERM_PRESENT);

		 }
		 else
		 {
			struct WorkingSetElement *elementToMove = LIST_LAST(&(curenv->ActiveList));
			LIST_REMOVE(&(curenv->ActiveList),elementToMove);
			LIST_INSERT_HEAD(&(curenv->SecondList), elementToMove);
			pt_set_page_permissions(curenv->env_page_directory,elementToMove->virtual_address,0,PERM_PRESENT);
			struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);
			LIST_INSERT_HEAD(&(curenv->ActiveList), newElement);
			pt_set_page_permissions(curenv->env_page_directory,fault_va,1,PERM_PRESENT);

		 }

	   }
	 else
	 {          ///ToTa
		 //TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - LRU Replacement
		struct WorkingSetElement *elem_set= (struct WorkingSetElement *)fault_va;
		struct WorkingSetElement *element;

		LIST_FOREACH (element, &(curenv->SecondList))
		{

			if(elem_set==element)
			{
				struct WorkingSetElement *carry_elem_set = elem_set; // i put it in carry_elem
				LIST_REMOVE(&(curenv->SecondList),elem_set);   //remove it to space

				struct WorkingSetElement *elem_Move = LIST_LAST(&(curenv->ActiveList));
				LIST_INSERT_HEAD(&(curenv->SecondList), elem_Move);
				pt_set_page_permissions(curenv->env_page_directory,elem_Move->virtual_address,0,PERM_PRESENT);

				LIST_INSERT_HEAD(&(curenv->ActiveList),carry_elem_set);
				pt_set_page_permissions(curenv->env_page_directory,carry_elem_set->virtual_address,1,PERM_PRESENT);


			}

		}

		struct WorkingSetElement *victim_Remove = LIST_LAST(&(curenv->SecondList));
		//check if modified => write it to disk
		uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory,(uint32)victim_Remove->virtual_address);
		if(page_permissions & PERM_MODIFIED)
		{
		   //write it to disk
		   LIST_REMOVE(&(curenv->SecondList),victim_Remove);
		}
		 else
		 {
			 LIST_REMOVE(&(curenv->SecondList),victim_Remove);
		 }
		struct WorkingSetElement *elem_Move = LIST_LAST(&(curenv->ActiveList));
		LIST_INSERT_HEAD(&(curenv->SecondList), elem_Move);
		//PDX (elem_Move->virtual_address) in case using curenv is false replace it with that .
		pt_set_page_permissions(curenv->env_page_directory,elem_Move->virtual_address,0,PERM_PRESENT);

		LIST_INSERT_HEAD(&(curenv->ActiveList),elem_set);
		pt_set_page_permissions(curenv->env_page_directory,fault_va,1,PERM_PRESENT);

	 }

		//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
	}
}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}



