/*This module implements device interrupts for the PAND OS*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

extern unsigned int setTIMER(unsigned int t);
void itInterrupt();
void pltInterrupt();
void nonTimerInterrupt(devregarea_t *devRegA, int lineNo);

/*General Interrupt Handler, determines the lineNo of the pending interrupt and moves into the appropriate interrupt handling function*/
void interruptHandler(state_PTR interruptState) {
	
	int cause = interruptState->s_cause;
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	if(cause & ITINTERRUPT)
	{
		itInterrupt();
	}
	if(cause & PLTINTERRUPT)
	{
		pltInterrupt();
	}
	if(cause & DISKINTERRUPT)
	{
		nonTimerInterrupt(devrega, DISKINTERRUPT);
	}
	if(cause & FLASHINTERRUPT)
	{
		nonTimerInterrupt(devrega, FLASHINTERRUPT);
	}
	if(cause & PRINTERINTERRUPT)
	{
		nonTimerInterrupt(devrega, PRINTERINTERRUPT);
	}
	if(cause & TERMINTERRUPT)
	{
		nonTimerInterrupt(devrega, TERMINTERRUPT);
	}
	
}

void nonTimerInterrupt(devregarea_t *devRegA, int lineNo) {

	/*Determine which deviceNo is interrupting*/
	int devNo;
	unsigned int devNoMap = devRegA-> interrupt_dev[lineNo];
	if(devNoMap & DEV0)
		devNo = 0;
	else if(devNoMap & DEV1)
		devNo = 1;
	else if(devNoMap & DEV2)
		devNo = 2;
	else if(devNoMap & DEV3)
		devNo = 3;
	else if(devNoMap & DEV4)
		devNo = 4;
	else if(devNoMap & DEV5)
		devNo = 5;
	else if(devNoMap & DEV6)
		devNo = 6;
	else
		devNo = 7;
	state_PTR exceptionState = (state_PTR) EXCEPTSTATEADDR;
	/*Calculate the address for the device register*/
	unsigned int devAddrBase = 0x10000054 + ((lineNo - 3) * 0x00000080) + (devNo * 0x00000010);
	int devIndex = ((lineNo - 3) * 8) + (devNo);
	
	device_t *devReg = (device_t*) devAddrBase;
	int statusCode;
	/*special case device is a terminal*/
	if(lineNo == 7)
	{
		/*Check if Transmit is interrupting*/
		if(devReg->t_transm_status == READY)
		{
			statusCode = devReg-> t_transm_status;
			devReg->t_transm_command = ACK;
			devIndex = devIndex + 8; /*devIndex is alterred so that it is for a transm devSem*/
		}
		else {
			
			statusCode = devReg-> t_recv_status;
			devReg->t_recv_command = ACK;
		}
		
	}
	/*Not a terminal*/
	else {
	/*save off the status code from the device register*/
	statusCode = devReg->d_status;
	/*ACK the Interrupt*/
	devReg->d_command = ACK;
	}
	int devSem = deviceSema4s[devIndex];
	/*Perform a V operation on the device semaphore*/
	pcb_PTR p;
	devSem++;
	if(devSem >= 0)
	{
		p = removeBlocked(&devSem);
		if(p == NULL)
		{
				exceptionState->s_pc+= PCINCREMENT;
				switchState(exceptionState);
		}
	}
	/*Place the stored off status code in v0*/
	p->p_s.s_v0 = statusCode;
	/*Insert the newly unblocked pcb onto the readyQ*/
	insertProcQ(&readyQ, p);
	/*Return control to the current Process. Perform a LDST on the saved exception state*/
	exceptionState->s_pc += PCINCREMENT;
	switchState(exceptionState);
}




void pltInterrupt() {

	int stopTod;
	/*ACK the Interrupt*/
	setTIMER(TIMESLICE);
	STCK(stopTod);
	/*copy the processor state*/
	state_PTR oldState;
	oldState = (state_PTR)EXCEPTSTATEADDR;
	/*Copy oldState into currentProc->p_s*/
	stateCopy(oldState, &currentProc->p_s); 
	currentProc->p_time = (stopTod - startTod);
	/*place currentProc on the readyQ*/
	insertProcQ(&readyQ, currentProc);
	currentProc = NULL;
	/*call the scheduler*/
	scheduler();
}

void itInterrupt()
{
	state_PTR exceptionState = (state_PTR) EXCEPTSTATEADDR;
	/*ACK the Interrupt by loading 100 milliseconds onto the interval timer*/
	LDIT(ITSECOND);
	/*Unblock ALL Pcbs blocked on clockSem*/
	pcb_PTR p;
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
