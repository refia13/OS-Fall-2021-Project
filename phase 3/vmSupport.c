/*This is the start of the vmSupport.c module*/
/*This module implements the TLB Exception Handler, the pager, and also reading and writing to flash devices to support the pager*/
/*The swap pool table and swap pool semaphores will be local to this module, and as such they will not be globaly declared in initProc module*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "/usr/include/umps3/umps/libumps.h"
#include "../h/initial.h";

static int i = 0; /*Static index for the swap pool. pick victim will increment this index % poolsize*/
int pickVictim(); /*Helper method that selects a frame index from the swap pool, round robin algorithm*/
static int swapSem = 1; /*Mutex Semaphore for the Swap Pool*/
int flashIO(int readFlash, int asid, int frameNo); /*Helper method for flash IO, takes a parameter to determine whether to read or write*/
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
	/*Gain mutex over the swap pool*/
	SYSCALL (PASSEREN,&swapSem,0,0); /*Perform a sys3 on the swap pool semaphore (mutex semaphore)*/
	/*Determine the missing page number*/
	int missingNo = (procsup->sup_exceptState[PGFAULTEXCEPT].s_entryHI & VPNMASK) >>VPNSHIFT;
	missingNo = missingNo % 32;
	/*Pick a frame in the swap pool*/
	int victimIndex = pickVictim();
	swap_t *frame = &swapPool[victimIndex];
	int retCode;
	if(frame->id != -1) {
		/*Frame is Occupied*/
		setSTATUS(getSTATUS() & IECOFF);
		frame->pgptr->entryLO = frame->pgptr->entryLO & VALIDOFF;
		/*Write page to backing store*/
		retCode = flashIO(WRITE, (frame->id), frame->pageNo);
		if(retCode == ERRORCODE) {
			programTrapHandler();
		}
		setSTATUS(getSTATUS() | IECON);
	}
	/*Read in page from backing store*/
	retCode = flashIO(READ, (procsup->sup_asid), missingNo);
	debugB(100);
	setSTATUS(getSTATUS() & IECOFF);
	frame->id = procsup->sup_asid;
	frame->pageNo = missingNo;
	procsup->sup_privatePgTbl[missingNo].entryLO = (victimIndex << VPNSHIFT) | VALIDON | DIRTYON;
	frame->pgptr = &(procsup->sup_privatePgTbl[missingNo]);
	TLBCLR();
	setSTATUS(getSTATUS() | IECON);
	/*Release Mutex over the swap pool*/
	SYSCALL(VERHOGEN, &swapSem, 0, 0);
	LDST(&(procsup->sup_exceptState[PGFAULTEXCEPT]));
	
	
}

/*uTLB refill handler, updates tlb with missing page table entry. Has access to relevant nucleus level data structures (current proc)*/
void uTLB_RefillHandler() {
	/*Determine the missing page number by inspecting the entryHI register of the exception
	state stored in the bios data page*/
	state_PTR exceptionState = (state_PTR) BIOSDATAPAGE;
	int missingNo = (exceptionState->s_entryHI & VPNMASK) >> VPNSHIFT;
	missingNo = missingNo % 32;
	/*Get the page table entry for page number missingNo for the currentProcess.*/
	support_t *pageSup = currentProc->p_supportStruct;
	/*Write the missing entry into the TLB*/
	setENTRYHI(pageSup->sup_privatePgTbl[missingNo].entryHI);
	setENTRYLO(pageSup->sup_privatePgTbl[missingNo].entryLO);
	
	TLBWR();
	LDST(exceptionState);
	
}

/*Helper method for round robin frame selection algorithm*/
int pickVictim() {
	i = (i+1) % POOLSIZE;
	return i;
}

/*Flash memory io handler, chooses to read or write based on value in readFlash*/
int flashIO(int readFlash, int asid, int frameNo) {
	/*Turn interrupts off*/
	setSTATUS((getSTATUS() & IECOFF));
	/*Determine device register*/
	devregarea_t *devrega = (devregarea_t*) BUSADDRESS;
	/*Get the start address of the swap pool*/
	memaddr ramtop = (devrega->rambase) + (devrega->ramsize);
	memaddr poolstartaddr = 0x20020000;
	/*Write the starting physical address to data0 and write the command with a block number based on frameNo*/
	int devIndex = ((FLASH - NONPERIPHERALDEV) * DEVPERLINE) + (asid-1);
	devrega->devreg[devIndex].d_data0 = POOLSTARTADDR + (frameNo*PAGESIZE);
	if(readFlash) {
		
		devrega->devreg[devIndex].d_command = ((frameNo) << FRAMESHIFT) | READBLK;
	}
	else{
		devrega->devreg[devIndex].d_command = ((frameNo) << FRAMESHIFT) | WRITEBLK;
	}
	/*Block process until IO completes*/
	return SYSCALL(WAITFORIO, FLASHINT, asid-1, 0);
}

