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
extern void passUpOrDie(unsigned int exceptCode);

/*******************************************************************/

#endif

