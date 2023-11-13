/********************************************************/
/*							*/
/*		DFLTLANG.C				*/
/*							*/
/*	This module contains the language specific	*/
/*	routines for the default case of no particular	*/
/*	language					*/
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




/********************************************************/
/*							*/
/*	LDFinit -- initialize for default language	*/
/*							*/
/********************************************************/


int LDFinit()
{
   if (*PSVAL(L1) == 0)
      PSET(L1,"$");
   if (*PSVAL(L2) == 0)
      PSET(L2,"^$");
   if (*PSVAL(L3) == 0)
      PSET(L3,"^\\.[PLIpli][Pp]");

   return 0;
}






/********************************************************/
/*							*/
/*	LDFfini -- finish current language		*/
/*							*/
/********************************************************/


int LDFfini()
{
   if (strcmp(PSVAL(L1),"$") == 0) PSET(L1,"");
   if (strcmp(PSVAL(L2),"^$") == 0) PSET(L2,"");
   if (strcmp(PSVAL(L3),"^\\.[PLIpli][Pp]") == 0) PSET(L3,"");

   return 0;
}






/********************************************************/
/*							*/
/*	LDFcmds -- language specific commands		*/
/*							*/
/********************************************************/


int LDFcmds(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT cnt,buf;

   cnt = (INT)(long) val;
   if (cnt == 0) cnt = 1;

   switch(cmd)
    { case CM_1LANG : buf = BF_A; break;
      case CM_2LANG : buf = BF_B; break;
      case CM_3LANG : buf = BF_C; break;
      case CM_4LANG : buf = BF_D; break;
      case CM_5LANG : buf = BF_E; break;
      case CM_6LANG : buf = BF_F; break;
      case CM_7LANG : buf = BF_G; break;
      case CM_8LANG : buf = BF_H; break;
      case CM_9LANG : buf = BF_I; break;
      case CM_0LANG : buf = BF_J; break;
    };

   DOstart(buf,cnt);

   return TRUE;
}






/********************************************************/
/*							*/
/*	LDFwtab -- tab forwards/backwards one word	*/
/*							*/
/********************************************************/


int LDFwtab(dir,chr,line)
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
/*	LDFetab -- tab to end of line			*/
/*							*/
/********************************************************/


int LDFetab(dir,chr,line)
   INT dir;
   INT *chr;			/* result position	*/
   DATA *line;
{
   return SEarch(PSVAL(L1),dir,TRUE,line,chr);
}





/********************************************************/
/*							*/
/*	LDFbtab -- tab to next/last blank line		*/
/*							*/
/********************************************************/


int LDFbtab(dir,chr,line)
   INT dir;
   INT *chr;			/* resultant position	*/
   DATA *line;
{
   return SEarch(PSVAL(L2),dir,TRUE,line,chr);
}





/********************************************************/
/*							*/
/*	LDFstab -- tab to next L2 string		*/
/*							*/
/********************************************************/


int LDFstab(dir,chr,line)
   INT dir;
   INT *chr;
   DATA *line;
{
   return SEarch(PSVAL(L3),dir,TRUE,line,chr);
}





/********************************************************/
/*							*/
/*	LDFhtab -- half tab				*/
/*							*/
/********************************************************/


int LDFhtab(dir,chr,line)
   INT dir;
   INT *chr;
   DATA *line;
{
   register INT x;

   x = (PVAL(curchar)+(dir < 0 ? -1 : 4))&(~3);
   if (x < PVAL(curmargin) || x >= PVAL(curmargin)+PVAL(curwidth))
      return FALSE;

   *chr = x;
   *line = PDVAL(curline);

   return TRUE;
}






/********************************************************/
/*							*/
/*	LDFindent -- compute normal automatic indentation  */
/*							*/
/********************************************************/


int LDFindent()
{
   register DATA y;
   register INT x;

   y = PDVAL(curline);

   while (--y >= 0)
    { LNget(y);
      for (x = 0; LBlinebuf[x] == ' '; ++x);
      if (LBlinebuf[x] != 0) break;
    };
   if (y < 0) x = 0;

   return x;
}





/* end of dfltlang.c */
