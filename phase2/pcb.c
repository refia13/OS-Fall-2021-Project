#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"

static pcb_PTR pcbList_h; /*Stack of PCBs*/
void debugG(int a) {
	int i = 0;
	i++;
}
void freePcb(pcb_PTR p) {
	/*Insert the element pointed to by p onto the pcbFree list. */
	insertProcQ(&pcbList_h, p);
}

pcb_PTR allocPcb() {
	/*Return NULL if thje pcbFree list is empty, otherwise remove an element from the pcbFree list and initialize it.*/
	if(pcbList_h == NULL)
	{
		return NULL; /*Free List is already empty*/
	}
	/*Remove a pcb from the free list*/
	pcb_PTR temp = removeProcQ(&pcbList_h);
	
	if(pcbList_h != NULL) 
	{
		/*Initialize the values of the PCB*/
		temp -> p_next = NULL;
		temp -> p_prev = NULL;
		temp -> p_semAdd = NULL;
		temp -> p_child = NULL;
		temp -> p_prnt = NULL;
		temp -> p_next_sib = NULL;
		temp -> p_prev_sib = NULL;
		temp -> p_time = 0;
		temp -> p_supportStruct = NULL;
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
	/*Initializes the queue to be empty*/
	return (NULL);
}

int emptyProcQ(pcb_PTR tp) {
	/*Return True if the queue whose tail is pointed to by tp is empty, and False otherwise. */
	return (tp == NULL);
}

void insertProcQ(pcb_PTR *tp, pcb_PTR p) {
	/*Insert the pcb pointed to by p into the process queue whose tail pointer is pointed to by tp.*/
	
	if(emptyProcQ(*tp)) 
	{
		/*procQ is empty*/
		p -> p_next = p;
		p -> p_prev = p;
	}
	else
	{
		/*Append p to the end of the queue and reassign the tail*/
		pcb_PTR temp = (*tp);
		p->p_next = temp;
		p->p_prev = temp->p_prev;
		temp->p_prev = p;
		p->p_prev->p_next = p;
	}
	(*tp) = p;
}

pcb_PTR removeProcQ(pcb_PTR *tp) {
	/*Remove the first element from the queue whose tail pointer is tp.
	if the queue is empty return NULL, otherwise return a pointer
	to the pcb that was removed. Update tp if necessary */
	if(emptyProcQ(*tp)) 
	{
		/*procQ is empty*/
		return NULL;
	}
	pcb_PTR tail = *tp;
	if(tail->p_prev == tail) {
		(*tp) = NULL;
		return tail;
	}
	else {
		pcb_PTR remove = tail->p_prev;
		remove->p_prev->p_next = remove->p_next;
		remove->p_next->p_prev = remove->p_prev;
		return remove;
	}
	return NULL;
	
}

pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p) {
	/*Remove the pcb pointed to by p from the queue pointed to by the tail pointer tp. Return NULL if the desired pcb is not in the given queue, otherwise return p. Note that p can point to any pcb in the queue. */
	pcb_PTR tail = (*tp);
	if(emptyProcQ(p)) return NULL;
	if(emptyProcQ(tail)) return NULL;
	pcb_PTR temp = tail->p_next;
	while(temp != p && temp != tail) {
		temp = temp->p_next;
	}
	if(temp == p) {
		p->p_next->p_prev = p->p_prev;
		p->p_prev->p_next = p->p_next;
		if(temp == tail && tail->p_next == tail) {
			(*tp) = NULL;
		}
		else if(temp == tail) {
			(*tp) = tail->p_next;
		}
		return temp;
	}
	else { return NULL; }
}

pcb_PTR headProcQ(pcb_PTR tp) {
	/*Return a pointer to the first pcb from the queue pointed to by the tail pointer tp. Do not remove this pcb from the queue. Return NULL if the queue is empty*/
	if(emptyProcQ(tp)) {
		return NULL; }
	return (tp->p_prev);
}

int emptyChild(pcb_PTR p) {
	/* Return TRUE if the pcb pointed to by p has no children. Return FALSE otherwise*/
	return(((p->p_child) == NULL));
}

void insertChild(pcb_PTR prnt, pcb_PTR p) {
	/* Make the pcb pointed to by the p a child of the pcb pointed to by prnt*/
	if(!emptyProcQ(p)) {
	if(emptyChild(prnt)) {
		prnt->p_child = p;
		p->p_prnt = prnt;
		p->p_next_sib = NULL;
		p->p_prev_sib = NULL;
	}
	else {
		p->p_next_sib = prnt->p_child;
		p->p_next_sib->p_prev_sib = p;
		prnt->p_child = p;
	}
	}
}

pcb_PTR removeChild(pcb_PTR p) {
	/* Make the first child of the pcb pointed to by p no longer a child of p. Return NULL if initially there were no children of p. Otherwise, return a pointer to this removed first child pcb*/
	if (emptyChild(p)) {
		return NULL;
	}
	/*Shift tree and remove child*/
	pcb_PTR child = p->p_child;
	p->p_child = child->p_next_sib;
	p->p_child->p_prev_sib = NULL;
	return child;
}

pcb_PTR outChild(pcb_PTR p) {
	/* Make the pcb pointed to by p no longer the child of its parent. If the pcb has no parent return NULL; otherwise return p. Note that the element pointed to by p need not be the first child of its parent*/
	if ((p->p_prnt) == NULL) 
	{
		/*p has no parent*/
		return NULL;
	}
	pcb_PTR prnt = p->p_prnt;
	pcb_PTR currC = prnt->p_child;
	pcb_PTR lastChild = NULL;
	if(currC == p) {
		return removeChild(prnt);
	}
	while(currC != NULL) {
		if(currC == p) {
			lastChild->p_next_sib = p->p_next_sib;
			lastChild->p_prev_sib = p->p_prev_sib;
			return p;
		}
		lastChild = currC;
		currC = currC->p_next_sib;
	}
	return NULL;
}
