#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"

static semd_PTR semdFree_h; //ASL Free List

int insertBlocked(int *semAdd, pcb_PTR p) {
	/*insert the pcb pointed to by p at the tail of the procQ associated with the semaphore 
	whose address is semAdd. If there is no active semaphore, allocate one from the 
	semdFreeList, insert it in the ASL at the sorted position and initialize.*/
	
	
}

pcb_PTR removeBlocked(int *semAdd) {

}

pcb_PTR outBlocked(pcb_PTR p) {

}

pcb_PTR headBlocked(int *semAdd) {

}

void initASL() {
	//Initialize the semd_tFreeList
	static semd_t semdTable[MAXPROC];
	for(int i=0; i<MAXPROC; i++)
	{
		
	{
}

