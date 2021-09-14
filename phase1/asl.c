#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include <limits.h>

static semd_PTR semdFree_h; /*ASL Free List*/
static semd_PTR semd_h; /*ASL Active List*/

void debugC(int a, int b, int c)
{
	int i = 1;
	i++;
}

void insertSemd(semd_PTR s) { /*Helper Method to Insert semd_ts to the semdFree List*/
	if(semdFree_h == NULL)
	{
		semdFree_h = s;
	}
	else
	{
		semd_PTR temp = semdFree_h;
		s-> s_next = temp;
		semdFree_h = s;
	}
}

semd_PTR semdAlloc() { /*Helper method that allocates a semaphore descriptor from free list*/
	if(semdFree_h == NULL) {
		return NULL;
	}
	semd_PTR temp = semdFree_h;
	semdFree_h = semdFree_h->s_next;
	
	temp -> s_semAdd = NULL;
	temp -> s_procQ = mkEmptyProcQ();
	return temp;
}



semd_PTR traverseASL(int *semAdd) { /*Helper Method to traverse the ASL*/
	
	semd_PTR current = semd_h;
	while((current-> s_next -> s_semAdd) < semAdd) 
	{
		current = current -> s_next;
	}
	return current;
}

void semdDealloc(semd_PTR s) {
	/*Helper method that removes the semaphore descriptor pointed to by s from the active list and pushes it onto the free list. */
	semd_PTR prnt = traverseASL(s->s_semAdd);
	prnt -> s_next = s -> s_next;
	
}

int insertBlocked(int *semAdd, pcb_PTR p) {
	/*insert the pcb pointed to by p at the tail of the procQ associated with the semaphore whose address is semAdd. If there is no active semaphore, allocate one from the semdFreeList, insert it in the ASL at the sorted position and initialize.*/
	semd_PTR temp = traverseASL(semAdd);
	
	if((temp -> s_next -> s_semAdd) == semAdd)
	{
		debugC((int)temp,(int)temp->s_next,(int)temp->s_next->s_procQ);
		temp = temp -> s_next;
		insertProcQ(&(temp -> s_procQ), p);
		
	} 
	else
	{
		semd_PTR newSemd = semdAlloc();
		
		if(newSemd == NULL)
			return TRUE;
		newSemd -> s_next = temp -> s_next;
		
		newSemd -> s_semAdd = semAdd;
		
		p->p_semAdd = semAdd;
		
		insertProcQ(&(newSemd->s_procQ),p);
		temp -> s_next = newSemd;
		
		return FALSE;
	}
	debugC(0,0,0);
	return FALSE;
}

pcb_PTR removeBlocked(int *semAdd) {
	/*Search the ASL for the semaphore descriptor pointed to by semAdd, if none is found Return NULL; otherwise remove the head pcb from the process queue of the found semaphore descriptor and return a pointer to it. If the procQ for the semaphore descriptor becomes empty, remove the semd from the ASL and return it to the free List.*/
	
	semd_PTR temp = traverseASL(semAdd);
	
	if(*(temp -> s_semAdd) != *semAdd)
	{
		return NULL;
	}
	pcb_PTR tempPCB = removeProcQ(&(temp->s_procQ));
	
	if(tempPCB == NULL || emptyProcQ((temp->s_procQ)))
	{
		
		semdDealloc(temp);
	}
	return tempPCB;
}

pcb_PTR outBlocked(pcb_PTR p) {
	/*remove the pcb pointed to by p from the procQ associated with p's semAdd on the ASL, if p's semAdd does not appear on the ASL, return NULL, otherwise return p */
	int *semAdd = p -> p_semAdd;
	semd_PTR temp = traverseASL(semAdd);
	if(temp -> s_semAdd != semAdd)
	{
		return NULL;
	}
	pcb_PTR tempPCB = outProcQ(&(temp->s_procQ),p);
	if(tempPCB == NULL && emptyProcQ((temp->s_procQ)))
	{
		semdDealloc(temp);
	}
	return tempPCB;
}

pcb_PTR headBlocked(int *semAdd) {
	/*Return a pointer to the pcb that is at the head of the procQ associated with the semAdd. Return NULL if semAdd is not on the ASL or if the procQ associated with the ASL is empty*/
	semd_PTR temp = traverseASL(semAdd);
	/*Case 1: SemAdd is not on the ASL*/
	if((temp -> s_semAdd) != semAdd)
	{
		return NULL;
	}
	/*Case 2: semAdd's procQ is empty*/
	if(emptyProcQ((temp -> s_procQ)))
	{
		return NULL;
	}
	/*Case 3: not empty*/
	return headProcQ((temp -> s_procQ));
}

void initASL() {
	/*Initialize the semd_tFreeList and active list*/
	static semd_t semdTable[MAXPROC+2];
	int i;
	semdFree_h = NULL;
	semd_h = NULL;
	for(i=0; i<MAXPROC+2; i++)
	{
		insertSemd(&semdTable[i]);
	}
	semd_h = semdAlloc();
	semd_PTR dummyTemp2 = semdAlloc();
	semd_h -> s_semAdd = 0;
	dummyTemp2 -> s_semAdd = (int *)INT_MAX;
	dummyTemp2 -> s_next = NULL;
	semd_h -> s_next = dummyTemp2;
	
	
}

