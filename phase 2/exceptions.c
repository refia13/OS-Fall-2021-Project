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

/*Single parameter method to handle syscalls*/
public void syscallHandler(int syscallCode)
{
	/*Checks current state of program, to see if currently in user mode*/
	if(currentProc->p_s.s_status & KUPON)
	{
		/*Attempted a syscall in user mode, triggger a program trap*/
		programTrap();
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
			/*Assigns [s_v0] of currentProc to its own time value*/
			currentProc->p_s.s_v0 = currentProc->p_time;
			/*Increment pc to avoid Syscall loop*/
			/*NOTE TO WILL: thingy to change*/
			currentProc->p_s.s_pc + WORDLEN;
			/*Creates a new state based on the state of the current Proc*/
			newState(&procState); }
			
		/*SYS7 Wait for Clock*/
		case WAITFORCLOCK: {
			waitForClock(); }
			
		/*SYS8 Get Support Data*/
		case GETSUPPORTT: {
			currentProc->p_s.s_v0 = currentProc->p_support;
			currentProc->p_s.s_pc + WORDLEN;
			newState(&procState); }
			
		/*Sycall code is greater than 8*/
		default: {			
			/*Pass up or Die*/
			passUpOrDie(); }
	}
	

}

/*Creates a new process for use*/
public void createProcess() {
	/*Creates new process by allocating PCB*/
	pcb_PTR newProc = allocPcb();
	/*Initializes new process' state using value pointed to by a1*/
	newProc -> p_s = procState -> s_a1;
	/*Initializes the support structure of the new process using value pointed to by a2*/
	newProc -> p_supportStruct = procState -> s_a2;
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
	/*NOTE TO WILL: a thingy to change*/
	currentProc->p_s.s_pc + WORDLEN;
}

/*Terminate the current process, and all of its children*/
public void terminateProcess(pcb_PTR current) {
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
	if(current->p_semAdd >= &devSem[0] && current->p_semAdd <= &clockSem)
	{
		/*Copies current's semaphore address to a seperate integer variable*/
		/*NOTE TO SELF: figure out why*/
		int semAdd = current->p_semAdd;
		/*Remove semAdd (current?) from the process queue, returning its pointer*/
		/*NOTE TO SELF: double check*/
		pcb_PTR p = outBlocked(semAdd);
		/*While p is not NULL*/
		while(p != NULL)
		{
			/*Decrement count of soft blocked items*/
			softBlockCount--;
			/*Remove semAdd (current?) from the process queue, returning its pointer*/
			p = outBlocked(semAdd);
		}
	}
	/*Add current to the free PCB list*/
	/*NOTE TO SELF: double check*/
	freePcb(current); 
}

/*Performs a P operation on a semaphore*/
/*NOTE TO SELF: really need to figure out what this means*/
public void passeren() {
	/*Current process' a1 value is decremented by one*/
	currentProc -> p_s.a1--;
	/*Checks if current process' a1 value is below 0*/
	if(currentProc->p_s.a1 < 0)
	{
		/*Add current process to ASL*/
		/*NOTE TO SELF: Double check*/
		insertBlocked(&(currentProc->p_s.a1), p);
		/*Call the scheduler*/
		scheduler();
	}
	else {
		/*Increment pc to avoid Syscall loop*/
		/*NOTE TO WILL: thingy to change*/
		currentProc->p_s.s_pc + WORDLEN;
		/*Initializes new state based on current process' status*/
		newState(&(currentProc->p_s));
	}
	
}

/*Performs a V operation on a semaphore*/
/*NOTE TO SELF: find out what this means*/
public void verhogen() {
	/*Increment current process' a1 value*/
	currentProc->p_s.a1++;
	/*Check if the current process' a1 value is at or above 0*/
	if(currentProc->p_s.a1 >= 0)
	{
		/*Remove current process from ASL, storing pointer in p*/
		/*NOTE TO SELF: double check*/
		p = removeBlocked(&(currentProc->p_s.a1));
		/*Checks if p has a value*/
		if(p != NULL)
		{
			/*Adds current process to the ready queue*/
			insertProcQ(&readyQ, p);
		}
		
	}
}

/*Waits for input or output from a device*/
public void waitForDevice() {
	/*Sets line number to the current process' a1 value*/
	int lineNo = currentProc->p_s.a1;
	/*Sets device number to the current process' a2 value*/
	int devNo = currentProc->p_s.a2;
	/*Sets device index as the line number multiplied by device number incremented by one*/
	int devIndex = (lineNo)*devNo+1;
	/*Sets device semaphore as [...]*/
	/*NOTE TO SELF: figure out what this does*/
	int devSem = deviceSema4s[devIndex];
	/*Set current process' a1 value to the device semaphore*/
	currentProc->p_s.a1 = devSem;
	/*Switch the state of the current process*/
	switchState(currentProc->p_s);
	/*Perform a P operation on current process*/
	passeren();
}

public void waitForClock() {
	currentProc->p_s.a1 = clockSem;
	switchState(currentProc->p_s);
	passeren();
}

void uTLB_RefillHandler() {
	setENTRYHI (0x80000000);
	setENTRYLO (0x00000000);
	TLBWR();
	newState((state_PTR) 0x0FFFF000);
}

void passUpOrDie()
{
	/*Support_t is not NULL*/
	if(currentProc->support_t != NULL) {
		/*Pass Up*/
		currentProc->p_support->sup_exceptState[0] = (state_PTR) EXCEPTSTATEADDR;
		LDCXT(currentProc->p_support->sup_exceptContext);
	}
	else {/*Die*/
		outChild(currentProc); 
		terminateProcess(currentProc); }
}

