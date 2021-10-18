/*this will be the scheduler, which handles the scheduling and execution of processes*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/scheduler.h"
#include "/usr/include/umps3/umps/libumps.h"

public void scheduler() {
	pcb_PTR currentProc = removeProcQ(&readyQ);
	/*Load 5 milliseconds onto the PLT*/
	newTimer = getTIMER() + 5;
	setTIMER(newTimer);
	newState(currentProc->p_s);
	if(processCount == 0)
	{
		HALT();
	}
	else if(blockedCount > 0)
	{
		/*Twiddle Thumbs until device interrupt*/
		/*currentProc->p_s->s_status bit 0 is set to 1, then set bits 8-15 to 1*/
		currentProc -> p_s = ALLOFF | ION | IMON;
		WAIT();
	}
	else
	{
		PANIC();
	}
}

public int newState(state_PTR s) {
	LDST(*s);
}
