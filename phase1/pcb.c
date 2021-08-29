#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"

pcb_PTR pcbList _h; /*Stack of PCBs*/

void freePcb(pcb_PTR p) {
	/*Insert the element pointed to by p onto the pcbFree list. */
	if(_h == NULL) { /*Stack is Empty*/
	p -> _h; 
	}
	else{ /*Stack is not Empty*/
	pcb_PTR temp = _h;
	_h = p;
	_h->p_next = temp;
	}
}

pcb_PTR allocPcb() {
	/*Return NULL if thje pcbFree list is empty, otherwise remove an element from the*/
	/*pcbFree list and initialize it.*/
	if(_h == NULL) { /*PCB Free List is Empty*/
		return NULL;
	}
	pcb_PTR temp = _h;
	_h = _h.p_next;
	//temp -> p_next = NULL;
	return temp;
}
void initPcbs() {
	/*initializes pcbFree list*/
	static pcb_t procTable[MAXPROC];
	
	for(i=0; i<MAXPROC; i++) {
		freePcb(&(procTable[i]));
	}
}


pcb_PTR mkEmptyProcQ() {
	return (NULL);
}

int emptyProcQ(pcb_PTR *tp) {
	/*Return True if the queue whose tail is pointed to by tp is empty, and False*/	
	/*otherwise. */
	
	return (tp == NULL);
}

void insertProcQ(pcb_PTR *tp, pcb_PTR p) {
	/*Insert the pcb pointed to by p into the process queue whose tail pointer is pointed 
	to by tp.*/
	//store head pointer
	//tp.next = p
	//p.next = head
	pcb_PTR headTemp = &(tp.p_next);
	&(tp) -> p_next = p;
	p -> p_next = headTemp;
	tp = &(p);
}

pcb_PTR removeProcQ(pcb_PTR *tp) {
	/*Remove the first element from the queue whose tail pointer is tp.
	if the queue is empty return NULL, otherwise return a pointer
	to the pcb that was removed. Update tp if necessary */
	if(emptyProcQ(tp)) {
		return NULL;
	}
	pcb_PTR headTemp = &(tp.p_next);
	pcb_PTR newHead = headTemp.p_next;
	&(tp) -> p_next = newHead;
	return headTemp;
	
}

pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p) {
	/*Remove the pcb pointed to by p from the queue pointed to by the tail pointer tp.
	Return NULL if the desired pcb is not in the given queue, otherwise return p.
	Note that p can point to any pcb in the queue. */
	if(emptyProcQ(tp)) {
		return NULL;
	}
	//temp = NULL
	//loop starts at head
	//while(temp == NULL || &(temp) != tail)
	pcb_PTR temp = NULL;
	pcb_PTR current = tp.p_next;
	while(temp == NULL && &(temp) != &(tp))
	{
		if(current == p)
		{
			temp = &(p);
		}
		current = current.p_next;
	}
	if(temp != NULL)
	{
		//remove it from the q
		//previous.next = temp.next;
		//next.previous = temp.previous;
		pcb_PTR tempPrev = temp.p_prev;
		pcb_PTR tempNext = temp.p_next;
		tempPrev -> p_next = &(tempNext);
		tempNext -> p_prev = &(tempPrev);
	}
	return temp; //return temp, either null or equal to the pointer p

}

pcb_PTR headProcQ(pcb_PTR tp) {
	/*Return a pointer to the first pcb from the queue pointed to by the tail pointer tp.
	Do not remove this pcb from the queue. Return NULL if the queue is empty*/
	pcb_PTR temp;
	temp = tp.p_next;
	return temp;
}
