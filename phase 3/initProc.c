/*This is the start if the initProc module for phase 3. It implements test and exports the support level's global variables.*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"

int devMutex[SEMCOUNT];
/*test function, instantiates the processes for phase 3*/
void test() {
	/*Initialize the Phase 3 data structures*/
	/*Initialize and launch the processes*/
	
	int i;
	int j;
	int resultCode;
	for(i = 0; i < SEMCOUNT; i++) {
		devMutex[i] = 1;
	}
	for(i = 0; i < UPROCMAX; i++) {
		/*Set up the UPROC state*/
		state_t uState;
		uState.s_pc = TEXTADDR;
		uState.s_sp = STACKPAGEADDR;
		uState.s_status = ALLOFF | KUPON | IEPON | IMON | TEON;
		uState.s_entryHI = ALLOFF | i;
		/*Set up the UPROC Support Structure*/
		support_t *uSup;
		uSup->sup_asid = i;
		for(j = 0; j < PGMAX - 1; j++) {
			uSup->sup_privatePgTbl[j].entryHI = (STARTPGNO + j) | i << ENTRYHISHIFT;
			uSup->sup_privatePgTbl[j].entryLO = ALLOFF | DIRTYON;
		}
		uSup->sup_privatePgTbl[31].entryHI = 0xBFFFF + i;
		uSup->sup_privatePgTbl[31].entryLO = ALLOFF | DIRTYON;
		uSup->sup_exceptContext[1].c_stackPtr = (int) &(uSup->sup_stackGen[500]);
		uSup->sup_exceptContext[0].c_stackPtr = (int) &(uSup->sup_stackTLB[500]);
		uSup->sup_exceptContext[0].c_pc = (memaddr) tlbExceptionHandler;
		uSup->sup_exceptContext[1].c_pc = (memaddr) supGenExceptionHandler;
		uSup->sup_exceptContext[0].c_status = ALLOFF | IEPON | IMON | TEON;
		uSup->sup_exceptContext[1].c_status = ALLOFF | IEPON | IMON | TEON;
		resultCode = SYSCALL (CREATEPROCESS, &(uState), uSup, 0);
		if(resultCode != ERRORCODE) {
			SYSCALL(PASSEREN, &procSem,0,0);
		}	
		
	}
	SYSCALL (TERMPROCESS, 0, 0, 0);
}


