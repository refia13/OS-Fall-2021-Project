/*this will be the scheduler, which handles the scheduling and execution of processes, it also contains the loadState and CopyState functions*/
/*other files referenced within the code of the scheduler*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/scheduler.h"
#include "/usr/include/umps3/umps/libumps.h"

/*External function provided by umps*/
extern unsigned int setTIMER(unsigned int t);
/*Debug function, calls load state on given state pointer*/
extern void switchState(state_PTR s);
/*Copies source state pointer into sink's state pointer*/
void stateCopy(state_PTR source, state_PTR sink);


/*Program for the scheduler, assigns time to processes and begins their execution*/
void scheduler() {
	/*Removes a process from the ready queue to become the current process*/
	if(!emptyProcQ(readyQ)) {
		/*Initializes the state of the current process*/
		currentProc = removeProcQ(&readyQ);
		setTIMER(TIMESLICE);
		STCK(startTod);
		switchState(&currentProc->p_s);
	}
	else {
	/*If there are no more processes*/
		if(processCount == 0){
			/*Stop the program*/
			HALT();
		}
		/*If there are any blocked processes*/
		else if(softBlockCount > 0){
			/*sets the status to have interrupts on, and turns on the interrupt mask. Ensures that the next interrupt will be a device by keeping TEBit off*/
			setSTATUS(ALLOFF | IECON | IMON);
			/*Twiddle Thumbs until device interrupt*/
			WAIT();
		}
		/*Deadlocked - Process count > 0 and blocked count = 0*/
		else if(processCount > 0 && softBlockCount <= 0){
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
void stateCopy(state_PTR source, state_PTR sink) {
		
		sink->s_cause = source->s_cause;
		sink->s_entryHI = source->s_entryHI;
		sink->s_pc = source->s_pc;
		sink->s_status = source->s_status;
		int i;
		for(i = 0; i < STATEREGNUM; i++) {
	
			sink->s_reg[i] = source->s_reg[i];
			
		}
	
	
}
