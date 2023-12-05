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

	if(wsSize < (curenv->page_WS_max_size))
	{
		//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
		//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
		// Write your code here, remove the panic and write your code
		//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");
		//cprintf("Fault address: %x \n", fault_va);
		void* va= (void*)fault_va;
		struct FrameInfo *ptr_frame_info=NULL;
		allocate_frame(&ptr_frame_info);
		map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_AVAILABLE | PERM_PRESENT|PERM_USER|PERM_WRITEABLE);

		int ret = pf_read_env_page(curenv,va);

		if (ret == E_PAGE_NOT_EXIST_IN_PF)
		{
			//cprintf("Not in page file\n");

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
		//cprintf("page fault ws %x \n",fault_va);
		//env_page_ws_print(curenv);
		LIST_INSERT_TAIL(&(curenv->page_WS_list), newElement);
		if (LIST_SIZE(&(curenv->page_WS_list)) == curenv->page_WS_max_size)
		{
			//cprintf("FULL WORKING SET\n");
			curenv->page_last_WS_element = LIST_FIRST(&(curenv->page_WS_list));
			//env_page_ws_print(curenv);

		}
		else
        {
 			curenv->page_last_WS_element = NULL;
		}
		//refer to the project presentation and documentation for details

	}
	else
	{
		//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
				//refer to the project presentation and documentation for details
	}







	if(isPageReplacmentAlgorithmFIFO())
		{
	 //TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
	 // Write your code here, remove the panic and write your code
	 //panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");
		if(wsSize < (curenv->page_WS_max_size))
			{
				//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
				//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
				// Write your code here, remove the panic and write your code
				//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");
				//cprintf("Fault address: %x \n", fault_va);
				void* va= (void*)fault_va;
				struct FrameInfo *ptr_frame_info=NULL;
				//1)Allocate space for the faulted page
				allocate_frame(&ptr_frame_info);
				map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_AVAILABLE | PERM_PRESENT|PERM_USER|PERM_WRITEABLE);
                //2)Read the faulted page from page file to memory
				int ret = pf_read_env_page(curenv,va);
                //3)If the page does not exist on page file, then
				if (ret == E_PAGE_NOT_EXIST_IN_PF)
				{
					//cprintf("Not in page file\n");
                      //4)If it is a stack or a heap page, then, it�s OK.
					if ((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) || (fault_va >= USTACKBOTTOM && fault_va < USTACKTOP))
					{
					}
					else
					{
                      //5)Else, it must be rejected without harm to the kernel or other running processes, by killing the process.
						sched_kill_env(curenv->env_id);
						return;
					}
				}
				//6)Reflect the changes in the page working set list (i.e. add new element to list & update its last one)
				struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);
				//cprintf("page fault ws %x \n",fault_va);
				//env_page_ws_print(curenv);
				//6.1) add new element to list
				LIST_INSERT_TAIL(&(curenv->page_WS_list), newElement);
				//6.2)update its last one
				if (LIST_SIZE(&(curenv->page_WS_list)) == curenv->page_WS_max_size)
				{
					//cprintf("FULL WORKING SET\n");

					curenv->page_last_WS_element = LIST_FIRST(&(curenv->page_WS_list));
					//env_page_ws_print(curenv);

				}
				else
		        {
		 			curenv->page_last_WS_element = NULL;
				}
				//refer to the project presentation and documentation for details

			}
		}
			else
			{
				//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
						//refer to the project presentation and documentation for details
				void* va= (void*)fault_va;
			    struct FrameInfo *ptr_frame_info=NULL;
				struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);

				/////////////part 1 take the victim to the page file//////////////

				 //victim awl element ethat ally hya 3nd el head list first return pointer (ally hoa head )
				struct WorkingSetElement *victim = LIST_FIRST(&(curenv->page_WS_list));

				 //pointer to the oldest after the victim
				struct WorkingSetElement * nextelement = LIST_NEXT(*victim);

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
                      //(3.1)If it is a stack or a heap page, then, it�s OK.
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




		/*if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
		{
			//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
			// Write your code here, remove the panic and write your code
			panic("page_fault_handler() LRU Replacement is not implemented yet...!!");

			//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
		}*/

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}


}
