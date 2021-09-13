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
	if(semdFree_h == NULL) {
		semdFree_h = s;
	}
	else {
	semd_PTR temp = semdFree_h;
	semdFree_h = s;
	s-> s_next = temp;
	}
}

semd_PTR semdAlloc() { /*Helper method that allocates a semaphore descriptor from free list*/
	if(semdFree_h == NULL) {
		return NULL;
	}
	semd_PTR temp = semdFree_h;
	semdFree_h = temp->s_next;
	temp -> s_next = NULL;
	temp -> s_semAdd = NULL;
	temp -> s_procQ = NULL;
	return temp;
}

void semdDealloc(semd_PTR toBeRemoved, semd_PTR prnt) {
	/*Helper method that removes the semaphore descriptor pointed to by s from the active list and pushes it onto the free list. */
	prnt -> s_next = toBeRemoved -> s_next;
	
	semd_PTR temp = semdFree_h;
	
	toBeRemoved -> s_next = temp;
	
	semdFree_h = toBeRemoved;
	
}

semd_PTR traverseASL(int *semAdd) { /*Helper Method to traverse the ASL*/
	
	semd_PTR current = semd_h -> s_next;

	semd_PTR previous = semd_h;
	
	int *tempAdd = semd_h -> s_semAdd;
	
	while(tempAdd < semAdd) 
	{
		/*traverses the asl, looks for the semd whose semAdd is *semAdd*/
		if((current -> s_semAdd) == NULL)
		{
			return previous;
		}
		previous = current;
		current = current -> s_next;
		tempAdd = current -> s_semAdd;

	}

	return previous;
}

int insertBlocked(int *semAdd, pcb_PTR p) {
	/*insert the pcb pointed to by p at the tail of the procQ associated with the semaphore whose address is semAdd. If there is no active semaphore, allocate one from the semdFreeList, insert it in the ASL at the sorted position and initialize.*/
	semd_PTR temp = traverseASL(semAdd);
	if(temp == NULL)
	{
		/*error case*/
		return TRUE;
	}
	
	if((temp -> s_semAdd) == semAdd)
	{
		
		insertProcQ((temp -> s_procQ), p);
		return FALSE;
	} 
	else
	{
		semd_PTR newSemd = semdAlloc();
		if(newSemd == NULL)
		{
			return TRUE;
		}
		
		newSemd -> s_next = temp -> s_next;
		
		newSemd -> s_semAdd = semAdd;
		
		newSemd -> s_procQ = &p;
		
		temp -> s_next = newSemd;
	
	}
	return FALSE;
}

pcb_PTR removeBlocked(int *semAdd) {
	/*Search the ASL for the semaphore descriptor pointed to by semAdd, if none is found Return NULL; otherwise remove the head pcb from the process queue of the found semaphore descriptor and return a pointer to it. If the procQ for the semaphore descriptor becomes empty, remove the semd from the ASL and return it to the free List.*/
	debugC(4,5,6);
	semd_PTR tempPrnt = traverseASL(semAdd);
	semd_PTR tempChld = tempPrnt -> s_next;
	
	if(tempChld -> s_semAdd != semAdd)
	{
		return NULL;
	}
	
	pcb_PTR tempPcb = removeProcQ(tempChld->s_procQ);
	
	if(emptyProcQ(*tempChld->s_procQ))
	{
		semdDealloc(tempPrnt, tempChld);
	}
	debugC((int)tempPcb,3,5);
	return tempPcb;
}

pcb_PTR outBlocked(pcb_PTR p) {
	/*remove the pcb pointed to by p from the procQ associated with p's semAdd on the ASL, if p's semAdd does not appear on the ASL, return NULL, otherwise return p */
	int *tempAdd = p -> p_semAdd;
	semd_PTR search = traverseASL(tempAdd);
	if(search == NULL || search -> s_semAdd < tempAdd)
	{
		/*semaphore address does not appear on the ASL*/
		return NULL;
	}
	/*Semaphore Address is on the ASL*/
	return outProcQ((search -> s_procQ), p);
}

pcb_PTR headBlocked(int *semAdd) {
	/*Return a pointer to the pcb that is at the head of the procQ associated with the semAdd. Return NULL if semAdd is not on the ASL or if the procQ associated with the ASL is empty*/
	semd_PTR temp = traverseASL(semAdd);
	/*Case 1: SemAdd is not on the ASL*/
	if(temp -> s_semAdd != semAdd)
	{
		return NULL;
	}
	/*Case 2: semAdd's procQ is empty*/
	if(emptyProcQ(*(temp -> s_procQ)))
	{
		return NULL;
	}
	/*Case 3: not empty*/
	return headProcQ(*(temp -> s_procQ));
}

void initASL() {
	/*Initialize the semd_tFreeList and active list*/
	static semd_t semdTable[MAXPROC+2];
	int i;
	semdFree_h = NULL;
	for(i=0; i<MAXPROC; i++)
	{
		insertSemd(&semdTable[i]);
	}
	semd_PTR dummyTemp1 = &semdTable[MAXPROC];
	semd_PTR dummyTemp2 = &semdTable[MAXPROC+1];
	dummyTemp1 -> s_next = dummyTemp2;
	dummyTemp1 -> s_semAdd = 0;
	dummyTemp2 -> s_next = NULL;
	dummyTemp2 -> s_semAdd = NULL;
	semd_h = dummyTemp1;
	
}

