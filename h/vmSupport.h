#ifndef VMSUPPORT
#define VMSUPPORT

/*********************vmSupport.h************************************
*
*        This file declares the externals for vmSupport.h
*
*
*
*
*
*
*/

#include "../h/types.h"
#include "../h/const.h"
extern int pickVictim();
extern void tlbExceptionHandler();
extern void uTLB_RefillHandler();
extern int  flashIO(int command, int id);
/*******************************************************************/

#endif

