/*This module implements device interrupts for the PAND OS*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"

void interruptHandler(state_ptr interruptState) {
	
	int cause = interruptState->s_cause;
	devregarea_t devrega = (*deregarea_t) BUSADDRESS;
	if(cause & ITINTERRUPT)
	{
		itInterrupt(&devrega);
	}
	if(cause & PLTINTERRUPT)
	{
		pltInterrupt(&devrega);
	}
	if(cause & DISKINTERRUPT)
	{
		nonTimerInterrupt(&devrega, DISKINTERRUPT);
	}
	if(cause & FLASHINTERRUPT)
	{
		nonTimerInterrupt(&devrega, FLASHINTERRUPT);
	}
	if(cause & PRINTERINTERRUPT)
	{
		nonTimerInterrupt(&devrega, PRINTERINTERRUPT);
	}
	if(cause & TERMINTERRUPT)
	{
		termInterrupt(&devrega);
	}
	
}

void nonTimerInterrupt(devregarea_t devrega*, int lineNo, int devNo) {

	exceptionState = (state_ptr) EXCEPTSTATEADDR;
	/*Calculate the address for the device register*/
	devAddrBase = 0x10000054 + ((lineNo - 3) * 0x00000080) + (devNo * 0x00000010);
	/*save off the status code from the device register*/
		/*int statusCode = devReg.statusCode;*/
	/*ACK the Interrupt*/
		/*devReg.command = ACK;*/
	/*Perform a V operation on the device semaphore*/
		/*
		pcb_PTR p;
		devSem++;
		if(devSem >= 0)
		{
			p = removeBlocked(&devSem);
		} */
		
	/*Place the stored off status code in v0*/
	p->p_s.s_v0 = statusCode;
	/*Insert the newly unblocked pcb onto the readyQ*/
	insertProcQ(&readyQ, p);
	/*Return control to the current Process. Perform a LDST on the saved exception state*/
	switchState(exceptionState);
}

void pltInterrupt() {
	/*ACK the Interrupt*/
	setTimer(TIMESLICE);
	/*copy the processor state*/
	state_ptr exceptionState = (state_ptr) EXCEPTSTATEADDR;
	currentProc->p_s = exceptionState;
	
	/*place currentProc on the readyQ*/
	insertProcQ(&readyQ, currentProc);
	/*call the scheduler*/
	scheduler();
}

void itInterrupt()
{
	state_ptr exceptionState = (state_ptr) EXCEPTSTATEADDR;
	/*ACK the Interrupt by loading 100 milliseconds onto the interval timer*/
	LDIT(ITSECOND);
	/*Unblock ALL Pcbs blocked on clockSem*/
	p = removeBlocked(&clockSem);
	while(p != NULL)
	{
		insertProcQ(&readyQ, p);
		p = removeBlocked(&clockSem);
	}
	/*Reset ClockSem to 0*/
	clockSem = 0;
	/*Return control to the current process. Perform a LDST on the saved exception state*/
	switchState(exceptionState);
}
