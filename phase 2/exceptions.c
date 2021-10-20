/*This will be the exceptions handling file. It tells the OS what to do when syscalls and exceptions occur*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "../h/interrupts.h"

public void syscallHandler(int syscallCode)
{
	if(currentProc->p_s.s_status & KUPON)
	{
		/*Attempted a syscall in user mode, triggger a program trap*/
		programTrap();
	}
	switch(syscallCode) 
	{
		case CREATEPROCESS: {
			/*SYS1 Create Process*/
			createProcess(); }
		case TERMPROCESS: {
			/*SYS2 Terminate Process*/
			/*Recursively terminates currentProc and its children*/
			outChild(currentProc);
			terminateProcess(currentProc);}
		case PASSEREN: {
			/*SYS3 Passaren*/
			passeren(); }
		case VERHOGEN: {
			/*SYS4 Verhogen*/
			verhogen(); }
		case WAITFORIO: {
			/*SYS5 Wait for IO*/
			waitForDevice(); }
		case GETCPUT:
			/*SYS6 Get CPU Time*/
			cpu_t time = currentProc -> p_time;
			procState -> s_v0 = time;
			newState(&procState);
		case WAITFORCLOCK: {
			/*SYS7 Wait for Clock*/
			waitForClock(); }
		case GETSUPPORTT: {
			/*SYS8 Get Support Data*/
			support_t tempSupport = currentProc -> p_supportStruct;
			procState -> s_v0 = tempSupport;
			newState(&procState);
			}
		default:
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
}

public void terminateProcess(pcb_PTR current) {
	/*Terminate the current process, and all of its children*/
	if(!emptyChild(current))
	{
		terminateProcess(current->p_child);
	}
	freePcb(current);
	processCount--;
}

public void passeren() {
	currentProc -> p_s.a1--;
	if(currentProc->p_s.a1 < 0)
	{
		insertBlocked(&(currentProc->p_s.a1), p);
		softBlockCount++;
	}
	scheduler();
}

public void verhogen() {
	currentProc->p_s.a1++;
	if(currentProc->p_s.a1 >= 0)
	{
		p = removeBlocked(&(currentProc->p_s.a1));
		if(p != NULL)
		{
			insertProcQ(&readyQ, p);
			softBlockCount--;
		}
		
	}
}

public void waitForDevice() {
	
	int lineNo = currentProc->p_s.a1;
	int devNo = currentProc->p_s.a2;
	int devIndex = (lineNo)*devNo+1;
	int devSem = deviceSema4s[devIndex];
	currentProc->p_s.a1 = devSem;
	passeren();
}

public void waitForClock() {
	currentProc->p_s.a1 = clockSem;
	passeren();
}

void uTLB_RefillHandler() {
	setENTRYHI (0x80000000);
	setENTRYLO (0x00000000);
	TLBWR();
	LDST((state_PTR) 0x0FFFF000);
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

