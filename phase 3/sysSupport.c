/*This is the start of the sysSupport module for phase 3.*/
/*This module implements the support level general exception handler, the support level SYSCALL exception handler, and the support level Program Trap exception handler*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "/usr/include/umps3/umps/libumps.h"

debugC(int a) {
	return a;
}
/*Support level general exception handler, covers all non-tlb exceptions in phase 3*/
/*Either passes to syscall handler or program trap handler, based on cause*/
void supGenExceptionHandler() {
	support_t *sup = SYSCALL (GETSUPPORTT, 0, 0, 0);
	int cause = (sup->sup_exceptState[GENERALEXCEPT].s_cause & EXMASK) >> 2;
	debugC(cause);
	if(cause == 8){
		/*Syscall, move into the support level syscall handler*/
		supSyscallHandler(&(sup->sup_exceptState[GENERALEXCEPT]));
	}
	else {
		/*Not a syscall, treat as a program trap*/
		programTrapHandler(&(sup->sup_exceptState[GENERALEXCEPT]));
	}
}

/*Support level syscall handler, defines sys 9-13*/
void supSyscallHandler(state_PTR exceptState) {
	/*Get the relevant syscall code from a0*/
	int sysCode = exceptState->s_a0;
	int retValue;
	switch(sysCode) {
		case(UTERMINATE):
		/*Sys 9*/
			terminateUProc();
			break;
		
		case(GETTOD): /*Sys 10*/
			retValue = (int) getTod();
			break;
		
		case(PRNTRW): /*Sys 11*/
			retValue = writePrinter();
			break;
		
		case(TERMW): /*Sys 12*/
			retValue = writeTerminal();
			break;
		
		case(TERMR): /*Sys 13*/
			retValue = readTerminal();
			break;
		
		default: /*Case other than sys 9-13, treat as a program trap*/
			programTrapHandler(exceptState);
	}
	/*Increment the pc, store the return value in v0, and return to the calling process*/
	exceptState->s_v0 = retValue;
	exceptState->s_pc += WORDLEN;
	LDST(exceptState);
}

/*Terminates calling process and releases mutex if held*/
void programTrapHandler() {
	/*Check if holding mutex on something*/
	SYSCALL(VERHOGEN, &procSem, 0, 0);
	/*terminate*/
	SYSCALL (TERMPROCESS,0,0,0);
}

/*User mode wrapper for the kernal mode exclusive SYS2*/
void terminateUProc()
{
	/*Isomorphic to programTrapHandler, so this function simply calls that*/
	programTrapHandler();
}

/*Returns current time of day*/
unsigned int getTod() {
	int currentTod;
	STCK(currentTod);
	return currentTod;
}

/*Sys 11, writes a string to a printer, printer is selected based on process id*/
/*Returns status code upon completion*/
int writePrinter() {
	/*Turn interrupts off*/
	setSTATUS((getSTATUS() >> 1) << 1);
	/*Get support structure, processor state, device register, and local variables*/
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

/*Sys 12, writes value to a terminal, terminal is selected based on process id, returns status code upon completion*/
int writeTerminal() {
	/*Turn interrupts off*/
	setSTATUS((getSTATUS() >> 1) << 1);
	/*Get support structure, processor state, device register, and local variables*/
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

/*Sys 13, reads value to a terminal, terminal is selected based on process id, returns status code upon completion*/
int readTerminal() {
	/*Turn interrupts off*/
	setSTATUS((getSTATUS() >> 1) << 1);
	/*Get support structure, processor state, device register, and local variables*/
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
