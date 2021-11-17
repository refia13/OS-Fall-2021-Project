/*This is the start of the sysSupport module for phase 3.*/
/*This module implements the support level general exception handler, the support level SYSCALL exception handler, and the support level Program Trap exception handler*/


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

int writePrinter() {

}

int writeTerminal() {

}

int readTerminal() {
	
}
