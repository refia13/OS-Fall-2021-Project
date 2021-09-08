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
	if(pcbList_h == NULL) { /*Stack is Empty*/
		pcbList_h = p;
	}
	else{ /*Stack is not Empty*/
		pcbList_h -> p_prev = p;
		p -> p_next = pcbList_h;
		pcbList_h = p;
	}
}

pcb_PTR allocPcb() {
	/*Return NULL if thje pcbFree list is empty, otherwise remove an element from the pcbFree list and initialize it.*/
	/*3 Cases, list is empty, list has 2+ before alloc, list has 1 after alloc*/
	if(pcbList_h == NULL) {
		return NULL; /*Free List is Empty*/
	}
	pcb_PTR temp = (pcbList_h);
	pcbList_h = pcbList_h -> p_next;
	if(pcbList_h == temp)
	{
		
		pcbList_h -> p_next = NULL;
		pcbList_h -> p_prev = NULL;
		pcbList_h = NULL;
	}
	temp -> p_next = NULL;
	temp -> p_prev = NULL;
	debugA((int)temp,2,3,4);
	return temp;
}
void initPcbs() {
	/*initializes pcbFree list*/
	static pcb_t procTable[MAXPROC];
	int i;
	
	pcbList_h = NULL;
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
		*tp = p;
	}
	else {
		(*tp) -> p_next = p;
		p -> p_prev = *tp;
	}
}

pcb_PTR removeProcQ(pcb_PTR *tp) {
	/*Remove the first element from the queue whose tail pointer is tp.
	if the queue is empty return NULL, otherwise return a pointer
	to the pcb that was removed. Update tp if necessary */
	if(emptyProcQ(*tp)) {
		return NULL;
	}
	pcb_PTR headTemp = (*tp);
	(*tp) = (*tp) -> p_next;
	(*tp) -> p_prev = headTemp -> p_prev;
	
	return headTemp;
	
}

pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p) {
	/*Remove the pcb pointed to by p from the queue pointed to by the tail pointer tp. Return NULL if the desired pcb is not in the given queue, otherwise return p. Note that p can point to any pcb in the queue. */
	if(emptyProcQ(*tp)) {
		return NULL;
	}
	/*temp = NULL
	loop starts at head
	while(temp == NULL || &(temp) != tail)*/
	pcb_PTR temp = NULL;
	pcb_PTR current = (*tp) -> p_next;
	while((temp == NULL) && ((temp) != (*tp)))
	{
		if(current == p)
		{
			temp = (p);
		}
		current = current -> p_next;
	}
	if(temp != NULL)
	{
		/*remove it from the q
		previous.next = temp.next;
		next.previous = temp.previous;*/
		pcb_PTR tempPrev = temp -> p_prev;
		pcb_PTR tempNext = temp -> p_next;
		tempPrev -> p_next = (tempNext);
		tempNext -> p_prev = (tempPrev);
	}
	return temp; /*return temp, either null or equal to the pointer p*/

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
