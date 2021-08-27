#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"

void initPcbs() {
	/*initializes pcbFree list*/
}
void freePcb(pcb_t *p) {
	/*Insert the element pointed to by p onto the pcbFree list. */
}

pcb_t *allocPcb() {
	/*Return NULL if thje pcbFree list is empty, otherwise remove an element from the 	
	/*pcbFree list and initialize it.*/
	
}

pcb_t *mkEmptyProcQ() {
	return (NULL);
}

int emptyProcQ(pcb_t *tp) {
	/*Return True if the queue whose tail is pointed to by tp is empty, and False otherwise. */
	
	return (tp == NULL);
}
