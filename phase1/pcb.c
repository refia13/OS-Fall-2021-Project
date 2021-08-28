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
	p->p_next = temp;
	}
}

pcb_PTR allocPcb() {
	/*Return NULL if thje pcbFree list is empty, otherwise remove an element from the 	
	/*pcbFree list and initialize it.*/
	
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
