/*This is the start if the initProc module for phase 3. It implements test and exports the support level's global variables.*/


void initContext(support_t *sup, int status, int sp)
/*test function, instantiates the processes for phase 3*/
void test() {
	/*Initialize the Phase 3 data structures*/
	/*Initialize and launch the processes*/
	int i;
	for(i = 0; i < UPROCMAX; i++) {
		/*Set up the UPROC state*/
		state_t uState;
		uState.s_pc = TEXTADDR;
		uState.s_t9 = TEXTADDR;
		uState.s_sp = USTACK;
		uState.s_status = ALLOFF | KUCON | KUPON | IEPON | IMON | TEON;
		uState.s_EntryHI = uState.s_EntryHI | /*ASID (i) */;
		/*set up the UPROC support structure*/
		support_t *uSupport;
		uSupport->sup_asid = i;
		int j;
		for(j = 0; j < PGMAX-1; j++) {
			/*Initialize page table*/
			uSupport->sup_privatePgTbl[j] = (STARTPGNO + j) | (STARTASID + i) | (DRON);
		}
		int sSP = uSupport->sup_stackGen[499];
		int sStatus = ALLOFF | IECON | IEPON | IMON | TEON;
		uSupport->sup_exceptContext[0].c_PC = (memaddr) supTlbHandler
		uSupport->sup_exceptContext[1].c_PC = (memaddr supGenExceptionHandler
		initContext(uSupport,sStatus, sSP);
		int retValue = SYSCALL (CREATEPROC, &uState, uSupport, 0);
		if(retValue == ERRORCODE) {
			/*Create Process returned an error, program trap*/
			programTrapHandler();
		}
	}
	/*Do something to cause test to wait on the u procs*/
	SYSCALL (TERMINATE, 0, 0, 0);
}

void initContext(support_t *sup, int status, int sp) {
	int i;
	for(i = 0; i < 2; i++) {
		sup->sup_exceptContext[i].c_status = status;
		sup->sup_exceptContext[i].c_sp = sp;
	}
}
