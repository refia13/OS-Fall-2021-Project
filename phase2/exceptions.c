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
			createProcess(); }
			
		/*SYS2 Terminate Process*/	
		case TERMPROCESS: {
			/*Terminates the current process and all of its progeny*/
			terminateProcess(currentProc); }
			
		/*SYS3 Passaren*/
		case PASSEREN: {
			/*Perform a P operation on a semaphore*/
			/*NOTE TO SELF: write a better comment*/
			passeren(); }
			
		/*SYS4 Verhogen*/	
		case VERHOGEN: {
			/*Perform a V operation on a semaphore*/
			/*NOTE TO SELF: write a better comment*/
			verhogen(); }
			
		/*SYS5 Wait for IO*/
		case WAITFORIO: {
			/*Waits for input or output from a device*/
			/*NOTE TO SELF: check what this function does, write better comment*/
			waitForDevice(); }
			
		/*SYS6 Get CPU Time*/
		case GETCPUT: {
			oldState->s_v0 = (currentProc->p_time);
			oldState->s_pc += WORDLEN;
			stateCopy(oldState, &(currentProc->p_s));
			switchState(&(currentProc->p_s));
			}
			
		/*SYS7 Wait for Clock*/
		case WAITFORCLOCK: {
			/*Performs a P operation on the pseudo-clock semaphore and blocks the current process*/
			waitForClock(); }
			
		/*SYS8 Get Support Data*/
		case GETSUPPORTT: {
			oldState->s_v0 = (currentProc->p_supportStruct);
			oldState->s_pc += WORDLEN;
			stateCopy(oldState, &(currentProc->p_s));
			switchState(&(currentProc->p_s));
			}
			
		/*Sycall code is greater than 8*/
		default: {			
			/*Pass up or Die*/
			passUpOrDie(GENERALEXCEPT); }
	}
	

}

/*Creates a new process for use*/
void createProcess() {

	state_PTR exceptionState = (state_PTR)EXCEPTSTATEADDR;
	pcb_PTR newProc = allocPcb();
	if(newProc == NULL) {
		exceptionState->s_v0 = ERRORCODE;
		exceptionState->s_pc += WORDLEN;
		switchState(exceptionState);
	}
	state_PTR newState = (exceptionState->s_a1);
	stateCopy(newState, &(newProc->p_s));
	newProc->p_supportStruct = (support_t*)(exceptionState->s_a2);
	insertProcQ(&readyQ, newProc);
	insertChild(newProc,currentProc);
	exceptionState->s_pc += WORDLEN;
	processCount++;
	switchState(exceptionState);
}

/*Terminate the current process, and all of its children*/
void terminateProcess(pcb_PTR current) {
	/*Checks if the current node has a child*/
	while(!emptyChild(current)) 
	{
		/*Recurses, using the current node's child as a parameter input*/
		terminateProcess(current->p_child);
	}
	
	/*Checks if the inputted parameter is the current process*/
	if(&current == &currentProc)
	{	
		/*Disconnect current from its parent*/
		outChild(current);
	}
	
	/*Checks if current's semaphore address is NULL*/
	if(current->p_semAdd == NULL)
	{	
		/*Remove current from the ready queue*/
		outProcQ(&readyQ, current);
	}
	
	/*Checks if current PCB is waiting for I/O*/
	if(current->p_semAdd >= &deviceSema4s[0] && current->p_semAdd <= &clockSem)
	{
		/*Copies current's semaphore address to a seperate integer variable*/
		/*Remove semAdd (current?) from the process queue, returning its pointer*/
		/*NOTE TO SELF: double check*/
		pcb_PTR p = outBlocked(current);
		/*While p is not NULL*/
		while(p != NULL)
		{
			/*Decrement count of soft blocked items*/
			softBlockCount--;
			/*Remove semAdd (current?) from the process queue, returning its pointer*/
			p = outBlocked(current);
		}
	}
	/*Add current to the free PCB list*/
	/*NOTE TO SELF: double check*/
	freePcb(current);
	processCount--;
}

/*Performs a P operation on a semaphore*/
/*NOTE TO SELF: really need to figure out what this means*/
void passeren() {
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	
	int *sem = oldState->s_a1;
	(*sem)--;
	
	oldState->s_pc+= WORDLEN;
	stateCopy(oldState, &(currentProc->p_s));
	if((*sem) < 0) {
		insertBlocked(&(oldState->s_a1), currentProc);
		scheduler();
	}
	else {
		oldState->s_pc += WORDLEN;
		stateCopy(oldState, &(currentProc->p_s));
		switchState(&(currentProc->p_s));
	}
	
}

/*Performs a V operation on a semaphore*/
/*NOTE TO SELF: find out what this means*/
void verhogen() {
	
	
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	int *sem = oldState->s_a1;
	(*sem)++;
	oldState->s_a1 = *sem;
	pcb_PTR p;
	if(oldState->s_a1 >= 0) {
		p = removeBlocked(&(oldState->s_a1));
		if(p != NULL) 
			insertProcQ(&readyQ, p);
		
	}
	oldState->s_v0 = oldState->s_a1;
	oldState->s_pc+= WORDLEN;
	
	stateCopy(oldState, &(currentProc->p_s));
	switchState(&(currentProc->p_s));
}

/*Waits for input or output from a device*/
void waitForDevice() {
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
	softBlockCount++;
	if(clockSem < 0) {
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
		/*The current process' exception state is modified*/
		/*NOTE TO SELF: details needed*/
		state_PTR exceptStatePtr = (state_PTR) EXCEPTSTATEADDR;
		stateCopy(exceptStatePtr, &currentProc->p_supportStruct->sup_exceptState[0]);
		unsigned int newPc = currentProc->p_supportStruct->sup_exceptContext[0].c_pc;
		unsigned int newStackPtr = currentProc->p_supportStruct->sup_exceptContext[0].c_stackPtr;
		unsigned int newStatus = currentProc->p_supportStruct->sup_exceptContext[0].c_status;
		/*NOTE TO SELF: figure out wtf this does*/
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



