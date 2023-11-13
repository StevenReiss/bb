/********************************************************/
/*							*/
/*		JAVALANG.C				*/
/*							*/
/*	This module contains the language specific	*/
/*	routines for java.				*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "langmod.h"
#include "lbuff.h"
#include "parms.h"
#include "cmds.h"
#include "bufmod.h"


/************************************************************************/
/*									*/
/*	Forward definitions						*/
/*									*/
/************************************************************************/





/********************************************************/
/*							*/
/*	JAVAinit -- initialize for c			*/
/*							*/
/********************************************************/


void JAVAinit()
{
   LCCinit();
}






/********************************************************/
/*							*/
/*	JAVAfini -- finish current language		*/
/*							*/
/********************************************************/


void JAVAfini()
{
   LCCfini();
}






/********************************************************/
/*							*/
/*	JAVAindent -- compute c automatic indentation	*/
/*							*/
/********************************************************/


int JAVAindent()
{
   return LCCindent();
}





/* end of javalang.c */



