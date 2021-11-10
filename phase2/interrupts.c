/*This module implements device interrupts for the PAND OS*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

/*External function provided by umps, loads plt with given value*/
extern unsigned int setTIMER(unsigned int t);
/*Interval timer interrupt handler*/
void itInterrupt();
/*Process local timer interrupt handler*/
void pltInterrupt();
/*Peripheral device interrupt handler*/
void nonTimerInterrupt(devregarea_t *devRegA, int lineNo);

/*General Interrupt Handler, determines the lineNo of the pending interrupt and moves into the appropriate interrupt handling function*/
void interruptHandler(state_PTR interruptState) {
	/*Computes line number to ensure lower line number interrupts have higher priority*/
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
	if(cause & NETWORKINTERRUPT) {
		nonTimerInterrupt(devrega, NETWORK);
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

/*Peripheral device interrupt handler, takes pointer to dev register area and line number*/
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
	
	/*Compute index of device semaphore and device register*/
	int devIndex = ((lineNo - 3) * 8) + (devNo);
	int statusCode;
	
	/*Special case device is a terminal*/
	if(lineNo == 7)
	{
		statusCode = devRegA->devreg[devIndex].t_transm_status & 0xFF;
		if(statusCode != READY) {
			/*Transmit was interrupting, send an ACK*/
			devRegA->devreg[devIndex].t_transm_command = ACK;
		}
		else{
			/*Recieve was interrupting, store off status then send an ACK, increment devIndex for appropriate dev semaphore*/
			statusCode = devRegA->devreg[devIndex].t_recv_status & 0xFF;
			devRegA->devreg[devIndex].t_recv_command = ACK;
			devIndex+= 8;
		}
		
	}else {
	/*Not a terminal*/
		/*Store off status and send an ACK*/
		statusCode = devRegA->devreg[devIndex].d_status;
		devRegA->devreg[devIndex].d_command = ACK;
	}
	
	/*Perform a v operation on device semaphore*/
	pcb_PTR p;
	deviceSema4s[devIndex]++;
	if(deviceSema4s[devIndex] <= 0) {
		p = removeBlocked(&(deviceSema4s[devIndex]));
		if(p != NULL) {
			/*Decrement softBlockCount if process was removed from waiting list, store status code in p's v0*/
			softBlockCount--;	
			p->p_s.s_v0 = statusCode;
			insertProcQ(&readyQ, p);
		}
	}
	
	/*No process to return to, call scheduler*/
	if(currentProc == NULL) {
		scheduler();
	}
	/*Copy state then load state*/
	stateCopy(exceptionState,&(currentProc->p_s));
	switchState(&(currentProc->p_s));
}


 
/*Process local timer interrupt handler, handles interrupt every time slice*/
void pltInterrupt() {
	/*Stores current time of day to track interval*/
	int stopTod;
	STCK(stopTod);
	
	if(currentProc != NULL) {
		/*Stores accumulated time into currentProc and inserts it onto ready queue*/
		stateCopy((state_PTR)EXCEPTSTATEADDR, &(currentProc->p_s));
		currentProc->p_time += (stopTod - startTod);
		insertProcQ(&readyQ, currentProc);
		currentProc = NULL;
	}
	
	/*Start new interval, ACK interrupt, call scheduler*/
	STCK(startTod);
	setTIMER(TIMESLICE);
	scheduler();
}

/*Interval timer interrupt handler, occurs every ITSECOND*/
void itInterrupt()
{
	/*ACK interrupt by loading ITSECOND into interval timer*/
	LDIT(ITSECOND);
	
	/*Remove all processes waiting on clock*/
	pcb_PTR p = removeBlocked(&clockSem);
	while(p != NULL) {
		insertProcQ(&readyQ, p);
		softBlockCount--;
		p = removeBlocked(&clockSem);
	}
	
	/*Reset clock semaphore to 0*/
	clockSem = 0;
	
	/*No process to return to, call scheduler*/
	if(currentProc == NULL) {
		scheduler();
	}
	
	/*Copy state, insert current process to ready queue, and call scheduler*/
	stateCopy((state_PTR)EXCEPTSTATEADDR, &(currentProc->p_s));
	insertProcQ(&readyQ, currentProc);
	scheduler();
}



