#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include <limits.h>

static semd_PTR semdFree_h; /*ASL Free List*/
static semd_PTR semd_h; /*ASL Active List*/

void insertSemd(semd_PTR s) { /*Helper Method to Insert semd_ts to the semdFree List*/
	if(semdFree_h == NULL) {
		semdFree_h = s;
	}
	semdFree_h -> s_next = s;
}

semd_PTR semdAlloc() { /*Helper method that allocates a semaphore descriptor from free list*/
	semd_PTR temp = semd_h;
	semd_h = temp -> s_next;
	temp -> s_next = NULL;
	return temp;
}

void semdDealloc(semd_PTR s) {
	/*Helper method that removes the semaphore descriptor pointed to be s from the active list and pushes it onto the free list. */
	s -> s_next = semd_h;
	s -> s_semAdd = NULL;
	s -> s_procQ = NULL;
	semd_h = s;

}

semd_PTR traverseASL(int *semAdd) { /*Helper Method to traverse the ASL*/
	if((*(semd_h -> s_next -> s_semAdd)) == INT_MAX)
	{
		/*ASL is Empty*/
		return NULL;
	}
	int *tempAdd = semd_h -> s_semAdd;
	semd_PTR current = semd_h;
	semd_PTR previous;
	while(tempAdd < semAdd) /*Possible error point*/
	{
		/*traverses the asl, looks for the semd whose semAdd is *semAdd*/
		previous = current;
		current = current -> s_next;
		tempAdd = current -> s_semAdd;
	}
	/*Two cases*/
	
	/*Case 1: semAdd was not found, a larger semAdd was found instead*/
	if(current -> s_semAdd > semAdd)
	{
		return previous;
	}
	/*Case 2: semAdd was found*/
	return current;
}

int insertBlocked(int *semAdd, pcb_PTR p) {
	/*insert the pcb pointed to by p at the tail of the procQ associated with the semaphore whose address is semAdd. If there is no active semaphore, allocate one from the semdFreeList, insert it in the ASL at the sorted position and initialize.*/
	semd_PTR temp = traverseASL(semAdd);
	if(temp -> s_semAdd < semAdd)
	{
		/*Case where p does not have an active semaphore descriptor
		Allocate a semd from the free list*/
		semd_PTR tempNext = temp -> s_next;
		temp -> s_next = semdAlloc();
		temp -> s_next -> s_next = tempNext;
		/*Initialize the semd*/
		temp = temp -> s_next;
		temp -> s_semAdd = semAdd;
	}
	insertProcQ(temp -> s_procQ, p);
	return *semAdd;
}

pcb_PTR removeBlocked(int *semAdd) {
	/*Search the ASL for the semaphore descriptor pointed to by semAdd, if none is found Return NULL; otherwise remove the head pcb from the process queue of the found semaphore descriptor and return a pointer to it. If the procQ for the semaphore descriptor becomes empty, remove the semd from the ASL and return it to the free List.*/
	semd_PTR temp = traverseASL(semAdd);
	/*Case 1: temp's semAdd != semAdd parameter*/
	if(temp -> s_semAdd != semAdd)
	{
		return NULL;
	}
	/*Case 2: temp's semAdd == semAdd parameter*/
	pcb_PTR tempPCB = removeProcQ(temp->s_procQ);
	if(tempPCB == NULL || emptyProcQ(*(temp->s_procQ))) 
	{
		/*deallocate semAdd*/
		semdDealloc(temp);
	}
	return tempPCB;
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
	static semd_PTR semdTable[MAXPROC];
	int i;
	for(i=0; i<MAXPROC; i++)
	{
		insertSemd(semdTable[i]);
	}
	semd_PTR dummyTemp1 = NULL;
	dummyTemp1 -> s_next = NULL;
	dummyTemp1 -> s_semAdd = 0;
	semd_PTR dummyTemp2 = NULL;
	dummyTemp2 -> s_next = NULL;
	dummyTemp2 -> s_semAdd = NULL;
	semd_h = dummyTemp1;
	semd_h -> s_next = dummyTemp2;
	
}

