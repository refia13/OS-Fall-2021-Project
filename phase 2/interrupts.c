/*This module implements device interrupts for the PAND OS*/
/*Included files for use within interrupts.c*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

/*Determines actions to take based on the current interrupt state*/
void interruptHandler(state_ptr interruptState) {
	
	/*Stores cause register of exception state for ease of use*/
	int cause = interruptState->s_cause;
	/*Stores bus register area for compatibility with the interrupt handler*/
	devregarea_t devrega = (*deregarea_t) BUSADDRESS;
	/*If the cause is an interval timer interrupt*/
	if(cause & ITINTERRUPT)
	{
		itInterrupt(&devrega);
	}
	/*If the cause is a process level timer interrupt*/
	if(cause & PLTINTERRUPT)
	{
		pltInterrupt(&devrega);
	}
	/*If the cuase is a disk interrupt*/
	if(cause & DISKINTERRUPT)
	{
		nonTimerInterrupt(&devrega, DISKINTERRUPT);
	}
	/*If the cause is a flash interrupt*/
	if(cause & FLASHINTERRUPT)
	{
		nonTimerInterrupt(&devrega, FLASHINTERRUPT);
	}
	/*If the cause is a printer interrupt*/
	if(cause & PRINTERINTERRUPT)
	{
		nonTimerInterrupt(&devrega, PRINTERINTERRUPT);
	}
	/*If the cause is a terminal interrupt*/
	if(cause & TERMINTERRUPT)
	{
		termInterrupt(&devrega);
	}
	
}

/*Handles all interrupts that do not involve timers*/
void nonTimerInterrupt(devregarea_t devRegA*, int lineNo) {

	/*Determine which deviceNo is interrupting*/
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
		devNo = 5
	else if(devNoMap & DEV6)
		devNo = 6
	else
		devNo = 7
		
	/*Stores the exception state for ease of use*/	
	exceptionState = (state_ptr) EXCEPTSTATEADDR;
	/*Calculate the address for the device register*/
	devAddrBase = 0x10000054 + ((lineNo - 3) * 0x00000080) + (devNo * 0x00000010);
	/*Calculate the index for the device register*/
	int devIndex = ((lineNo - 3) * 8) + (devNo);
	/*Stores the needed device register*/
	device_t *devReg = (*device_t) devAddrBase;
	
	/*Special case if device is a terminal*/
	if(lineNo == 7)
	{
		/*Check if Transmit is interrupting*/
		if(devReg->t_transm_status == READY)
		{
			/*Stores transmission status code*/
			statusCode = devReg-> t_transm_status;
			/*Acknowledge interrupt via transmission*/
			devReg->t_transm_command = ACK;
			/*devIndex is altered so that it is for a transmit device semaphore*/
			devIndex = devIndex + 8; 
		}
		else {
			/*Store reception status code*/
			statusCode = devReg-> t_recv_status;
			/*Acknowledge interrupt via reception*/
			devReg->t_recv_command = ACK;
		}
		
	}
	
	/*If not a terminal*/
	else {
		/*Store the status code from the device register*/
		unsigned int statusCode = devReg->d_status;
		/*Acknowledge the interrupt*/
		devReg->d_command = ACK;
	}
	/*Store the device semaphore for ease of use*/
	int devSem = deviceSema4s[devIndex];
	
/*Perform a V operation on the device semaphore*/
	/*NOTE TO WILL: fix this shit*/
	pcb_PTR p;
	/*Increment the device semaphore*/
	devSem++;
	/*If the device semaphore is equal to or greater than zero*/
	if(devSem >= 0)
	{
		/*Remove device semaphore from blocked list and store in p*/
		p = removeBlocked(&devSem);
		/*If p has no value*/
		if(p == NULL)
		{
			/*Increments the exception state*/
			exceptionState->s_pc+= PCINCREMENT;
			/*Switch the state of the exception state*/
			switchState(&exceptionState);
		}
	}
	/*Place the stored status code in v0*/
	p->p_s.s_v0 = statusCode;
	/*Insert the newly unblocked pcb onto the readyQ*/
	insertProcQ(&readyQ, p);
	/*Return control to the current Process. Perform a LDST on the saved exception state*/
	exceptionState->s_pc += PCINCREMENT;
	/*Switch the state of the saved exception state*/
	switchState(exceptionState);
	}
	/*Case where the interrupting device is a terminal*/
	/*Need to determine whether it is the read or write device*/
	/*Process write device first*/
}



void pltInterrupt() {

	/*Variable to store the time of the TOD to calculate the accumulated time later*/
	int stopTod;
	/*Sets timer to 5 milliseconds*/
	setTimer(TIMESLICE);
	/*Stores the current clock time*/
	STCK(stopTod);
	/*Stores pointer to the current exception state*/
	state_ptr exceptionState = (state_ptr) EXCEPTSTATEADDR;
	/*Sets state of current process to the exception state*/
	currentProc->p_s = exceptionState;
	/*Calculates current process' accumulated time*/
	currentProc->p_time = (stopTod - startTod);
	/*place currentProc on the readyQ*/
	insertProcQ(&readyQ, currentProc);
	/*Makes sure there's nothing currently running before calling scheduler*/
	currentProc = NULL;
	/*call the scheduler*/
	scheduler();
}

void itInterrupt()
{
	/*Store pointer to the current exception state*/
	state_ptr exceptionState = (state_ptr) EXCEPTSTATEADDR;
	/*Load 100 milliseconds onto the interval timer*/
	LDIT(ITSECOND);
/*Unblock ALL Pcbs blocked on clockSem*/
	/*p is the first removed value from the blocked list*/
	p = removeBlocked(&clockSem);
	/*While p has a value*/
	while(p != NULL)
	{	
		/*Insert p into the ready queue*/
		insertProcQ(&readyQ, p);
		/*Collect next value from the blocked list, or NULL if list is empty*/
		p = removeBlocked(&clockSem);
	}
	/*Reset clock semaphore to 0*/
	clockSem = 0;
	/*Return control to the current process. Perform an LDST on the saved exception state*/
	switchState(exceptionState);
}
