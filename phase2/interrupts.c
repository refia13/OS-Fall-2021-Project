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

void debugD(int a) {
	int i = 0;
	i++;
}

/*General Interrupt Handler, determines the lineNo of the pending interrupt and moves into the appropriate interrupt handling function*/
void interruptHandler(state_PTR interruptState) {
	
	int cause = interruptState->s_cause;
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	if(cause & PLTINTERRUPT)
	{
		pltInterrupt();
	}
	if(cause & ITINTERRUPT)
	{
		itInterrupt();
	}
	
	if(cause & DISKINTERRUPT)
	{
		nonTimerInterrupt(devrega, DISK);
	}
	if(cause & FLASHINTERRUPT)
	{
		nonTimerInterrupt(devrega, FLASH);
	}
	if(cause & PRINTERINTERRUPT)
	{
		nonTimerInterrupt(devrega, PRINTER);
	}
	if(cause & TERMINTERRUPT)
	{
		nonTimerInterrupt(devrega, TERMINAL);
	}
	
}

void nonTimerInterrupt(devregarea_t *devRegA, int lineNo) {

	/*Determine which deviceNo is interrupting*/
	int devNo;
	
	int devNoMap = devRegA->interrupt_dev[lineNo-3];
	
	if(devNoMap & DEV0) {
		devNo = 0; }
	else if(devNoMap & DEV1) {
		devNo = 1; }
	else if(devNoMap & DEV2) {
		devNo = 2; }
	else if(devNoMap & DEV3) {
		devNo = 3; }
	else if(devNoMap & DEV4) {
		devNo = 4; }
	else if(devNoMap & DEV5) {
		devNo = 5; }
	else if(devNoMap & DEV6) {
		devNo = 6; }
	else if(devNoMap & DEV7) {
		devNo = 7; }

	state_PTR exceptionState = (state_PTR) EXCEPTSTATEADDR;
	/*Calculate the address for the device register*/
	int devIndex = ((lineNo - 3) * 8) + (devNo);
	int statusCode;
	
	/*special case device is a terminal*/
	if(lineNo == 7)
	{
		statusCode = devRegA->devreg[devIndex].t_transm_status;
		if(statusCode != READY) {
			
			devRegA->devreg[devIndex].t_transm_command = ACK;
		}
		else{
			statusCode = devRegA->devreg[devIndex].t_recv_status & 0xFF;
			devRegA->devreg[devIndex].t_recv_command = ACK;
			devIndex+= 8;
		}
		
	}
	else {
	/*Not a terminal*/
		statusCode = devRegA->devreg[devIndex].d_status;
		devRegA->devreg[devIndex].d_command = ACK;
	}
	pcb_PTR p;
	deviceSema4s[devIndex]++;

	if(deviceSema4s[devIndex] <= 0) {
		p = removeBlocked(&(deviceSema4s[devIndex]));
		if(p != NULL) {
			
			p->p_s.s_v0 = statusCode;
			softBlockCount--;
			insertProcQ(&readyQ, p);
		}
	}
	exceptionState->s_pc += WORDLEN;
	if(currentProc == NULL) {
		
		scheduler();
	}
	
	switchState(exceptionState);
}


 

void pltInterrupt() {

	int stopTod;
	STCK(stopTod);
	setTIMER(TIMESLICE);
	if(currentProc != NULL) {
		stateCopy((state_PTR)EXCEPTSTATEADDR, &(currentProc->p_s));
		currentProc->p_time += (stopTod - startTod);
		insertProcQ(&readyQ, currentProc);
		currentProc = NULL;
	}
	STCK(startTod);
	scheduler();
}

void itInterrupt()
{
	LDIT(ITSECOND);
	pcb_PTR p = removeBlocked(&clockSem);
	while(p != NULL) {
		insertProcQ(&readyQ, p);
		softBlockCount--;
		p = removeBlocked(&clockSem);
	}
	clockSem = 0;
	if(currentProc != NULL) {
		scheduler();
	}
	switchState((state_PTR)EXCEPTSTATEADDR);
}



