#ifndef SCHEDULE
#define SCHEDULE

/*********************Scheduler.h************************************
*
*        This file declares the externals for scheduler.c
*
*
*
*
*
*
*/

#include "../h/types.h"

extern void scheduler();
extern void switchState(state_PTR newState);
extern void stateCopy(state_t oldState, pcb_PTR current, int passup);
/*******************************************************************/

#endif

