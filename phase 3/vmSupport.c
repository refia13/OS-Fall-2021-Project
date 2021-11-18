/*This is the start of the vmSupport.c module*/
/*This module implements the TLB Exception Handler, the pager, and also reading and writing to flash devices to support the pager*/
/*The swap pool table and swap pool semaphores will be local to this module, and as such they will not be globaly declared in initProc module*/

static int i = 0;
int pickVictim()

/*Page Fault Handler*/
/*Implements the Pager*/
void tlbExceptionHandler() {
	support_t *procsup = SYSCALL (GETSUPPORTT,0,0,0);
	int cause = (procsup->sup_exceptState[0].s_cause & EXCMASK) >>2;
	if(cause == TLBMOD) {
		/*TLB Modification Exception, treat as a program trap*/
		programTrapHandler();
		
	}
	SYSCALL (PASSEREN,&swapSem,0,0); /*Perform a sys3 on the swap pool semaphore (mutex semaphore)*/
	
	/*Determine the missing page number*/
	int p = procsup->sup_exceptionState[0].entryHI & /*some value here to get only the page number*/
	
	/*Pick a frame from the swap pool based on nextFrame*/
	int nextFrame = pickVictim();
	int frame = swapPool[nextFrame];
	/*determine if the frame chosen is occupied*/
	if(frame != NULL) {
		/*turn interrupts off*/
		setStatus(/*Value*/);
		/*update page table of victim*/
		tlbclr();
		support_t *victim = SYSCALL (GETSUPPORTT,0,0,0);
		(*victim) = (*victim) & VALIDMASK;
		/*turn interrupts on*/
		setStatus(/*Value*/);
		/*Write victim page to backing store*/
	}
	/*flash read*/
	/*TUrn interrupts off*/
	setStatus(/*Value*/);
	p = p & VALIDON & nextFrame;
	frame = p;
	
	/*Turn interrupts on*/
	setStatus(/*Value*/);
	tlbclr();
	SYSCALL (VERHOGEN, &swapSem, 0, 0);
	LDST(&(procsup->sup_exceptionState[0]));
}

void uTLB_RefillHandler() {
	support_t *sup = SYSCALL(GETSUPPORTT,0,0,0);
	int exEntryHI = sup->sup_exceptState[PGFAULTEXCEPT].s_entryHI;
	int missingPageNo = (exEntryHI & PAGEMASK) >> VPNMASK;
	
}

int pickVictim() {
	i = (i+1) % POOLSIZE;
	return i;
}
