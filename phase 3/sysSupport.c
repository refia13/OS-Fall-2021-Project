/*This is the start of the sysSupport module for phase 3.*/
/*This module implements the support level general exception handler, the support level SYSCALL exception handler, and the support level Program Trap exception handler*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "/usr/include/umps3/umps/libumps.h"

void supGenExceptionHandler() {
	support_t *sup = SYSCALL (GETSUPPORTT, 0, 0, 0);
	int cause = sup->sup_exceptState[1].s_cause & EXMASK;
	if(cause == 8){
		supSyscallHandler(&(sup->sup_exceptState[GENERALEXCEPT]));
	}
	else {
		programTrapHandler(&(sup->sup_exceptState[GENERALEXCEPT]));
	}
}

void supSyscallHandler(state_PTR exceptState) {
	int sysCode = exceptState->s_a0;
	int retValue;
	switch(sysCode) {
		case(UTERMINATE):
			terminateUProc();
			break;
		
		case(GETTOD):
			retValue = (int) getTod();
			break;
		
		case(PRNTRW):
			retValue = writePrinter();
			break;
		
		case(TERMW):
			retValue = writeTerminal();
			break;
		
		case(TERMR):
			retValue = readTerminal();
			break;
		
		default:
			programTrapHandler(exceptState);
	}
	exceptState->s_v0 = retValue;
	exceptState->s_pc += WORDLEN;
	LDST(exceptState);
}

void programTrapHandler(state_PTR exceptState) {
	/*Check if holding mutex on something*/
	
	/*terminate*/
	SYSCALL (TERMPROCESS,0,0,0);
}
/*User mode wrapper for the kernal mode exclusive SYS2*/
void terminateUProc()
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
	setSTATUS((getSTATUS() >> 1) << 1);
	/*Determine device register*/
	support_t *sysSup = SYSCALL(GETSUPPORTT, 0, 0, 0);
	state_PTR sysState = &sysSup->sup_exceptState[GENERALEXCEPT];
	char *s = sysState->s_a1;
	int length = sysState->s_a2;
	int asid = sysSup->sup_asid;
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	int devIndex = ((TERMINAL - NONPERIPHERALDEV) * DEVPERLINE) + (asid);
	/*Check if string is within the acceptable length*/
	if(length < 0 || length > 128) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	/*Check if bad addr*/
	if(s < 0xC0000000) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	int i = 0;
	int retCode;
	/*Gain Mutex*/
	SYSCALL(PASSEREN, &(devMutex[devIndex]),0,0);
	while(i < length) {
	/*Loop through string until entire string has been sent to the device*/
		devrega->devreg[devIndex].d_data0 = (*s);
		devrega->devreg[devIndex].d_command = PRINTCHR;
		retCode = SYSCALL(WAITFORIO, PRINTER, asid, 0);
		if(retCode != 1) {
			return -1*retCode;
		}
		i++;
		s++;
	}
	SYSCALL(VERHOGEN, &(devMutex[devIndex]),0,0);
	/*Turn Interrupts On*/
	setSTATUS(getSTATUS() & IECON);
	return retCode;
	
}

int writeTerminal() {
	/*Turn interrupts off*/
	setSTATUS((getSTATUS() >> 1) << 1);
	support_t *sysSup = SYSCALL(GETSUPPORTT, 0, 0, 0);
	state_PTR sysState = &sysSup->sup_exceptState[GENERALEXCEPT];
	char *s = sysState->s_a1;
	int length = sysState->s_a2;
	int asid = sysSup->sup_asid;
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	int devIndex = ((TERMINAL - NONPERIPHERALDEV) * DEVPERLINE) + (asid);
	/*Check if string is within the acceptable length*/
	if(length < 0 || length > 128) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	/*check if bad addr*/
	if(s < 0xC0000000) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	int i = 0;
	int retCode;
	/*Gain Mutex*/
	SYSCALL(PASSEREN, &(devMutex[devIndex]),0,0);
	while(i < length) {
		/*Loop through string until entire string has been sent to the device*/
		devrega->devreg[devIndex].t_transm_command = PRINTCHR | ((*s) << BYTELEN);
		retCode = SYSCALL(WAITFORIO, TERMINAL, asid, 0);
		if(retCode != 1) {
			return -1*retCode;
		}
		i++;
		s++;
	}
	SYSCALL(VERHOGEN, &(devMutex[devIndex]),0,0);
	/*Turn Interrupts On*/
	setSTATUS(getSTATUS() & IECON);
	return retCode;
	
}

int readTerminal() {
	/*Turn interrupts off*/
	setSTATUS((getSTATUS() >> 1) << 1);
	/*Determine device register*/
	
	support_t *sysSup = SYSCALL(GETSUPPORTT, 0, 0, 0);
	state_PTR sysState = &sysSup->sup_exceptState[GENERALEXCEPT];
	char *s = sysState->s_a1;
	int length = sysState->s_a2;
	int asid = sysSup->sup_asid;
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	int devIndex = ((TERMINAL - NONPERIPHERALDEV) * DEVPERLINE) + (asid);
	/*Check if string is within the acceptable length*/
	if(length < 0 || length > 128) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	/*Check if bad addr*/
	if(s < 0xC0000000) {
		SYSCALL(TERMPROCESS,0,0,0);
	}
	int i = 0;
	int retCode;
	/*Gain Mutex*/
	SYSCALL(PASSEREN, &(devMutex[devIndex]),0,0);
	while(i < length) {
		/*Loop through string until entire string has been sent to the device*/
		devrega->devreg[devIndex].t_recv_command = PRINTCHR | ((*s) << BYTELEN);
		retCode = SYSCALL(WAITFORIO, TERMINAL, asid, 1);
		if(retCode != 1) {
			return -1*retCode;
		}
		i++;
		s++;
	}
	/*Release Mutex*/
	SYSCALL(VERHOGEN, &(devMutex[devIndex+8]),0,0);
	/*Turn Interrupts On*/
	setSTATUS(getSTATUS() & IECON);
	return retCode;
	
}
