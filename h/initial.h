#ifndef INITIAL
#define INITIAL

/*********************Initial.h************************************
*
*        This file declares the externals for initial.c
*
*
*
*
*
*
*/

#include "../h/types.h"
#include "../h/const.h"
extern int processCount;
extern int blockedCount;
extern pcb_PTR readyQ;
extern pcb_PTR currentProc;
extern int deviceSema4s[SEMCOUNT];
#define clockSem deviceSema4s[SEMCOUNT - 1]

/*******************************************************************/

#endif

