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
	/*NOTE TO WILL: Are breaks needed for cleanliness/to prevent redundant case checking? Also might move code descriptions for clarity*/
	switch(syscallCode) 
	{
		case CREATEPROCESS: {
			/*SYS1 Create Process*/
			/*Creates a new process for use*/
			createProcess(); }
		case TERMPROCESS: {
			/*SYS2 Terminate Process*/
			terminateProcess(currentProc);}
		case PASSEREN: {
			/*SYS3 Passaren*/
			/*NOTE TO SELF: check what this function does, write comment*/
			passeren(); }
		case VERHOGEN: {
			/*SYS4 Verhogen*/
			/*NOTE TO SELF: check what this function does, write comment*/
			verhogen(); }
		case WAITFORIO: {
			/*SYS5 Wait for IO*/
			/*Waits for input or output from a device*/
			/*NOTE TO SELF: check what this function does, write better comment*/
			waitForDevice(); }
		case GETCPUT:
			/*SYS6 Get CPU Time*/
			/*Assigns [s_v0] of currentProc to its own time value*/
			currentProc->p_s.s_v0 = currentProc->p_time;
			/*Adds the value of [WORDLEN] to currentProc's [s_pc]*/
			/*NOTE TO SELF: variables in brackets should be checked to make better comments*/
			currentProc->p_s.s_pc + WORDLEN;
			/*Creates a new state based on the state of the current Proc*/
			newState(&procState);
		case WAITFORCLOCK: {
			/*SYS7 Wait for Clock*/
			waitForClock(); }
		case GETSUPPORTT: {
			/*SYS8 Get Support Data*/
			currentProc->p_s.s_v0 = currentProc->p_support;
			currentProc->p_s.s_pc + WORDLEN;
			newState(&procState);
			}
		default:
			/*Emergency case*/
			/*Pass up or Die*/
			passUpOrDie();
	}
	

}


public void createProcess() {
	pcb_PTR newProc = allocPcb();
	newProc -> p_s = procState -> s_a1;
	newProc -> p_supportStruct = procState -> s_a2;
	insertProcQ(&readyQ,newProc);
	insertChild(currentProc,newProc);
	processCount++;
	newProc -> p_time = 0;
	newProc -> p_semAdd = NULL;
	currentProc->p_s.s_pc + WORDLEN;
}

public void terminateProcess(pcb_PTR current) {
	/*Terminate the current process, and all of its children*/
	/*Checks if the current node has a child*/
	while(!emptyChild(current)) 
	{
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

