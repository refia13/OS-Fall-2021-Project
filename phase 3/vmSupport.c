/*This is the start of the vmSupport.c module*/
/*This module implements the TLB Exception Handler, the pager, and also reading and writing to flash devices to support the pager*/
/*The swap pool table and swap pool semaphores will be local to this module, and as such they will not be globaly declared in initProc module*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "/usr/include/umps3/umps/libumps.h"

static int i = 0;
int pickVictim();
static int swapSem = 1;
int flashIO(int readFlash, int asid); /*Helper method for flash IO, takes a parameter to determine whether to read or write*/
swap_t swapPool[PGMAX];
debugB(int a) {
	return a;
}
/*Page Fault Handler*/
/*Implements the Pager, uses round robin algorithm to select frames*/ 
/*if frame is already in use when chosen, writes to flash memory*/
void tlbExceptionHandler() {
	support_t *procsup = SYSCALL (GETSUPPORTT,0,0,0);
	int cause = (procsup->sup_exceptState[0].s_cause & EXMASK) >>2;
	if(cause == TLBMOD) {
		/*TLB Modification Exception, treat as a program trap*/
		programTrapHandler(&(procsup->sup_exceptState[GENERALEXCEPT]));
		
	}
	SYSCALL (PASSEREN,&swapSem,0,0); /*Perform a sys3 on the swap pool semaphore (mutex semaphore)*/
	
	/*Determine the missing page number*/
	int p = procsup->sup_exceptState[PGFAULTEXCEPT].s_entryHI & VPNMASK;
	p = p % 32;
	pgte_t *pgptr = &procsup->sup_privatePgTbl[p];
	/*Pick a frame from the swap pool based on nextFrame*/
	int nextFrame = pickVictim();
	swap_t *frame = &swapPool[nextFrame];
	/*determine if the frame chosen is occupied*/
	if(frame != NULL) {
		/*turn interrupts off*/
		setSTATUS((getSTATUS() >> 1) << 1);
		/*update page table of victim*/
		TLBCLR();
		support_t *victim = SYSCALL (GETSUPPORTT,0,0,0);
		frame->pgptr->entryLO & VALIDOFF;
		/*turn interrupts on*/
		setSTATUS(getSTATUS() | IECON);
		/*Write victim page to backing store*/
		flashIO(WRITE, procsup->sup_asid);
	}
	/*flash read*/
	flashIO(READ, procsup->sup_asid);
	/*TUrn interrupts off*/
	setSTATUS((getSTATUS() >> 1) << 1);
	pgptr->entryLO = pgptr->entryLO | VALIDON;
	frame = p;
	
	/*Turn interrupts on*/
	setSTATUS(getSTATUS() | IECON);
	TLBCLR();
	SYSCALL (VERHOGEN, &swapSem, 0, 0);
	LDST(&(procsup->sup_exceptState[PGFAULTEXCEPT]));
}

/*uTLB refill handler, updates tlb with missing page table entry*/
void uTLB_RefillHandler() {

	state_PTR exceptionState = (state_PTR) BIOSDATAPAGE;
	support_t *currSupp = SYSCALL (GETSUPPORTT,0,0,0);
	/*determine page number*/
	int pageNo = exceptionState->s_entryHI & VPNMASK;
	
	pageNo = pageNo % 32;
	debugB(pageNo);
	pgte_t *pageEntry = &(currSupp->sup_privatePgTbl[pageNo]);
	unsigned int eH = pageEntry->entryHI;
	unsigned int eL = pageEntry->entryLO;
	/*Write the page table entry into the tlb*/
	setENTRYHI(pageEntry->entryHI);
	setENTRYLO(pageEntry->entryLO);
	TLBWR();
	/*return control to the currentProc*/
	debugB((int)exceptionState);
	LDST(exceptionState);
	
}

/*Helper method for round robin frame selection algorithm*/
int pickVictim() {
	i = (i+1) % POOLSIZE;
	return i;
}

/*Flash memory io handler, chooses to read or write based on value in readFlash*/
int flashIO(int readFlash, int asid) {
	/*Turn interrupts off*/
	setSTATUS((getSTATUS() >> 1) << 1);
	/*Determine device register*/
	devregarea_t *devrega = (devregarea_t*) RAMBASEADDR;
	int devIndex = ((FLASH - NONPERIPHERALDEV) * DEVPERLINE) + (asid);
	if(readFlash) {
		devrega->devreg[devIndex].d_data0 = (asid*PAGESIZE) + POOLSTARTADDR;
		devrega->devreg[devIndex].d_command = READBLK;
	}
	else{
		devrega->devreg[devIndex].d_data0 = (asid*PAGESIZE) + POOLSTARTADDR;
		devrega->devreg[devIndex].d_command = WRITEBLK;
	}
	setSTATUS(getSTATUS() | IECON);
	return SYSCALL(WAITFORIO, FLASHINT, asid, 0);
}

