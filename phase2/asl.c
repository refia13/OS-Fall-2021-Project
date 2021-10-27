#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include <limits.h>

static semd_PTR semdFree_h = NULL; /*ASL Free List*/
static semd_PTR semd_h; /*ASL Active List*/


semd_PTR semdAlloc(int *semAdd) { /*Helper method that allocates a semaphore descriptor from free list. Returns a pointer to the semaphore descriptor with its values initialized
				    Or in the case where the free list is empty or its pointer has defaulted to the default zero value, returns NULL*/
	if(semdFree_h == NULL || semdFree_h == 0)
	{
		/*This if statement also checks whether semdFree_h has become the default parameter
		value 0, as trying to access fields of 0 causes a Bus Error*/
		semdFree_h = NULL;
		return NULL;
	}
	
	semd_PTR temp = semdFree_h;
	semdFree_h = semdFree_h->s_next;
	temp -> s_semAdd = semAdd;
	temp -> s_procQ = mkEmptyProcQ();
	return temp;
}



semd_PTR traverseASL(int *semAdd) { /*Helper Method to traverse the ASL. Returns the Parent of the Semaphore Descriptor being searched for whether the semAdd was found or not*/
	
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
	s -> s_next = tempChild -> s_next;
	if(semdFree_h != 0 || semdFree_h != NULL)
	{
		/*Pushes tempChild onto the stack*/
		/*Checks whether the free list is empty or for the special case where its pointer has defaulted back to zero (prevent Bus Error)*/
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
	if((temp2 -> s_semAdd) != semAdd)
	{
		/*A semaphore descriptor with the requested semaphore address was not found*/
		/*allocate a semaphore descriptor*/
		semd_PTR tempNew = semdAlloc(semAdd);
		
		if(tempNew == NULL)
		{
			return TRUE;
		}
		
		tempNew -> s_next = temp2;
		temp -> s_next = tempNew;
		
		
	}
	/*Insert p onto temp's process queue*/
	insertProcQ(&(temp->s_next->s_procQ),p);
	p -> p_semAdd = semAdd;
	return FALSE;
}


pcb_PTR removeBlocked(int *semAdd) {
	/*Search the ASL for the semaphore descriptor pointed to by semAdd, if none is found Return NULL; otherwise remove the head pcb from the process queue of the found semaphore descriptor and return a pointer to it. If the procQ for the semaphore descriptor becomes empty, remove the semd from the ASL and return it to the free List.*/
	
	semd_PTR temp = traverseASL(semAdd);
	pcb_PTR tempPcb = NULL;
	semd_PTR tempChild = temp -> s_next;
	if(tempChild -> s_semAdd == semAdd)
	{
		/*A semaphore descriptor with the matching semAdd has been found
		remove a pcb from the semaphore descriptor's process queue*/
		tempPcb = removeProcQ(&(temp->s_next->s_procQ));
		
		if(emptyProcQ(tempChild->s_procQ))
		{
			/*temp's process queue is empty, remove temp from the ASL*/
			semdDealloc(temp);
		}
	}
	return tempPcb;
}
pcb_PTR outBlocked(pcb_PTR p) {
	/*remove the pcb pointed to by p from the procQ associated with p's semAdd on the ASL, if p's semAdd does not appear on the ASL, return NULL, otherwise return p */
	semd_PTR temp = traverseASL(p->p_semAdd);
	
	semd_PTR tempChild = temp -> s_next;
	
	pcb_PTR tempPcb = NULL;
	if((tempChild -> s_semAdd) == p->p_semAdd)
	{
		/*The requested semaphore descriptor has been found*/
		/*Call outProcQ on the semaphore descriptor's process queue*/
		tempPcb = outProcQ(&(tempChild -> s_procQ),p);
		if(emptyProcQ(tempChild->s_procQ))
		{
			/*temp's process queue is empty, remove temp from the ASL*/
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
		/*The requested semaphore descriptor has been found, return the first entry of its process queue*/
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
	/*Loop through up to MAXPROC times to add the semaphore descriptors to the free list*/
	/*Note that 2 semaphore descriptors are left off the freeList so that they can be used as dummy nodes*/
	for(i=1; i<MAXPROC; i++)
	{
		semdTable[i-1].s_next = &semdTable[i];
	}
	/*Initialize the values of the dummy nodes for the ASL*/
	semdTable[MAXPROC].s_next = &semdTable[MAXPROC+1];
	semdTable[MAXPROC+1].s_next = NULL;
	semdTable[MAXPROC].s_semAdd = 0;
	semdTable[MAXPROC+1].s_semAdd = NULL;
	semdTable[MAXPROC].s_procQ = mkEmptyProcQ();
	semdTable[MAXPROC+1].s_procQ = mkEmptyProcQ();
	semdFree_h = &semdTable[0];
	semd_h = &semdTable[MAXPROC];
	
	
	
}

