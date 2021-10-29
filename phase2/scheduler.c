/*this will be the scheduler, which handles the scheduling and execution of processes, it also contains the loadState and CopyState functions*/
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
void stateCopy(state_t oldState, pcb_PTR current, int passup);


void debugB(int a, int b, int c) {
	int i = 0;
	i++;
}

/*Program for the scheduler, assigns time to processes and begins their execution*/
void scheduler() {
	/*Removes a process from the ready queue to become the current process*/
	
	pcb_PTR currentProc = removeProcQ(&readyQ);
	/*Adds 5 milliseconds to the timer */
	/*Loads 5 milisecons onto the PLT*/
	setTIMER(TIMESLICE);
	if(currentProc != NULL)
	{	
		LDST(&currentProc->p_s);
		
	}
	/*Initializes the state of the current process*/
	else {
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
		stateCopy(*state, currentProc, 0);
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
}

/*Loads processor state based on the given state pointer*/
void switchState(state_PTR s) {
	LDST(s);
}

/*Performs a deep copy of the given state into the given pcb's state field, the passup parameter is used to determine whether to copy
into the pcb's p_s field or into its supportStruct*/
void stateCopy(state_t oldState, pcb_PTR current, int passup) {
	if(passup == 0) /*case when normally copying a state*/
	{
		
		current->p_s.s_cause = oldState.s_cause;
		debugB(2,3,4);
		current->p_s.s_entryHI = oldState.s_entryHI;
		
		current->p_s.s_pc = oldState.s_pc;
		current->p_s.s_status = oldState.s_status;
		int i;
		for(i = 0; i < STATEREGNUM; i++) {
	
			current->p_s.s_reg[i] = oldState.s_reg[i];
			
		}
	}
	else /*case when the state needs to be copied into the pcb's supportStruct*/
	{
		current->p_supportStruct->sup_exceptState[0].s_cause = oldState.s_cause;
		current->p_supportStruct->sup_exceptState[0].s_entryHI = oldState.s_entryHI;
		current->p_supportStruct->sup_exceptState[0].s_pc = oldState.s_pc;
		current->p_supportStruct->sup_exceptState[0].s_cause = oldState.s_status;
		int i;
		for(i = 0; i < STATEREGNUM; i++) {
	
			current->p_supportStruct->sup_exceptState[0].s_reg[i] = oldState.s_reg[i];
			
		}
	}
	
}
