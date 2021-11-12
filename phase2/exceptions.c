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

/*Sys 1, creates thread*/
void createProcess();
/*Sys 2, recursively terminates calling process and its progeny*/
void terminateProcess(pcb_PTR current);
/*Standard p operation*/
void passeren();
/*Standard v operation*/
void verhogen();
/*Sets calling process to wait for specified I/O*/
void waitForDevice();
/*Sets calling process to wait for pseudoclock*/
void waitForClock();
/*Performs pass up or die with PGFAULTEXCEPT*/
void tlbRefillHandler();
/*Performs pass up or die with GENERALEXCEPT*/
void programTrapHandler();

/*Single parameter method to handle syscalls*/
void syscallHandler(int syscallCode)
{
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	
	/*Checks KUP to see if process is in user mode*/
	if(oldState->s_status & KUPON)
	{
		/*Attempted a syscall in user mode, trigger a program trap*/
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
			passeren(); 
			break;}
			
		/*SYS4 Verhogen*/	
		case VERHOGEN: {
			/*Perform a V operation on a semaphore*/
			verhogen(); 
			break;}
			
		/*SYS5 Wait for IO*/
		case WAITFORIO: {
			/*Waits for input or output from a device*/
			waitForDevice(); 
			break;}
			
		/*SYS6 Get CPU Time*/
		case GETCPUT: {
			/*Stores accumulated CPU time in v0 and performs load state*/
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
			/*Stores pointer to support structure in v0 and performs load state*/
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
	/*Return to current process*/
	if(currentProc != NULL) {
		switchState(&(currentProc->p_s));
	}
	/*No process to return to, call scheduler*/
	else{
		scheduler();
	}
	

}

/*Creates a new process for use*/
void createProcess() {
	
	state_PTR exceptState = (state_PTR)EXCEPTSTATEADDR;
	
	/*Allocate new pcb*/
	pcb_PTR newProc = allocPcb();
	/*Automatically store ERRORCODE in result in case newProc is null*/
	int result = ERRORCODE;
	
	
	if(newProc != NULL) {
		/*Increment process count and initialize state of new process*/
		processCount++;
		state_PTR newState = (state_PTR)(exceptState->s_a1);
		stateCopy(newState, &(newProc->p_s));
		newProc->p_supportStruct = exceptState->s_a2;
		newProc->p_time = 0;
		newProc->p_semAdd = NULL;
		/*Makes new process child of current process*/
		insertChild(currentProc, newProc);
		/*Adds new process to the ready queue and stores SUCCESS into result*/
		insertProcQ(&readyQ, newProc);
		result = SUCCESS;
	}
	/*Stores result in v0 and increments pc to prevent syscall loop*/
	exceptState->s_v0 = result;
	exceptState->s_pc += WORDLEN;
	/*No process to return to, call scheduler*/
	if(currentProc == mkEmptyProcQ()) {
		scheduler();
	}
	
	/*Copy state into current process, load state*/
	stateCopy(exceptState, &(currentProc->p_s));
	switchState(&(currentProc->p_s));
}

/*Terminate the current process, and all of its children*/
void terminateProcess(pcb_PTR current) {
	/*Checks if the current node has a child*/
	while(emptyChild(current) == FALSE) 
	{
		/*Recursively calls terminate process on each child until current has no children*/
		terminateProcess(removeChild(current));
		
	}
	/*If current is on the ASL*/
	if((current->p_semAdd) != NULL) {
		/*Current is waiting on I/O*/
		if((current->p_semAdd >= &deviceSema4s[0]) && (current->p_semAdd <= &clockSem))
		{
			softBlockCount--;
		}
		/*Current is blocked*/
		else {
			int *sem = current->p_semAdd;
			(*sem) += 1;
		}
		/*Removes current from waiting list*/
		/*Decrement process count if p is not null, must be done per case to prevent process count from being decremented too much*/
		pcb_PTR p = outBlocked(current);
		if(p != mkEmptyProcQ()) {
			processCount--;
		}
		
		
	}
	/*Current is on the ready queue*/
	else {
		/*Remove from queue*/
		pcb_PTR p = outProcQ(&readyQ, current);
		/*Decrement process count if p is not null, must be done per case to prevent process count from being decremented too much*/
		if(p != mkEmptyProcQ()) {
			processCount--;
		}
		
		
	}
	
	
	
	
	/*Current is the current process*/
	if(current == currentProc) {
		/*Orphan current, decrement process count, and call scheduler*/
		if(!emptyChild(current)) {outChild(currentProc);}
		currentProc = mkEmptyProcQ();
		processCount--;
		scheduler();
	}
	
	/*Deallocate current*/
	freePcb(current);
}

/*Performs a P operation on a semaphore*/
void passeren() {
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	
	/*Dereference and decrement semaphore*/
	int *sem = oldState->s_a1;
	(*sem)--;
	
	/*Increment pc in case process doesn't block*/
	oldState->s_pc+= WORDLEN;
	stateCopy(oldState, &(currentProc->p_s));
	if( (*sem) < 0) {
		/*Process blocks, insert onto asl*/
		insertBlocked(oldState->s_a1, currentProc);
		
		currentProc = mkEmptyProcQ();
		scheduler();
	}
	/*load state*/
	switchState(&(currentProc->p_s));
	
}

/*Performs a V operation on a semaphore*/
void verhogen() {
		
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	
	/*Dereference and increment semaphore*/
	int *sem = oldState->s_a1;
	(*sem)++;
	
	pcb_PTR p;
	if((*sem) <= 0) {
		/*Signal a waiting process based on semaphore*/
		p = removeBlocked(sem);
		if(p != mkEmptyProcQ()) {
			insertProcQ(&readyQ, p);
		}
	}
	/*Store semaphore in v0 and increment pc*/
	oldState->s_v0 = sem;
	oldState->s_pc+= WORDLEN;
	
	/*Copy state then load state*/
	stateCopy(oldState, &(currentProc->p_s));
	switchState(&(currentProc->p_s));
}

/*Waits for input or output from a device*/
void waitForDevice() {
	
	/*Stop the current interval by storing the current TOD into stopTod*/
	int stopTod;
	STCK(stopTod);
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	int lineNo = oldState->s_a1;
	int devNo = oldState->s_a2;
	int waitForTermRead = oldState->s_a3;
	/*Compute the device semaphore index based on the given line number, device number, and whether read/write was specified*/
	int devSemIndex = (lineNo-3)*8 + devNo;
	if(waitForTermRead == TRUE) {
		devSemIndex += 8;
	}
	/*Decrement device semaphore*/
	deviceSema4s[devSemIndex]--;
	if(deviceSema4s[devSemIndex] < 0) {
		/*Since the device semaphore is a synchronization semaphore, this will always block*/
		oldState->s_pc += WORDLEN;
		stateCopy(oldState, &(currentProc->p_s));
		currentProc->p_time += (stopTod - startTod);
		insertBlocked(&(deviceSema4s[devSemIndex]), currentProc);
		/*Sets currentProc to null after inserting it onto the waiting list and calls the scheduler, incrementing soft block count*/
		currentProc = mkEmptyProcQ();
		softBlockCount++;
		scheduler();
	}
}

/*Performs a P operation on the pseudo-clock semaphore and blocks the current process*/
void waitForClock() {
	/*Store accumulated time in calling process*/
	int stopTod;
	STCK(stopTod);
	currentProc->p_time += (stopTod - startTod);
	state_PTR oldState = (state_PTR)EXCEPTSTATEADDR;
	/*Decrement clockSem*/
	clockSem--;
	
	if(clockSem < 0) {
		/*Insert process onto asl, waiting on pseudoclock*/
		softBlockCount++;
		oldState->s_pc += WORDLEN;
		stateCopy(oldState, &(currentProc->p_s));
		insertBlocked(&clockSem, currentProc);
		currentProc = NULL;
	}
	/*Start new interval and call scheduler*/
	STCK(startTod);
	scheduler();
}


void passUpOrDie(unsigned int passUpCase)
{
	/*Support_t is not NULL*/
	if((currentProc->p_supportStruct) != NULL) {
		/*Stores context and passes up to next level*/
		state_PTR exceptStatePtr = (state_PTR) EXCEPTSTATEADDR;
		stateCopy(exceptStatePtr, &(currentProc->p_supportStruct->sup_exceptState[passUpCase]));
		int newPc = currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_pc;
		int newStackPtr = currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_stackPtr;
		int newStatus = currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_status;
		LDCXT(currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_stackPtr, currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_status, currentProc->p_supportStruct->sup_exceptContext[passUpCase].c_pc);
	}
	else {/*Die*/
		/*Terminate the current process then call scheduler*/
		terminateProcess(currentProc); 
		scheduler();}
}
/*Nucleus level programTrapHandler, performs a passUpOrDie operation with the value GENERALEXCEPT*/
void programTrapHandler() {
	
	passUpOrDie(GENERALEXCEPT);
}
/*Nucleus level tlbException Handler, performs a passUpOrDie operation with the value PGFAULTEXCEPT*/
void tlbExceptionHandler() {
	passUpOrDie(PGFAULTEXCEPT);
}



