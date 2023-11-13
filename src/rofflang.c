/********************************************************/
/*							*/
/*		ROFFLANG.C				*/
/*							*/
/*	This module contains the language specific	*/
/*	routines for troff/nroff			*/
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





static	int	oldstream,oldcurstream;
static	int	oldtabout,oldcurtabout;


/********************************************************/
/*							*/
/*	LRFinit -- initialize for roff			*/
/*							*/
/********************************************************/


int LRFinit()
{
   if (*PSVAL(L1) == 0)
      PSET(L1,"$");
   if (*PSVAL(L2) == 0)
      PSET(L2,"^$");
   if (*PSVAL(L3) == 0)
      PSET(L3,"^\\.[PLIpli][Pp]");

   oldstream = PVAL(stream);
   oldcurstream = PVAL(curstream);
   oldtabout = PVAL(tabout);
   oldcurtabout = PVAL(curtabout);

   PSET(stream,TRUE);
   PSET(curstream,TRUE);
   PSET(tabout,FALSE);
   PSET(curtabout,FALSE);

   return 0;
}






/********************************************************/
/*							*/
/*	LRFfini -- finish current language		*/
/*							*/
/********************************************************/


int LRFfini()
{
   if (strcmp(PSVAL(L1),"$") == 0) PSET(L1,"");
   if (strcmp(PSVAL(L2),"^$") == 0) PSET(L2,"");
   if (strcmp(PSVAL(L3),"^\\.[PLIpli][Pp]") == 0) PSET(L3,"");

   PSET(stream,(PTR) oldstream);
   PSET(curstream,(PTR) oldcurstream);
   PSET(tabout,(PTR) oldtabout);
   PSET(curtabout,(PTR) oldcurtabout);

   return 0;
}






/* end of dfltlang.c */
