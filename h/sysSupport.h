#ifndef SYSSUPPORT
#define SYSSUPPORT

/*********************sysSupport.h************************************
*
*        This file declares the externals for sysSupport.h
*
*
*
*
*
*
*/

#include "../h/types.h"
#include "../h/const.h"
extern void supGenExceptionHandler();
extern void supSyscallHandler(state_PTR exceptState);
extern void programTrapHandler(state_PTR exceptState);
extern void terminateUProc();
extern unsigned int getTod();
extern int writePrinter();
extern int writeTerminal();
extern int readTerminal();
/*******************************************************************/

#endif

