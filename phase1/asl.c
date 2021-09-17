#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include <limits.h>

static semd_PTR semdFree_h = NULL; /*ASL Free List*/
static semd_PTR semd_h; /*ASL Active List*/

void debugC(int a, int b, int c)
{
	int i = 1;
	i++;
}

semd_PTR semdAlloc(int *semAdd) { /*Helper method that allocates a semaphore descriptor from free list*/
	if(semdFree_h == NULL || semdFree_h == 0)
	{
		
		return NULL;
	}
	
	semd_PTR temp = semdFree_h;
	semdFree_h = semdFree_h->s_next;
	temp -> s_semAdd = semAdd;
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
	semd_PTR tempChild = s -> s_next;
	s -> s_next = s -> s_next -> s_next;
	if(semdFree_h != 0 || semdFree_h != NULL)
	{
		tempChild -> s_next = semdFree_h;
	}
	semdFree_h = tempChild;
	semdFree_h -> s_semAdd = NULL;
	semdFree_h -> s_procQ = mkEmptyProcQ();
}

int insertBlocked(int *semAdd, pcb_PTR p) {
	/*insert the pcb pointed to by p at the tail of the procQ associated with the semaphore whose address is semAdd. If there is no active semaphore, allocate one from the semdFreeList, insert it in the ASL at the sorted position and initialize.*/
	semd_PTR temp = traverseASL(semAdd);
	semd_PTR temp2 = temp -> s_next;
	/*Case 1: semAdd was not found*/
	if((temp2 -> s_semAdd) != semAdd)
	{
		/*allocate a semaphore descriptor*/
		semd_PTR tempNew = semdAlloc(semAdd);
		
		if(tempNew == NULL)
		{
			return TRUE;
		}
		
		tempNew -> s_next = temp2;
		p -> p_semAdd = semAdd;
		temp -> s_next = tempNew;
		
		
	}
	/*Case 2: semAdd was found*/
	insertProcQ(&(temp->s_next->s_procQ),p);
		
	return FALSE;
}

pcb_PTR removeBlocked(int *semAdd) {
	/*Search the ASL for the semaphore descriptor pointed to by semAdd, if none is found Return NULL; otherwise remove the head pcb from the process queue of the found semaphore descriptor and return a pointer to it. If the procQ for the semaphore descriptor becomes empty, remove the semd from the ASL and return it to the free List.*/
	
	semd_PTR temp = traverseASL(semAdd);
	pcb_PTR tempPcb = NULL;
	semd_PTR tempChild = temp -> s_next;
	if(tempChild -> s_semAdd == semAdd)
	{
		tempPcb = removeProcQ(&(temp->s_next->s_procQ));
		
		if(emptyProcQ(tempChild->s_procQ))
		{
			semdDealloc(temp);
		}
	}
	return tempPcb;
}
pcb_PTR outBlocked(pcb_PTR p) {
	/*remove the pcb pointed to by p from the procQ associated with p's semAdd on the ASL, if p's semAdd does not appear on the ASL, return NULL, otherwise return p */
	int *semAdd = p -> p_semAdd;
	debugC((int)semAdd,4,5);
	semd_PTR temp = traverseASL(semAdd);
	semd_PTR tempChild = temp -> s_next;
	pcb_PTR tempPcb = NULL;
	if(tempChild -> s_semAdd == semAdd)
	{
		tempPcb = outProcQ(&(temp->s_next->s_procQ),p);
		debugC((int)tempPcb,300,400);
		if(emptyProcQ(tempChild->s_procQ))
		{
			semdDealloc(temp);
		}
	}
	return tempPcb;
}

pcb_PTR headBlocked(int *semAdd) {
	/*Return a pointer to the pcb that is at the head of the procQ associated with the semAdd. Return NULL if semAdd is not on the ASL or if the procQ associated with the ASL is empty*/
	semd_PTR temp = traverseASL(semAdd);
	semd_PTR tempChild = temp -> s_next;
	if(tempChild->s_semAdd == semAdd)
	{
		if(!emptyProcQ(tempChild->s_procQ))
		{
			return headProcQ((tempChild->s_procQ));
		}
		
	}
	return NULL;
}

void initASL() {
	/*Initialize the semd_tFreeList and active list*/
	static semd_t semdTable[MAXPROC+2];
	int i;
	semdFree_h = NULL;
	
	for(i=1; i<MAXPROC; i++)
	{
		semdTable[i-1].s_next = &semdTable[i];
	}
	semdTable[MAXPROC].s_next = &semdTable[MAXPROC+1];
	semdTable[MAXPROC+1].s_next = NULL;
	semdTable[MAXPROC].s_semAdd = 0;
	semdTable[MAXPROC+1].s_semAdd = NULL;
	semdTable[MAXPROC].s_procQ = mkEmptyProcQ();
	semdTable[MAXPROC+1].s_procQ = mkEmptyProcQ();
	semdFree_h = &semdTable[0];
	semd_h = &semdTable[MAXPROC];
	
	
	
}

