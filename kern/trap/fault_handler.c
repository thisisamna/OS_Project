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
		fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);

	if(isPageReplacmentAlgorithmFIFO())
	{
		uint32 *ptr_table = NULL;

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

					cprintf("fifo kill\n");
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
		else   //// FIFO Replacement
			{
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			// Write your code here, remove the panic and write your cod

		//panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");
			//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
			//refer to the project presentation and documentation for details
			void* va= (void*)fault_va;


				struct WorkingSetElement *victim = curenv->page_last_WS_element;
				struct WorkingSetElement *next = curenv->page_last_WS_element->prev_next_info.le_next;

				uint32 *ptr_table=NULL;
				struct FrameInfo *newframe = get_frame_info(curenv->env_page_directory , victim->virtual_address ,&ptr_table);
				map_frame(curenv->env_page_directory,newframe,fault_va, PERM_AVAILABLE| PERM_PRESENT|PERM_USER|PERM_WRITEABLE);


				uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory,(uint32)victim->virtual_address);
				 if(page_permissions & PERM_MODIFIED)
				 {
					uint32 *ptr_table=NULL;
					struct FrameInfo *victimFrameInfo = get_frame_info(curenv->env_page_directory, (uint32)victim->virtual_address , &ptr_table);
					pf_update_env_page(curenv, victim->virtual_address,victimFrameInfo);
				 }
				unmap_frame(curenv->env_page_directory , victim->virtual_address);

				struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);
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

				//LIST_INSERT_BEFORE(&(curenv->page_WS_list), next, newElement);


				LIST_REMOVE(&(curenv->page_WS_list), victim);
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
//				curenv->page_last_WS_element=newElement->prev_next_info.le_next;
//				if(curenv->page_last_WS_element==LIST_LAST(&curenv->page_WS_list))
//				{
//					curenv->page_last_WS_element=LIST_FIRST(&curenv->page_WS_list);
//				}





			}


		}
	if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		uint32 *ptr_table = NULL;
		 struct WorkingSetElement *element;

		cprintf("Fault va: %x \n", fault_va);
		//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
		// Write your code here, remove the panic and write your code
		//panic("page_fault_handler() LRU Replacement is not implemented yet...!!");
		int ActiveSize=LIST_SIZE(&(curenv->ActiveList)); //not max just its self
		int SecondSize=LIST_SIZE(&(curenv->SecondList));  //not max just its self
		 if((ActiveSize + SecondSize) < (curenv->page_WS_max_size))
		   {
			  //TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER – LRU Placement
			 //look for address in second list
			int found=0;
			struct WorkingSetElement *newElement;
			LIST_FOREACH(element,&(curenv->SecondList))
			{
				if(element->virtual_address == fault_va)
				{
					found=1;
					//pt_set_page_permissions(curenv->SecondList,fault_va,1,PERM_PRESENT);
					newElement = element;
					LIST_REMOVE(&(curenv->SecondList),newElement);

					break;
				}
			}
			if(!found)
			{
				struct FrameInfo *ptr_frame_info=NULL;
				allocate_frame(&ptr_frame_info);
				map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_AVAILABLE | PERM_PRESENT|PERM_USER|PERM_WRITEABLE);
				newElement= env_page_ws_list_create_element(curenv, fault_va);
			}
			 //if there's space in active list

			 if((ActiveSize) < (curenv->ActiveListSize))  //will add to active list
			 {
				LIST_INSERT_HEAD(&(curenv->ActiveList), newElement);
				pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);

			 }
			 else
			 {

				struct WorkingSetElement *elementToMove = LIST_LAST(&(curenv->ActiveList));
				LIST_REMOVE(&(curenv->ActiveList),elementToMove);
				LIST_INSERT_HEAD(&(curenv->SecondList), elementToMove);
				pt_set_page_permissions(curenv->env_page_directory,elementToMove->virtual_address,0,PERM_PRESENT);
				LIST_INSERT_HEAD(&(curenv->ActiveList), newElement);
				pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);

			 }

		   }
		 else
		 {          ///ToTa
			 //TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - LRU Replacement
			 cprintf("1\n");
			int inSecondList=0;
			element=NULL;
			LIST_FOREACH(element, &(curenv->SecondList))
			{
				 cprintf("%Adress in loop: %x\n", element->virtual_address);

				if(element->virtual_address==fault_va)
				{
					inSecondList=1;
					 LIST_REMOVE(&(curenv->SecondList),element);   //remove it to space
					//env_page_ws_invalidate(curenv, elem_set->virtual_address);

					struct WorkingSetElement *elem_Move = LIST_LAST(&(curenv->ActiveList));
					LIST_REMOVE(&(curenv->ActiveList), elem_Move);
					LIST_INSERT_HEAD(&(curenv->SecondList), elem_Move);
					pt_set_page_permissions(curenv->env_page_directory,elem_Move->virtual_address,0,PERM_PRESENT);

					LIST_INSERT_HEAD(&(curenv->ActiveList),element);
					pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);



					break;
				}

			}
			 cprintf("2\n");
			 if(!inSecondList)
			{
				 cprintf("HERE\n");

			struct WorkingSetElement *newElement= env_page_ws_list_create_element(curenv, fault_va);

			struct WorkingSetElement *victim_Remove = LIST_LAST(&(curenv->SecondList));
			struct FrameInfo *frame= get_frame_info(curenv->env_page_directory,(uint32)victim_Remove, &ptr_table);

			//check if modified => write it to disk
			uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory,victim_Remove->virtual_address);
			if(page_permissions & PERM_MODIFIED)
			{
			  //write it to disk(update)
				pf_update_env_page(curenv, (uint32)victim_Remove->virtual_address, frame);
			}

	     	LIST_REMOVE(&(curenv->SecondList),victim_Remove);
			map_frame(curenv->env_page_directory,frame,fault_va,PERM_AVAILABLE | PERM_PRESENT|PERM_USER|PERM_WRITEABLE);

	     	env_page_ws_invalidate(curenv, victim_Remove->virtual_address);



			struct WorkingSetElement *elem_Move = LIST_LAST(&(curenv->ActiveList));
	     	 LIST_REMOVE(&(curenv->ActiveList),elem_Move);

			LIST_INSERT_HEAD(&(curenv->SecondList), elem_Move);
			//PDX (elem_Move->virtual_address) in case using curenv is false replace it with that .
			pt_set_page_permissions(curenv->env_page_directory,elem_Move->virtual_address,0,PERM_PRESENT);

			LIST_INSERT_HEAD(&(curenv->ActiveList),newElement);
			pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);


			}


		 }

			//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
		}
		 }
		 void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
		 {
		 	panic("this function is not required...!!");
		 }

