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
static int semCount = 49;
static extern int deviceSema4s[semCount];
void extern test();
#define clockSem deviceSema4s[semCount]

/*
This is the main function for the PANDOS Nucleus. It initializes the pcbs, asl, global variables,
and 
*/
public int main() {

	/*Populate the Processor 0 pass Up Vector*/
	static passUpVector_t pv = (memaddr) PASSUPVECTOR;
	pv->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
	pv->tlb_refll_stackPtr = (memaddr) STACKADDRESS;
	
	/*Set the nucleus exception handler*/
	pv->exception_handler = (memaddr) genExceptionHandler;
	pv->exception_stackPtr = (memaddr) STACKADDRESS;
	
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
	for(i = 0, i < semCount, i++)
	{
		devSema4s[i] = 0;
	} 
	
	/*Load the system wide interval timer with 100 milliseconds*/
	LDIT(100);
	/*Allocate the first pcb and insert it onto the readyQ*/
	tempPcb = allocPcb();
	/*set up the first processor state*/
	tempPcb->p_s.s_sp = RAMTOP;
	tempPcb->p_s.s_pc = (memaddr) test;
	tempPcb->p_s.s_t9 = (memaddr) test;
	tempPcb->p_s.s_status = ALLOFF | IEPON | IMON | TEON;
	insertProcQ(&readyQ,tempPcb);
	
	scheduler();
	return 0;
}

void uTLB_RefillHandler() {
	setENTRYHI (0x80000000);
	setENTRYLO (0x00000000);
	TLBWR();
	LDST((state_PTR) 0x0FFFF000);
}

public int genExceptionHandler() {
	/*Wake up in the general exception handler*/
	/*Check Cause Exception Code*/
	/*Case 1 Cause Code == 0: Interrupts*/
	if(currentProc->p_s.s_cause.ExcCode == 0)
	{
	
	
	}
	/*Case 2 Cause Code <= 3 && >=1: TLB*/
	if(currentProc.p_s.s_cause.ExcCode <= 3)
	{
	
	}
	/*Case 3 Code >= 4 <=12 !=8: Prgm Traps*/
	if(currentProc->p_s.s_cause.ExcCode != 8)
	{
	
	}
	/*Case 4 Code == 8: SYSCALL*/
	if(currentProc->p_s.s_cause.ExcCode == 8)
	{
		syscallHandler(currentProc->p_s.a0);
	}
	/*Other: Pass Up or Die*/
	else {
		passUpOrDie();
	}
	


