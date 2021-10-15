/*This module initializes the nucleus of the PAND OS*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/scheduler.h"

static extern int processCount;
static extern int blockedCount;
static extern pcb_PTR readyQ;
static extern pcb_PTR currentProc;
static extern int deviceSema4s[5][8]; /*Two Sema4s per Device*/
void extern test();

/*
This is the main function for the PANDOS Nucleus. It initializes the pcbs, asl, global variables,
and 
*/
public void main() {

	/*Populate the Processor 0 pass Up Vector*/
	xxx->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
	
	/*Set the nucleus exception handler*/
	xxx->exception_handler = (memaddr) foobar;
	
	/*Initialize the pcbs and ASL*/
	initPcbs();
	initAsl();
	/*Initialize the global variables*/
	processCount = 0;
	blockedCount = 0;
	readyQ = mkEmptyProcQ();
	currentProc = NULL;
	int i;
	int j;
	for(i = 0, i < 5, i++)
	{
		for(j = 0, j < 8, j++)
		{
			deviceSema4s[i][j] = 0;
		}
	} 
	
	/*Load the system wide interval timer with 100 milliseconds*/
	
	/*Allocate the first pcb and insert it onto the readyQ*/
	tempPcb = allocPcb;
	tempPcb -> p_s.s_pc = (memaddr) test;
	insertProcQ(&readyQ,tempPcb);
	
	scheduler();
}
