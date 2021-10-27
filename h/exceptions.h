#ifndef EXCEPTIONS
#define EXCEPTIONS

/********************exceptions.h************************************
*
*        This file declares the externals for exceptions.c
*
*
*
*
*
*
*/

#include "../h/types.h"

extern void syscallHandler(int syscallCode);
extern void programTrapHandler();
extern void tlbRefillHandler();
extern void passUpOrDie(unsigned int passUpCode);
extern void createProcess();
extern void terminateProcess(pcb_PTR current);
extern void passeren();
extern void verhogen();
extern void waitForClock();
extern void waitForDevice();

/*******************************************************************/

#endif

