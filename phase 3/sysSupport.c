/*This is the start of the sysSupport module for phase 3.*/
/*This module implements the support level general exception handler, the support level SYSCALL exception handler, and the support level Program Trap exception handler*/
static int term_mut = 1;
static int print_mut = 1;

void supGenExceptionHandler() {
	support_t *sup = SYSCALL (GETSUPPORTT, 0, 0, 0);
	int cause = sup->sup_exceptState[0].s_cause & EXMASK;
	if(cause == 8){
		supSyscallHandler(&(sup->exceptionState[0]));
	}
	else {
		programTrapHandler(&(sup->exceptionState[0]));
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
			retValue = writePrinter(exceptState);
		}
		case(TERMW) {
			retValue = writeTerminal(exceptState);
		}
		case(TERMR) {
			retValue = readTerminal(exceptState);
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
	SYSCALL (TERMINATE,0,0,0);
}
/*User mode wrapper for the kernal mode exclusive SYS2*/
void terminateUProc();
{
	
	SYSCALL (TERMINATE, 0, 0, 0);
}

unsigned int getTod() {
	int currentTod;
	STCK(currentTod);
	return currentTod;
}

int writePrinter(state_PTR exceptionState) {
	char *s = msg;
	devregtr * base = (devregtr *) (TERM0ADDR);
	devregtr status;
	
	SYSCALL(PASSERN, (int)&print_mut, 0, 0);				/* P(term_mut) */
	while (*s != EOS) {
		*(base + 3) = PRINTCHR | (((devregtr) *s) << BYTELEN);
		status = SYSCALL(WAITIO, PRINTER, 0, 0);	
		if ((status & TERMSTATMASK) != RECVD)
			/*Store error code*/
		s++;	
	}
	SYSCALL(VERHOGEN, (int)&print_mut, 0, 0);
	return status;
}

int writeTerminal(state_PTR exceptionState) {
	char *s = msg;
	devregtr * base = (devregtr *) (TERM0ADDR);
	devregtr status;
	
	SYSCALL(PASSERN, (int)&term_mut, 0, 0);				/* P(term_mut) */
	while (*s != EOS) {
		*(base + 3) = PRINTCHR | (((devregtr) *s) << BYTELEN);
		status = SYSCALL(WAITIO, TERMINAL, 0, 0);	
		if ((status & TERMSTATMASK) != RECVD)
			/*Store error code*/
		s++;	
	}
	SYSCALL(VERHOGEN, (int)&term_mut, 0, 0);
	return status;
}

int readTerminal(state_PTR exceptionState) {
	char *s = msg;
	devregtr * base = (devregtr *) (TERM0ADDR);
	devregtr status;
	
	SYSCALL(PASSERN, (int)&term_mut, 0, 0);				/* P(term_mut) */
	while (*s != EOS) {
		*(base + 3) = PRINTCHR | (((devregtr) *s) << BYTELEN);
		status = SYSCALL(WAITIO, TERMINAL, 1, 0);	
		if ((status & TERMSTATMASK) != RECVD)
			/*Store error code*/
		s++;	
	}
	SYSCALL(VERHOGEN, (int)&term_mut, 0, 0);
	return status;
}
