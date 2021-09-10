#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"

static pcb_PTR pcbList_h; /*Stack of PCBs*/

void debugA(int a, int b, int c, int d) {
	int i;
	i = 0;
	i++;
}

void freePcb(pcb_PTR p) {
	/*Insert the element pointed to by p onto the pcbFree list. */
	insertProcQ(&pcbList_h, p);
}

pcb_PTR allocPcb() {
	/*Return NULL if thje pcbFree list is empty, otherwise remove an element from the pcbFree list and initialize it.*/
	/*3 Cases, list is empty, list has 2+ before alloc, list has 1 after alloc*/
	pcb_PTR temp = removeProcQ(&pcbList_h);
	
	if(pcbList_h != NULL) {
		temp -> p_next = NULL;
		temp -> p_prev = NULL;
		temp -> p_semAdd = NULL;
	}
	return temp;
}
void initPcbs() {
	/*initializes pcbFree list*/
	static pcb_t procTable[MAXPROC];
	int i;
	
	pcbList_h = mkEmptyProcQ();
	for(i=0; i<MAXPROC; i++) {
		freePcb(&(procTable[i]));
	}
}


pcb_PTR mkEmptyProcQ() {
	return (NULL);
}

int emptyProcQ(pcb_PTR tp) {
	/*Return True if the queue whose tail is pointed to by tp is empty, and False otherwise. */
	
	return (tp == NULL);
}

void insertProcQ(pcb_PTR *tp, pcb_PTR p) {
	/*Insert the pcb pointed to by p into the process queue whose tail pointer is pointed to by tp.*/
	/*store head pointer
	tp.next = p
	p.next = head*/
	
	/*procQ is empty*/
	if(emptyProcQ(*tp)) {
		p -> p_next = p;
		p -> p_prev = p;
		*tp = p;
	}
	else
	{
		pcb_PTR temp = *tp;
		*tp = p;
		p -> p_next = temp -> p_next;
		temp -> p_next = p;
		p-> p_prev = temp;
		p-> p_next -> p_prev = p;
	}
}

pcb_PTR removeProcQ(pcb_PTR *tp) {
	/*Remove the first element from the queue whose tail pointer is tp.
	if the queue is empty return NULL, otherwise return a pointer
	to the pcb that was removed. Update tp if necessary */
	if(emptyProcQ(*tp)) {
		return NULL;
	}
	pcb_PTR headTemp = (*tp)->p_next;
	(*tp)->p_next = headTemp->p_next;
	(*tp)->p_next->p_prev = (*tp);
	
	if(headTemp == (*tp))
	{
		(*tp) = mkEmptyProcQ();
	}
	
	/*pcb_PTR headTemp = outProcQ(tail,tail->next);*/
	return headTemp;
	
}

pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p) {
	/*Remove the pcb pointed to by p from the queue pointed to by the tail pointer tp. Return NULL if the desired pcb is not in the given queue, otherwise return p. Note that p can point to any pcb in the queue. */
	if(emptyProcQ(*tp)) {
		return NULL;
	}
	pcb_PTR current = (*tp)->p_next;
	
	if((current) == (p))
	{
		(*tp) -> p_next -> p_prev = (*tp) -> p_prev;
		(*tp) -> p_prev -> p_next = (*tp) -> p_next;
		(*tp) = (*tp) -> p_prev;
		return current;
	}
	else
	{
		int i;
		for(i=0; i<MAXPROC; i++) {
			pcb_PTR currentNext;
			currentNext = current -> p_next;
			current = &currentNext;
			if((current) == p)
			{
				debugA(8,0,0,8);
				current -> p_prev -> p_next = current -> p_next;
				current -> p_next -> p_prev = current -> p_prev;
				return current;
			}
		}
		
		debugA(4,3,2,1);
	}
	return NULL;

}

pcb_PTR headProcQ(pcb_PTR tp) {
	/*Return a pointer to the first pcb from the queue pointed to by the tail pointer tp. Do not remove this pcb from the queue. Return NULL if the queue is empty*/
	pcb_PTR temp;
	temp = tp -> p_next;
	return temp;
}

int emptyChild(pcb_PTR p) {
	/* Return TRUE if the pcb pointed to by p has no children. Return FALSE otherwise*/
	return(p->p_child == NULL);
}

void insertChild(pcb_PTR prnt, pcb_PTR p) {
	/* Make the pcb pointed to by the p a child of the pcb pointed to by prnt*/
	if (!emptyChild(prnt)) {
		p->p_next_sib = prnt->p_child;
	}

	p->p_prnt = prnt;
	prnt->p_child = p;
}

pcb_PTR removeChild(pcb_PTR p) {
	/* Make the first child of the pcb pointed to by p no longer a child of p. Return NULL if initially there were no children of p. Otherwise, return a pointer to this removed first child pcb*/
	if (emptyChild(p)) {
		return NULL;
	}
	pcb_PTR temp = p -> p_child;
	p->p_child = temp -> p_next_sib;
	return temp;
}

pcb_PTR outChild(pcb_PTR p) {
	/* Make the pcb pointed to by p no longer the child of its parent. If the pcb has no parent return NULL; otherwise return p. Note that the element pointed to by p need not be the first child of its parent*/
	if (p->p_prnt == NULL) {
		return NULL;
	}

	pcb_PTR prev_sib_temp = p->p_prev_sib;
	pcb_PTR next_sib_temp = p->p_next_sib;

	prev_sib_temp->p_next_sib = next_sib_temp; 
	next_sib_temp->p_prev_sib = prev_sib_temp;
	p->p_prnt = NULL;

	return p;
}
