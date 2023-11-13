/********************************************************/
/*							*/
/*		FORTLANG.C				*/
/*							*/
/*	This module contains the language specific	*/
/*	routines for fortran 77 			*/
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
#include "linemod.h"



static	int	oldstream,oldcurstream;
static	int	oldcurwidth = 0;


/********************************************************/
/*							*/
/*	LF77init -- initialize for default language	*/
/*							*/
/********************************************************/

#define FCTSRCHSTR "[SsFf][Uu][BbNn][RrCc][OoTt][UuIi][TtOo][IiNn]"

int LF77init()
{
   if (*PSVAL(L1) == 0)
      PSET(L1,"$");
   if (*PSVAL(L2) == 0)
      PSET(L2,"^$");
   if (*PSVAL(L3) == 0)
      PSET(L3,FCTSRCHSTR);
   if (PVAL(L4) == 0)
      PSET(L4,3);

   oldstream = PVAL(stream);
   oldcurstream = PVAL(curstream);
   oldcurwidth = PVAL(curwidth);

   PSET(stream,TRUE);
   PSET(curstream,TRUE);
   PSET(curwidth,72);

   return 0;
}






/********************************************************/
/*							*/
/*	LF77fini -- finish current language		*/
/*							*/
/********************************************************/


int LF77fini()
{
   if (strcmp(PSVAL(L1),"$") == 0) PSET(L1,"");
   if (strcmp(PSVAL(L2),"^$") == 0) PSET(L2,"");
   if (strcmp(PSVAL(L3),FCTSRCHSTR) == 0) PSET(L3,"");
   if (PVAL(L4) == 3) PSET(L4,0);

   PSET(stream,(PTR) oldstream);
   PSET(curstream,(PTR) oldcurstream);

   if (oldcurwidth != 0) PSET(curwidth,(PTR) oldcurwidth);

   return 0;
}






/********************************************************/
/*							*/
/*	LF77indent -- compute normal automatic indentation  */
/*							*/
/********************************************************/


int LF77indent()
{
   register DATA y;
   register INT x,ln,z;

   y = PDVAL(curline);
   LNget(y);
   for (x = 0; LBlinebuf[x] == ' '; ++x);
   if (LBlinebuf[x] == '*' || isdigit(LBlinebuf[x]))
      return 0;
   if (LBlinebuf[x] == '&') return 6;
   z = 0;
   if (strcmp((char *) &LBlinebuf[x],"END IF") == 0 ||
	  strcmp((char *) &LBlinebuf[x],"end if") == 0 ||
	  strcmp((char *) &LBlinebuf[x],"ENDIF") == 0 ||
	  strcmp((char *) &LBlinebuf[x],"endif") == 0 ||
	  strncmp((char *) &LBlinebuf[x],"ELSE",4) == 0 ||
	  strncmp((char *) &LBlinebuf[x],"else",4) == 0 )
      z = PVAL(L4);

   while (--y >= 0)
    { LNget(y);
      ln = strlen((char *) LBlinebuf);
      for (x = 0;
	   LBlinebuf[x] == ' ' || isdigit(LBlinebuf[x]) || LBlinebuf[x] == '&';
	   ++x);
      if ((LBlinebuf[x] == 'D' || LBlinebuf[x] == 'd') &&
	     (LBlinebuf[x+1] == 'O' || LBlinebuf[x+1] == 'o') &&
	     (LBlinebuf[x+2] == ' ')) {
	 x += PVAL(L4);
	 break;
       };
      if (ln >= 4 &&
	     (strcmp((char *) &LBlinebuf[ln-4],"THEN") == 0 ||
		 strcmp((char *) &LBlinebuf[ln-4],"then") == 0 ||
		 strcmp((char *) &LBlinebuf[ln-4],"ELSE") == 0 ||
		 strcmp((char *) &LBlinebuf[ln-4],"else") == 0)) {
	 x += PVAL(L4);
	 break;
       };
      if (x == 0 && (LBlinebuf[x] == 'C' || LBlinebuf[x] == 'c' ||
			LBlinebuf[x] == '*'))
	 continue;
      if (LBlinebuf[x] != 0) break;
    };

   if (x <= 8) z = 0;
   if (y < 0) x = 8;
   else x -= z;

   return x;
}





/* end of fortlang.c */
