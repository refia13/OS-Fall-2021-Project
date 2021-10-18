/*This will be the exceptions handling file. It tells the OS what to do when syscalls and exceptions occur*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "../h/interrupts.h"

public void syscallHandler(int syscallCode, state_t *procState, support_t supportp)
{
	switch(syscallCode) 
	{
		case 1: {
			/*SYS1 Create Process*/
			createProcess(); }
		case 2: {
			/*SYS2 Terminate Process*/
			/*Recursively terminates currentProc and its children*/
			terminateProcess();}
		case 3: {
			/*SYS3 Passaren*/
			passaren(); }
		case 4: {
			/*SYS4 Verhogen*/
			verhogen(); }
		case 5: {
			/*SYS5 Wait for IO*/
			deviceInterrupt(); }
		case 6:
			/*SYS6 Get CPU Time*/
			cpu_t time = currentProc -> p_time;
			procState -> s_v0 = time;
			newState(&procState);
		case 7: {
			/*SYS7 Wait for Clock*/
			waitForClock(); }
		case 8: {
			/*SYS8 Get Support Data*/
			support_t tempSupport = currentProc -> p_supportStruct;
			procState -> s_v0 = tempSupport;
			newState(&procState);
			}
		default:
			/*Pass up or Die*/
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

public void terminateProcess() {
	freePcb(currentProc);
	if(!emptyChild(currentProc))
	{
		terminateProcess();
		processCount--;
	}
}

public void pasaren() {
	currentProc -> p_s.a1--;
	if(currentProc->p_s.a1 < 0)
	{
		insertBlocked(&(currentProc->p_s.a1, p);
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
		}
		
	}
}

public void deviceInterupt() {

}

public void waitForClock() {

}
