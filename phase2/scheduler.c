/*this will be the scheduler, which handles the scheduling and execution of processes*/
/*other files referenced within the code of the scheduler*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/scheduler.h"
#include "/usr/include/umps3/umps/libumps.h"

extern void setTimer();
extern unsigned int setTIMER(unsigned int t);
extern void switchState(state_PTR s);

/*Program for the scheduler, assigns time to processes and begins their execution*/
void scheduler() {
	/*Removes a process from the ready queue to become the current process*/
	pcb_PTR currentProc = removeProcQ(&readyQ);
	/*Adds 5 milliseconds to the timer*/
	int newTimer = getTIMER() + TIMESLICE;
	/*Loads 5 milisecons onto the PLT*/
	setTIMER(newTimer);
	/*Initializes the state of the current process*/
	switchState(&currentProc->p_s);
	/*If there are no more processes*/
	if(processCount == 0)
	{
		/*Stop the program*/
		HALT();
	}
	/*If there are any blocked processes*/
	else if(softBlockCount > 0)
	{
		/*currentProc->p_s->s_status bit 0 is set to 1, then set bits 8-15 to 1*/
		state_PTR state = (state_PTR) ((ALLOFF) | (IEPON | IMON));
		currentProc->p_s = *state;
		/*Twiddle Thumbs until device interrupt*/
		WAIT();
	}
	/*Deadlocked - Process count > 0 and blocked count = 0*/
	else
	{
		/*Panic to deal with deadlock*/
		PANIC();
	}
}

/*Loads processor state based on the given state pointer*/
void switchState(state_PTR s) {
	LDST(&s);
}
