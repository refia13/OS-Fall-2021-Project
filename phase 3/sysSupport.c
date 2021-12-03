/*This is the start of the sysSupport module for phase 3.*/
/*This module implements the support level general exception handler, the support level SYSCALL exception handler, and the support level Program Trap exception handler*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"

void supGenExceptionHandler() {
	support_t *sup = SYSCALL (GETSUPPORTT, 0, 0, 0);
	int cause = sup->sup_exceptState[1].s_cause & EXMASK;
	if(cause == 8){
		supSyscallHandler(&(sup->exceptionState[1]));
	}
	else {
		programTrapHandler(&(sup->exceptionState[1]));
	}
}

void supSyscallHandler(state_PTR exceptState) {
	int sysCode = exceptState.s_a0;
	int retValue;
	switch(sysCode) {
		case(UTERMINATE) {
			terminateUProc();
		}
		case(GETTOD) {
			retValue = (int) getTod();
		}
		case(PRNTRW) {
			retValue = writePrinter();
		}
		case(TERMW) {
			retValue = writeTerminal();
		}
		case(TERMR) {
			retValue = readTerminal();
		}
		default {
			programTrapHandler(exceptState);
		}
	}
	exceptionState->s_v0 = retValue;
	exceptionState->s_pc += WORDLEN;
	LDST(exceptionState);
}

void programTrapHandler(state_PTR exceptState) {
	/*Check if holding mutex on something*/
	
	/*terminate*/
	SYSCALL (TERMPROCESS,0,0,0);
}
/*User mode wrapper for the kernal mode exclusive SYS2*/
void terminateUProc();
{
	
	SYSCALL (TERMPROCESS, 0, 0, 0);
}

unsigned int getTod() {
	int currentTod;
	STCK(currentTod);
	return currentTod;
}

int writePrinter() {
	/*Turn interrupts off*/
	setStatus(getSTATUS() & IECOFF);
	/*Determine device register*/
	support_t *sup = SYCALL(GETSUPPORTT, 
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	int devIndex = ((PRINTER - NONPERIPHERALDEV) * DEVPERLINE) + (asid);
	support_t *sysSup = SYSCALL(GETSUPPORTT, 0, 0, 0);
	state_PTR sysState = sysSup->sup_exceptState[GENERALEXCEPT];
	char *s = sysState->s_a1;
	int length = sysState->s_a2;
	int asid = sysSup->sup_asid;
	if(length < 0 || length > 128) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	if(s < 0xC0000000) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	int i = 0;
	SYSCALL(PASSEREN, &(devMutex[devIndex],0,0));
	while(i < length) {
		devrega->devreg[devIndex].d_data0 = (*s);
		devrega->devreg[devIndex].d_command = PRINTCHR;
		int retCode = SYSCALL(WAITFORIO, PRINTER, asid, 0);
		if(retCode != 1) {
			return -1*retCode;
		}
		i++;
		s++;
	}
	SYSCALL(VERHOGEN, &(devMutex[devIndex],0,0));
	return retCode;
	
}

int writeTerminal() {
	/*Turn interrupts off*/
	setStatus(getSTATUS() & IECOFF);
	/*Determine device register*/
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	int devIndex = ((TERMINAL - NONPERIPHERALDEV) * DEVPERLINE) + (asid);
	support_t *sysSup = SYSCALL(GETSUPPORTT, 0, 0, 0);
	state_PTR sysState = sysSup->sup_exceptState[GENERALEXCEPT];
	char *s = sysState->s_a1;
	int length = sysState->s_a2;
	int asid = sysSup->sup_asid;
	if(length < 0 || length > 128) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	if(s < 0xC0000000) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	int i = 0;
	SYSCALL(PASSEREN, &(devMutex[devIndex],0,0));
	while(i < length) {
		devrega->devreg[devIndex].t_transm_command = PRINTCHR | (((devregtr) *s) << BYTELEN);
		int retCode = SYSCALL(WAITFORIO, TERMINAL, asid, 0);
		if(retCode != 1) {
			return -1*retCode;
		}
		i++;
		s++;
	}
	SYSCALL(VERHOGEN, &(devMutex[devIndex],0,0));
	return retCode;
	
}

int readTerminal() {
	/*Turn interrupts off*/
	setStatus(getSTATUS() & IECOFF);
	/*Determine device register*/
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	int devIndex = ((TERMINAL - NONPERIPHERALDEV) * DEVPERLINE) + (asid);
	support_t *sysSup = SYSCALL(GETSUPPORTT, 0, 0, 0);
	state_PTR sysState = sysSup->sup_exceptState[GENERALEXCEPT];
	char *s = sysState->s_a1;
	int length = sysState->s_a2;
	int asid = sysSup->sup_asid;
	if(length < 0 || length > 128) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	if(s < 0xC0000000) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	int i = 0;
	SYSCALL(PASSEREN, &(devMutex[devIndex],0,0));
	while(i < length) {
		devrega->devreg[devIndex].t_recv_command = PRINTCHR | (((devregtr) *s) << BYTELEN);
		int retCode = SYSCALL(WAITFORIO, TERMINAL, asid, 1);
		if(retCode != 1) {
			return -1*retCode;
		}
		i++;
		s++;
	}
	SYSCALL(VERHOGEN, &(devMutex[devIndex+8],0,0));
	return retCode;
	
}
