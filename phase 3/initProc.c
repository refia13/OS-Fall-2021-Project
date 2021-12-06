/*This is the start if the initProc module for phase 3. It implements test and exports the support level's global variables.*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "/usr/include/umps3/umps/libumps.h"
int procSem;
int devMutex[SEMCOUNT];
extern void tlbExceptionHandler();
extern void supGenExceptionHandler();
static support_t supportArray[UPROCMAX];
static state_t stateArray[UPROCMAX];
debugA(int a) {
	return a;
}
/*test function, initializes up to UPROCMAX (1-8) processes*/
void test() {
	/*Initialize the Phase 3 data structures*/
	/*Initialize and launch the processes*/
	
	int i;
	int j;
	int resultCode;
	procSem = 0;
	for(i = 0; i < SEMCOUNT; i++) {
		devMutex[i] = 1;
	}
	
	for(i = 0; i < UPROCMAX; i++) {
		/*Set up the Processor State*/
		stateArray[i].s_pc = TEXTADDR;
		stateArray[i].s_t9 = TEXTADDR;
		stateArray[i].s_sp = STACKPAGEADDR;
		stateArray[i].s_status = ALLOFF | KUPON | IEPON | IMON | TEON;
		stateArray[i].s_entryHI = (i+1) << 6;
		debugA(stateArray[i].s_entryHI);
		/*Set up the Support Structure*/
		supportArray[i].sup_asid = i+1;
		supportArray[i].sup_exceptContext[PGFAULTEXCEPT].c_pc = (memaddr) tlbExceptionHandler;
		supportArray[i].sup_exceptContext[PGFAULTEXCEPT].c_status = ALLOFF | IEPON | IMON | TEON;
		supportArray[i].sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = (int) &(supportArray[i].sup_stackTLB[499]);
		supportArray[i].sup_exceptContext[GENERALEXCEPT].c_pc = (memaddr) supGenExceptionHandler;
		supportArray[i].sup_exceptContext[GENERALEXCEPT].c_status = ALLOFF | IEPON | IMON | TEON;
		supportArray[i].sup_exceptContext[GENERALEXCEPT].c_stackPtr = (int) &(supportArray[i].sup_stackGen[499]);
		/*Set up the Page Table for the U Proc*/
		int j;
		for(j = 0; j < PGMAX - 1; j++) {
			supportArray[i].sup_privatePgTbl[j].entryHI = (((0x80000) + j) << 6) + (i+1) << 6;
			supportArray[i].sup_privatePgTbl[j].entryLO = ALLOFF | DIRTYON;
		}
		/*Set up the stack page*/
		supportArray[i].sup_privatePgTbl[PGMAX-1].entryHI = (((0xBFFFF) << 6) + (i+1)) << 6;
		supportArray[i].sup_privatePgTbl[PGMAX-1].entryLO = ALLOFF | DIRTYON;
		SYSCALL(CREATEPROCESS, &stateArray[i], &supportArray[i], 0);
		SYSCALL(PASSEREN, &procSem, 0, 0);
		
	}
	/*All processes have finished, terminate*/
	SYSCALL (TERMPROCESS, 0, 0, 0);
}


