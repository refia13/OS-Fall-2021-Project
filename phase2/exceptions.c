/*This will be the exceptions handling file. It tells the OS what to do when syscalls and exceptions occur*/
/*Included header files for use within exceptions.c*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

void createProcess();
void terminateProcess(pcb_PTR current);
void passeren();
void verhogen();
void waitForDevice();
void waitForClock();
void tlbRefillHandler();
void programTrapHandler();

void debugC(int a) {
	int i = 0;
	i--;
}

/*Single parameter method to handle syscalls*/
void syscallHandler(int syscallCode)
{
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	
	/*Checks current state of program, to see if currently in user mode*/
	if(oldState->s_status & KUPON)
	{
		/*Attempted a syscall in user mode, triggger a program trap*/
		programTrapHandler();
	}
	
	/*Switch case using parameter value to determine current syscall code, then calls other methods to act*/
	switch(syscallCode) 
	{
		/*SYS1 Create Process*/
		case CREATEPROCESS: {
			/*Creates a new process for use*/
			createProcess(); 
			break;}
			
		/*SYS2 Terminate Process*/	
		case TERMPROCESS: {
			/*Terminates the current process and all of its progeny*/
			terminateProcess(currentProc); 
			break;}
			
		/*SYS3 Passaren*/
		case PASSEREN: {
			/*Perform a P operation on a semaphore*/
			/*NOTE TO SELF: write a better comment*/
			passeren(); 
			break;}
			
		/*SYS4 Verhogen*/	
		case VERHOGEN: {
			/*Perform a V operation on a semaphore*/
			/*NOTE TO SELF: write a better comment*/
			verhogen(); 
			break;}
			
		/*SYS5 Wait for IO*/
		case WAITFORIO: {
			/*Waits for input or output from a device*/
			/*NOTE TO SELF: check what this function does, write better comment*/
			waitForDevice(); 
			break;}
			
		/*SYS6 Get CPU Time*/
		case GETCPUT: {
			oldState->s_v0 = (currentProc->p_time);
			oldState->s_pc += WORDLEN;
			stateCopy(oldState, &(currentProc->p_s));
			switchState(&(currentProc->p_s));
			break;
			}
			
		/*SYS7 Wait for Clock*/
		case WAITFORCLOCK: {
			/*Performs a P operation on the pseudo-clock semaphore and blocks the current process*/
			waitForClock(); 
			break;}
			
		/*SYS8 Get Support Data*/
		case GETSUPPORTT: {
			oldState->s_v0 = (currentProc->p_supportStruct);
			oldState->s_pc += WORDLEN;
			stateCopy(oldState, &(currentProc->p_s));
			switchState(&(currentProc->p_s));
			break;
			}
			
		/*Sycall code is greater than 8*/
		default: {			
			/*Pass up or Die*/
			passUpOrDie(GENERALEXCEPT); }
	}
	if(currentProc != NULL) {
		switchState(&(currentProc->p_s));
	}
	else{
		scheduler();
	}
	

}

/*Creates a new process for use*/
void createProcess() {

	state_PTR exceptState = (state_PTR)EXCEPTSTATEADDR;
	pcb_PTR newProc = allocPcb();
	int result = ERRORCODE;
	if(newProc != NULL) {
		processCount++;
		state_PTR newState = (state_PTR)(exceptState->s_a1);
		stateCopy(newState, &(newProc->p_s));
		newProc->p_supportStruct = exceptState->s_a2;
		newProc->p_time = 0;
		newProc->p_semAdd = NULL;
		insertChild(currentProc, newProc);
		insertProcQ(&readyQ, newProc);
		result = SUCCESS;
	}
	exceptState->s_v0 = result;
	exceptState->s_pc += WORDLEN;
	if(currentProc == NULL) {
		scheduler();
	}
	stateCopy(exceptState, &(currentProc->p_s));
	
	switchState(&(currentProc->p_s));
}

/*Terminate the current process, and all of its children*/
void terminateProcess(pcb_PTR current) {
	/*Checks if the current node has a child*/
	while(!emptyChild(current)) 
	{
		/*Recurses, using the current node's child as a parameter input*/
		terminateProcess(removeChild(current));
	}
	
	if(current == currentProc) {
		outChild(current);
		currentProc = NULL;
		processCount --;
		scheduler();
	}
	if(current->p_semAdd != NULL) {
		if(!(current->p_semAdd <= &deviceSema4s[0] && current->p_semAdd >= &clockSem))
		{
			int *sem = current->p_semAdd;
			(*sem)++;
		}
		pcb_PTR p = outBlocked(current->p_semAdd);
		processCount--;
	}
	else {
		pcb_PTR p = outProcQ(&readyQ, current);
		if(p != NULL) { processCount--; }
	}
	freePcb(current);
}

/*Performs a P operation on a semaphore*/
/*NOTE TO SELF: really need to figure out what this means*/
void passeren() {
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	
	int *sem = oldState->s_a1;
	(*sem)--;
	
	oldState->s_pc+= WORDLEN;
	stateCopy(oldState, &(currentProc->p_s));
	if( (*sem) < 0) {
		insertBlocked(sem, currentProc);
		currentProc = NULL;
		scheduler();
	}
	switchState(&(currentProc->p_s));
	
}

/*Performs a V operation on a semaphore*/
/*NOTE TO SELF: find out what this means*/
void verhogen() {
	
	
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	int *sem = oldState->s_a1;
	(*sem)++;
	pcb_PTR p;
	if((*sem) <= 0) {
		debugC(3);
		p = removeBlocked(sem);
		p->p_s.s_v0 = sem;
		if(p != NULL) {
			insertProcQ(&readyQ, p);
		}
	}
	oldState->s_v0 = sem;
	oldState->s_pc+= WORDLEN;
	
	stateCopy(oldState, &(currentProc->p_s));
	switchState(&(currentProc->p_s));
}

/*Waits for input or output from a device*/
void waitForDevice() {

	int stopTod;
	STCK(stopTod);
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	int lineNo = oldState->s_a1;
	int devNo = oldState->s_a2;
	int waitForTermRead = oldState->s_a3;
	int devSemIndex = (lineNo-3)*8 + devNo;
	if(waitForTermRead == TRUE) {
		devSemIndex += 8;
	}
	deviceSema4s[devSemIndex]--;
	if(deviceSema4s[devSemIndex] < 0) {
		
		oldState->s_pc += WORDLEN;
		stateCopy(oldState, &(currentProc->p_s));
		currentProc->p_time += (stopTod - startTod);
		insertBlocked(&(deviceSema4s[devSemIndex]), currentProc);
		
		currentProc = NULL;
		softBlockCount++;
		scheduler();
	}
}

/*Performs a P operation on the pseudo-clock semaphore and blocks the current process*/
void waitForClock() {
	int stopTod;
	STCK(stopTod);
	currentProc->p_time += (stopTod - startTod);
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	clockSem--;
	
	if(clockSem < 0) {
		softBlockCount++;
		oldState->s_pc += WORDLEN;
		stateCopy(oldState, &(currentProc->p_s));
		insertBlocked(&clockSem, currentProc);
		currentProc = NULL;
	}
	STCK(startTod);
	scheduler();
}


void passUpOrDie(unsigned int passUpCase)
{
	/*Support_t is not NULL*/
	if(currentProc->p_supportStruct != NULL) {
		/*Pass Up*/
		state_PTR exceptStatePtr = (state_PTR) EXCEPTSTATEADDR;
		stateCopy(exceptStatePtr, &(currentProc->p_supportStruct->sup_exceptState[passUpCase]));
		int newPc = currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_pc;
		int newStackPtr = currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_stackPtr;
		int newStatus = currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_status;
		LDCXT(newStackPtr, newStatus, newPc);
	}
	else {/*Die*/
		/*Terminate the current process*/
		terminateProcess(currentProc); }
}
/*Nucleus level programTrapHandler, performs a passUpOrDie operation with the value GENERALEXCEPT*/
void programTrapHandler() {
	debugC(4000);
	passUpOrDie(GENERALEXCEPT);
}
/*Nucleus level tlbException Handler, performs a passUpOrDie operation with the value PGFAULTEXCEPT*/
void tlbExceptionHandler() {
	passUpOrDie(PGFAULTEXCEPT);
}



