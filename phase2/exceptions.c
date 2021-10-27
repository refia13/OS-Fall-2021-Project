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

/*Single parameter method to handle syscalls*/
void syscallHandler(int syscallCode)
{
	/*Checks current state of program, to see if currently in user mode*/
	if(currentProc->p_s.s_status & KUPON)
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
			/*Accumulated processor time is plced in the current process' v0 register*/
			currentProc->p_s.s_v0 = currentProc->p_time;
			/*Increment pc to avoid Syscall loop*/
			/*NOTE TO WILL: thingy to change*/
			currentProc->p_s.s_pc += PCINCREMENT;
			/*Creates a new state based on the state of the current process*/
			newState(&currentProc->p_s); }
			
		/*SYS7 Wait for Clock*/
		case WAITFORCLOCK: {
			/*Performs a P operation on the pseudo-clock semaphore and blocks the current process*/
			waitForClock(); }
			
		/*SYS8 Get Support Data*/
		case GETSUPPORTT: {
			/*Stores pointer for current process' support structure in register v0*/
			currentProc->p_s.s_v0 = currentProc->p_supportStruct;
			/*Increment pc to avoid Syscall loop*/
			currentProc->p_s.s_pc += PCINCREMENT;
			/*Creates new state for current process*/
			newState(&currentProc->p_s); }
			
		/*Sycall code is greater than 8*/
		default: {			
			/*Pass up or Die*/
			passUpOrDie(GENERALEXCEPT); }
	}
	

}

/*Creates a new process for use*/
void createProcess() {
	/*Creates new process by allocating PCB*/
	pcb_PTR newProc = allocPcb();
	/*Initializes new process' state using value pointed to by a1*/
	state_PTR newState = (state_PTR) currentProc->p_s.s_a1;
	newProc -> p_s = *newState;
	/*Initializes the support structure of the new process using value pointed to by a2*/
	newProc -> p_supportStruct = currentProc -> p_s.s_a2;
	/*Inserts new process into the ready queue*/
	insertProcQ(&readyQ,newProc);
	/*Assigns new process as the child of the current process*/
	insertChild(currentProc,newProc);
	/*Process count incremented*/
	processCount++;
	/*CPU time initialized to zero*/
	newProc -> p_time = 0;
	/*Initializes the new process in the "ready" state, rather than a blocked state*/
	newProc -> p_semAdd = NULL;
	/*Increment pc to avoid Syscall loop*/
	currentProc->p_s.s_pc += PCINCREMENT;
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
	if(current == currentProc)
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
}

/*Performs a P operation on a semaphore*/
/*NOTE TO SELF: really need to figure out what this means*/
void passeren() {
	/*Current process' a1 value is decremented by one*/
	currentProc -> p_s.s_a1--;
	/*Checks if current process' semaphore value is below 0*/
	if(currentProc->p_s.s_a1 < 0)
	{
		/*Add current process to ASL*/
		/*NOTE TO SELF: Double check*/
		insertBlocked(&(currentProc->p_s.s_a1), currentProc);
		/*Call the scheduler*/
		currentProc = NULL;
		scheduler();
	}
	else {
		/*Increment pc to avoid Syscall loop*/
		/*NOTE TO WILL: thingy to change*/
		currentProc->p_s.s_pc += PCINCREMENT;
		/*Initializes new state based on current process' status*/
		newState(&(currentProc->p_s));
	}
	
}

/*Performs a V operation on a semaphore*/
/*NOTE TO SELF: find out what this means*/
void verhogen() {
	/*Increment current process' a1 value*/
	currentProc->p_s.s_a1++;
	/*Check if the current process' a1 value is at or above 0*/
	if(currentProc->p_s.s_a1 >= 0)
	{
		/*Remove current process from ASL, storing pointer in p*/
		/*NOTE TO SELF: double check*/
		pcb_PTR p = removeBlocked(&(currentProc->p_s.s_a1));
		/*Checks if p has a value*/
		if(p != NULL)
		{
			/*Adds current process to the ready queue*/
			insertProcQ(&readyQ, p);
		}
		
	}
}

/*Waits for input or output from a device*/
void waitForDevice() {
	/*Sets line number to the current process' a1 value*/
	int lineNo = currentProc->p_s.s_a1;
	/*Sets device number to the current process' a2 value*/
	int devNo = currentProc->p_s.s_a2;
	/*Sets device index as the line number multiplied by device number incremented by one*/
	int devIndex = (lineNo-3)*8 + devNo;
	/*Set current process' a1 value to the device semaphore*/
	currentProc->p_s.s_a1 = deviceSema4s[devIndex];
	/*Switch the state of the current process*/
	switchState(&currentProc->p_s);
	/*Perform a P operation on current process*/
	passeren();
}

/*Performs a P operation on the pseudo-clock semaphore and blocks the current process*/
void waitForClock() {
	/*Set current process' a1 register to the pseudo-clock semaphore*/
	currentProc->p_s.s_a1 = clockSem;
	/*Block the current process*/
	switchState(&currentProc->p_s);
	/*Perform a P operation on the current process*/
	passeren();
}


void passUpOrDie(unsigned int passUpCase)
{
	/*Support_t is not NULL*/
	if(currentProc->p_supportStruct != NULL) {
		/*Pass Up*/
		/*The current process' exception state is modified*/
		/*NOTE TO SELF: details needed*/
		state_PTR exceptStatePtr = (state_PTR) EXCEPTSTATEADDR;
		currentProc->p_supportStruct->sup_exceptState[0] = *exceptStatePtr;
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

	passUpOrDie(GENERALEXCEPT);
}
/*Nucleus level programTrapHandler, performs a passUpOrDie operation with the value PGFAULTEXCEPT*/
void tlbExceptionHandler() {
	passUpOrDie(PGFAULTEXCEPT);
}

