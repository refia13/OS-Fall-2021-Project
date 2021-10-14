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
		case 1:
			/*SYS1 Create Process*/
			pcb_PTR newProc = allocPcb();
			newProc -> p_s = procState -> s_a1;
			newProc -> p_supportStruct = procState -> s_a2;
			insertProcQ(&readyQ,newProc);
			insertChild(currentProc,newProc);
			processCount++;
			newProc -> p_time = 0;
			newProc -> p_semAdd = NULL;
		case 2:
			/*SYS2 Terminate Process*/
			/*Recursively terminates currentProc and its children*/
			freePcb(currentProc);
			if(!emptyChild(currentProc))
			{
				SYS2;
				processCount--;
			}
		case 3:
			/*SYS3 Passaren*/
			sema4--;
			if(sema4 < 0)
			{
				insertBlocked(&sema4, p);
			}
			scheduler();
		case 4:
			/*SYS4 Verhogen*/
			sema4++;
			if(sema4 >= 0)
			{
				removeBlocked(sema4);
				insertProcQ(readyQ, p);
			}
		case 5:
			/*SYS5 Wait for IO*/
			
		case 6:
			/*SYS6 Get CPU Time*/
			cpu_t time = currentProc -> p_time;
			procState -> s_v0 = time;
			LDST(procState);
		case 7:
			/*SYS7 Wait for Clock*/
			
		case 8:
			/*SYS8 Get Support Data*/
			support_t tempSupport = currentProc -> p_supportStruct;
			procState -> s_v0 = tempSupport;
			LDST(procState);
		default:
			/*Pass up or Die*/
	}
}
