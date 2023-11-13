/********************************************************/
/*							*/
/*		COPSLANG.C				*/
/*							*/
/*	This module contains the language specific	*/
/*	routines for cops.	(pss 7/83).		*/
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


#define FCTSRCHSTR "^[A-Za-z_]"



/********************************************************/
/*							*/
/*	LCOinit -- initialize for cops			*/
/*							*/
/********************************************************/


int LCOinit()
{
   if (*PSVAL(L1) == 0)
      PSET(L1,"$");
   if (*PSVAL(L2) == 0)
      PSET(L2,"^$");
   if (*PSVAL(L3) == 0)
      PSET(L3,FCTSRCHSTR);
   if (PVAL(L4) == 0)
      PSET(L4,3);
   if (PVAL(L5) == 0)
      PSET(L5,3);

   PSET(stream,FALSE);
   PSET(curstream,FALSE);

   return 0;
}






/********************************************************/
/*							*/
/*	LCOfini -- finish current language		*/
/*							*/
/********************************************************/


int LCOfini()
{
   if (strcmp(PSVAL(L1),"$") == 0) PSET(L1,"");
   if (strcmp(PSVAL(L2),"^$") == 0) PSET(L2,"");
   if (strcmp(PSVAL(L3),FCTSRCHSTR) == 0) PSET(L3,"");
   if (PVAL(L4) == 3) PSET(L4,0);
   if (PVAL(L5) == 3) PSET(L5,0);

   return 0;
}






/********************************************************/
/*							*/
/*	LCOwtab -- tab forwards/backwards one word	*/
/*							*/
/********************************************************/


int LCOwtab(dir,chr,line)
   INT dir;
   INT *chr;			/* resultant position	*/
   DATA *line;
{
   register INT x;
   register DATA y;
   register INT fg;
   register CHAR ch;

#define TEST(c) (strchr(" \t",c) != NULL)
#define EOLSTOP TRUE
#define BLANKSTOP FALSE

   x = PVAL(curchar);
   y = PDVAL(curline);

   if (dir > 0 && y >= PDVAL(filesize)) return FALSE;
   if (dir < 0 && y == 0 && x == 0) return FALSE;

   fg = FALSE;

   for ( ; dir > 0; --dir)
    { LNget(y);
      if (x >= (int) strlen((char *) LBlinebuf))
       { x = 0;
	 fg = TRUE;
	 if (y < PDVAL(filesize)) { ++y; LNget(y); }
	    else break;
       }
      else
       { while ((ch = LBlinebuf[x++]) != 0 && !TEST(ch)) fg = TRUE;
	 --x;
       };
      while (LBlinebuf[x] != 0 && TEST(LBlinebuf[x])) ++x;
      if (!EOLSTOP && LBlinebuf[x] == 0 && x != 0) ++dir;
	 else if (!BLANKSTOP && LBlinebuf[x] == 0 && x == 0) ++dir;
	 else if (LBlinebuf[x] == 0 && !fg) ++dir;
      fg = TRUE;
    };

   for ( ; dir < 0; ++dir)
    { LNget(y);
      if (x > (int) strlen((char *) LBlinebuf))
       { x = strlen((char *) LBlinebuf);
	 if (EOLSTOP && x != 0) continue;
	 if (BLANKSTOP && x == 0) continue;
       };
      fg = 0;
      while (--x >= 0)
	 if (!TEST(LBlinebuf[x])) fg = 1;
	    else if (fg == 1) { ++x; break; };
      if (x < 0) {
	 if (y == 0 || fg != 0)
	  { x = 0;
	    if (y == 0) break;
	  }
	 else
	  { LNget(--y);
	    x = strlen((char *) LBlinebuf);
	    if (!EOLSTOP && x != 0) --dir;
	       else if (!BLANKSTOP && x == 0) --dir;
	  };
       }
    };

   if (x > 0 && LBlinebuf[x] == 0)
      while (x > 0 && strchr(" \t",LBlinebuf[x-1]) != NULL) --x;

   *chr = x;
   *line = y;

   return TRUE;
}





/********************************************************/
/*							*/
/*    LCOindent -- compute cops automatic indentation	*/
/*							*/
/********************************************************/


int LCOindent()
{
   register DATA y;
   register INT  x, indent, end, cur_nest, prev_nest;
   INT		 prev_ended, prev_prev_ended;

   y = PDVAL(curline);
   LNget(y);
   for (x = 0; LBlinebuf[x] == ' '; ++x);
   if (LBlinebuf[x] == '#' || LBlinebuf[x] == '$') return(0);
   cur_nest = prev_nest = 0;
   for (; x < (int) strlen((char *) LBlinebuf); x++) if (LBlinebuf[x] == '}') --cur_nest;

   prev_ended = prev_prev_ended = TRUE;
   for (end = -1; end < 0 && --y >= 0;) {
      LNget(y);
      for (x = 0; LBlinebuf[x] == ' '; ++x);
      if (LBlinebuf[x] != '#' && LBlinebuf[x] != '$')
	 for (end = strlen((char *) LBlinebuf) - 1; LBlinebuf[end] == ' '; --end);
   }
   if (y < 0) return(0);
   indent = x;
   for (; x <= end; x++) {
      if      (LBlinebuf[x] == '{')              ++prev_nest;
      else if (LBlinebuf[x] == '}' && prev_nest) --prev_nest;
   }
   if (LBlinebuf[end] != '{' && LBlinebuf[end] != '}') {
      prev_ended = (LBlinebuf[end] == ';' ||
		       (LBlinebuf[end] == '>' && LBlinebuf[end-1] == '<'));
      for (end = 0; end == 0 && --y >= 0; ) {
	 LNget(y);
	 for (x = 0; LBlinebuf[x] == ' '; ++x);
	 if (LBlinebuf[x] != '#' && LBlinebuf[x] != '$')
	    for (end = strlen((char *) LBlinebuf) - 1; LBlinebuf[end] == ' '; --end);
      }
      prev_prev_ended = (y < 0 ||
			    LBlinebuf[end] == '{' ||
			    LBlinebuf[end] == '}' ||
			    LBlinebuf[end] == ';' ||
			    (LBlinebuf[end] == '>' && LBlinebuf[end-1] == '<'));
   }
   indent += (cur_nest + prev_nest) * PVAL(L5);
   if	   (prev_ended && ! prev_prev_ended) indent -= PVAL(L4);
   else if (! prev_ended && prev_prev_ended) indent += PVAL(L4);

   return(indent < 0 ? 0 : indent);
}





/* end of copslang.c */
