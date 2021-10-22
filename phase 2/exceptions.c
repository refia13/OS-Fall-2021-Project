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
			terminateProcess(currentProc); }
			
		/*SYS3 Passaren*/
		case PASSEREN: {
			/*NOTE TO SELF: check what this function does, write comment*/
			passeren(); }
			
		/*SYS4 Verhogen*/	
		case VERHOGEN: {
			/*NOTE TO SELF: check what this function does, write comment*/
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
			/*Adds the value of [WORDLEN] to currentProc's [s_pc]*/
			/*NOTE TO SELF: variables in brackets should be checked to make better comments*/
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
	/*Increment pc to avoid syscall loop*/
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
	if(current == currentProc)
	{	
		outChild(current);
	}
	if(current->p_semAdd == NULL)
	{
		outProcQ(&readyQ, current);
	}
	if(current->p_semAdd >= &devSem[0] && current->p_semAdd <= &clockSem)
	{
		int semAdd = current->p_semAdd;
		pcb_PTR p = outBlocked(semAdd);
		while(p != NULL)
		{
			softBlockCount--;
			p = outBlocked(semAdd);
		}
	}
	freePcb(current); 
}

public void passeren() {
	currentProc -> p_s.a1--;
	if(currentProc->p_s.a1 < 0)
	{
		insertBlocked(&(currentProc->p_s.a1), p);
		scheduler();
	}
	else {
		currentProc->p_s.s_pc + WORDLEN;
		newState(&(currentProc->p_s));
	}
	
}

public void verhogen() {
	currentProc->p_s.a1++;
	if(currentProc->p_s.a1 >= 0)
	{
		p = removeBlocked(&(currentProc->p_s.a1));
		if(p != NULL)
		{
			insertProcQ(&readyQ, p);
		}
		
	}
}

public void waitForDevice() {
	
	int lineNo = currentProc->p_s.a1;
	int devNo = currentProc->p_s.a2;
	int devIndex = (lineNo)*devNo+1;
	int devSem = deviceSema4s[devIndex];
	currentProc->p_s.a1 = devSem;
	switchState(currentProc->p_s);
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

