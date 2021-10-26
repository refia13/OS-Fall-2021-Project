/*This module initializes the nucleus of the PAND OS*/
/*Included files for use within initial.c*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/scheduler.h"

/*The number of processes that exist*/
static extern int processCount;
/*The number of blocked processes that exist*/
static extern int blockedCount;
/*A pointer for the initialization of the ready queue*/
static extern pcb_PTR readyQ;
/*A pointer to the current process*/
static extern pcb_PTR currentProc;
/*The number of semaphores*/
static int semCount = 49;
/*An array containing semaphores assigned or corresponding to devices for syncronization, initialized to zero*/
static extern int deviceSema4s[semCount];
/*Stored in the first process, defined later*/
void extern test();
/*Stores current TOD, used for timekeeping*/
static extern int startTod;
/*Pseudoclock semaphore alias for ease of access*/
#define clockSem deviceSema4s[semCount-1]

/*
This is the main function for the PANDOS Nucleus. It initializes the pcbs, asl, global variables,
and 
*/
public int main() {
	
	/*Populate the Processor 0 pass Up Vector*/
	static passUpVector_t pv = (memaddr) PASSUPVECTOR;
	pv->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
	pv->tlb_refll_stackPtr = (memaddr) STACKADDRESS;
	
	STCK(startTod);
	
	/*Set the nucleus exception handler*/
	pv->exception_handler = (memaddr) genExceptionHandler;
	pv->exception_stackPtr = (memaddr) STACKADDRESS;
	devregarea_t devrega = (*deregarea_t) BUSADDRESS;
	ramtop = (devrega->rambase) + (devrega->ramsize);
	
/*Initialize the global variables*/
	/*Initialize the pcbs*/
	initPcbs();
	/*Initialize the asl*/
	initAsl();
	/*Initialize the process count*/
	processCount = 0;
	/*Initialize the blocked process count*/
	blockedCount = 0;
	/*Initialize the ready queue*/
	readyQ = mkEmptyProcQ();
	/*Initialize the current process*/
	currentProc = NULL;
	/*Initialize integers i and j for use within for loop*/
	int i;
	int j;
	/*Initialize the device semaphore array*/
	for(i = 0, i < semCount, i++)
	{
		devSema4s[i] = 0;
	} 
	/*Load the system wide interval timer with 100 milliseconds*/
	LDIT(ITSECOND);
	/*Allocate the first pcb and insert it onto the readyQ*/
	tempPcb = allocPcb();
	
	/*set up the first processor state*/
	tempPcb->p_s.s_sp = ramtop;
	tempPcb->p_s.s_pc = (memaddr) test;
	tempPcb->p_s.s_t9 = (memaddr) test;
	tempPcb->p_s.s_status = ALLOFF | IEPON | IMON | TEON;
	/*insert temporary pcb into the ready queue*/
	insertProcQ(&readyQ,tempPcb);
	
	/*Call the scheduler*/
	scheduler();
	/*Return 0*/
	return 0;
}



public int genExceptionHandler() {
	/*Wake up in the general exception handler*/
	/*Check Cause Exception Code*/
	/*Store the exception state*/
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	/*Find and store the exception code*/
	int excCode = (oldState->s_cause & EXMASK) >> 
	/*Case 1 Cause Code == 0: Interrupts*/
	if(excCode == IOEXCEPT)
	{
		/*Call the interrupt handler*/
		interrupHandler();
	}
	/*Case 2 Cause Code <= 3 && >=1: TLB*/
	/*TLB Refill Exception*/
	if(excCode <= TLBREFILLEXCEPT)
	{
		/*Call the tlb refill handler*/
		tlb_refillHandler()
	}
	/*Case 3 Code == 8: SYSCALL*/
	if(excCode == SYSCALLEXCEPT)
	{
		/*Call the syscall handler, inputting exception state for syscall code*/
		syscallHandler(oldState->s_a0);
	}
	/*Other: Pass Up or Die*/
	else {
		passUpOrDie();
	}
	
	/*Call the program trap handler*/
	programTrapHandler();


