#ifndef INTERRUPTS
#define INTERRUPTS

/*********************Interrupts.h************************************
*
*        This file declares the externals for interrupts.c
*
*
*
*
*
*
*/

#include "../h/types.h"

extern void interruptHandler(state_PTR exceptionsState);
extern void itInterrupt();
extern void pltInterrupt();
extern void nonTimerInterrupt(devregarea_t *devRegA, int lineNo);
/*******************************************************************/

#endif

