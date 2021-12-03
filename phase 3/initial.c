/*This module initializes the nucleus of the PAND OS*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

/*Initial process*/
extern void test();
/*External function, handles user TLB refill events*/
extern void uTLB_RefillHandler();
/*External function, handles syscalls 1-8, I/O, and program traps, performing pass up or die for anything else*/
extern int genExceptionHandler();
/*External function, handles TLB related exceptions, performs pass up or die with PGFAULTEXCEPT value*/
extern void tlbExceptionHandler();
/*Passes up to higher level of OS or terminates process based on support structure pointer*/
extern void passUpOrDie();
/*The number of processes*/
int processCount;
/*The number of processes waiting on I/O*/
int softBlockCount;
/*Ready queue, a queue of processes in the ready state*/
pcb_PTR readyQ;
/*The current process running*/
pcb_PTR currentProc;
/*Starting time of day at the beginning of an interval*/
int startTod;
/*Array of 49 syncronization semaphores for devices*/
int deviceSema4s[SEMCOUNT];

/*
This is the main function for the PANDOS Nucleus. It initializes the pcbs, asl, global variables,
and 
*/
int main() {
	
	/*Populate the Processor 0 pass Up Vector*/
	passupvector_t *pv;
	pv = (passupvector_t*) PASSUPVECTOR;
	pv->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
	pv->tlb_refll_stackPtr = (memaddr) STACKADDRESS;
	
	/*Store current time of day in startTod to mark the beginning of an interval*/
	STCK(startTod);
	
	/*Set the nucleus exception handler*/
	pv->execption_handler = (memaddr) genExceptionHandler;
	pv->exception_stackPtr = (memaddr) STACKADDRESS;
	devregarea_t *devrega;
	devrega = (devregarea_t*) BUSADDRESS;
	memaddr ramtop;
	ramtop = (devrega->rambase) + (devrega->ramsize);
	
	/*Initialize the pcbs and ASL*/
	initPcbs();
	initASL();
	
	/*Initialize the global variables*/
	processCount = 0;
	softBlockCount = 0;
	readyQ = mkEmptyProcQ();
	currentProc = NULL;
	int i;
	for(i = 0; i < SEMCOUNT; i++)
	{
		deviceSema4s[i] = 0;
	} 
	
	/*Load the system wide interval timer with 100 milliseconds*/
	LDIT(ITSECOND);
	
	/*Allocate the first pcb and insert it onto the readyQ*/
	pcb_PTR tempPcb;
	tempPcb = allocPcb();
	
	/*set up the first processor state*/
	state_t initialState;
	initialState.s_pc = (memaddr) test;
	initialState.s_status = ALLOFF | IEPON | IMON | TEON;
	initialState.s_sp = ramtop;
	initialState.s_t9 = (memaddr) test;
	
	/*StateCopy used rather than directly editing the registers for encapsulation purposes*/
	stateCopy(&initialState, &tempPcb->p_s);
	insertProcQ(&readyQ,tempPcb);
	processCount++;
	
	/*Call the scheduler*/
	scheduler();
	return 0;
}



int genExceptionHandler() {
	/*Wake up in the general exception handler*/
	/*Check Cause Exception Code*/
	state_PTR oldState = (state_t*)EXCEPTSTATEADDR;
	
	int excCode = (((oldState->s_cause) & EXMASK) >> 2);
	/*Case 1 Cause Code == 0: Interrupts*/
	if(excCode == 0)
	{
		interruptHandler(oldState);
	}
	/*Case 2 Cause Code <= 3 && >=1: TLB*/
	else if(excCode <= TLBREFILLEXCEPT)
	{
		passUpOrDie(PGFAULTEXCEPT);
	}
	/*Case 3 Code == 8: SYSCALL*/
	else if(excCode == SYSCALLEXCEPT)
	{
		syscallHandler(oldState->s_a0);
	}
	/*Other: Pass Up or Die*/
	else {
		passUpOrDie(GENERALEXCEPT);
	}
	
	/*Error handling, should not be able to get here, serious issue if this is called*/
	passUpOrDie(GENERALEXCEPT);
	return 0;
}



