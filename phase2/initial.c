/*This module initializes the nucleus of the PAND OS*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

extern int processCount;
extern int blockedCount;
extern pcb_PTR readyQ;
extern pcb_PTR currentProc;
extern void test();
extern int startTod;
extern void uTLB_RefillHandler();
extern int genExceptionHandler();

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
	blockedCount = 0;
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
	tempPcb->p_s.s_sp = ramtop;
	tempPcb->p_s.s_pc = (memaddr) test;
	tempPcb->p_s.s_t9 = (memaddr) test;
	tempPcb->p_s.s_status = ALLOFF | IEPON | IMON | TEON;
	insertProcQ(&readyQ,tempPcb);
	
	scheduler();
	return 0;
}



int genExceptionHandler() {
	/*Wake up in the general exception handler*/
	/*Check Cause Exception Code*/
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	int excCode = ((oldState->s_cause & EXMASK) >> 2);
	/*Case 1 Cause Code == 0: Interrupts*/
	if(excCode == IOEXCEPT)
	{
		interruptHandler(oldState);
	}
	/*Case 2 Cause Code <= 3 && >=1: TLB*/
	if(excCode <= TLBREFILLEXCEPT)
	{
		uTLB_RefillHandler();
	}
	/*Case 3 Code == 8: SYSCALL*/
	if(excCode == SYSCALLEXCEPT)
	{
		syscallHandler(oldState->s_a0);
	}
	/*Other: Pass Up or Die*/
	else {
		passUpOrDie(GENERALEXCEPT);
	}
	
	programTrapHandler();
	return 0;
}

