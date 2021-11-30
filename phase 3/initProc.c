/*This is the start if the initProc module for phase 3. It implements test and exports the support level's global variables.*/

#include "../h/initProc.h"

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
			uSup->privatePgTbl[j].entryHI = (STARTPGNO + j) | i << ENTRYHISHIFT;
			uSup->privatePgTbl[j].entryLO = ALLOFF | DIRTYON;
		}
		uSup->privatePgTbl[31].entryHI = STACKADDR + i;
		uSup->privatePgTbl[31].entryLO = ALLOFF | DIRTYON;
		/*Initialize sup_exceptContext*/
		resultCode = SYSCALL (CREATEPROCESS, &(uState), uSup, 0);
		if(resultCode != ERRORCODE) {
			SYSCALL(PASSEREN, &procSem,0,0);
		}	
		
	}
	/*Do something to cause test to wait on the u procs*/
	SYSCALL (TERMINATE, 0, 0, 0);
}


