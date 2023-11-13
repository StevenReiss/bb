/********************************************************/
/*							*/
/*		CCCCLANG.C				*/
/*							*/
/*	This module contains the language specific	*/
/*	routines for c. 				*/
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


#define FCTSRCHSTR "^[A-Za-z_][A-Za-z0-9_]*[ ]*("
#define DEFAULT_L4 3
#define DEFAULT_L5 3
#define DEFAULT_L6 1
#define DEFAULT_L7 2




/************************************************************************/
/*									*/
/*	Forward definitions						*/
/*									*/
/************************************************************************/


static	BOOL	isprototype();





/********************************************************/
/*							*/
/*	LCCinit -- initialize for c			*/
/*							*/
/********************************************************/


int  LCCinit()
{
   if (*PSVAL(L1) == 0) PSET(L1,"$");
   if (*PSVAL(L2) == 0) PSET(L2,"^$");
   if (*PSVAL(L3) == 0) PSET(L3,FCTSRCHSTR);
   if (PVAL(L4) == 0) PSET(L4,(1+DEFAULT_L4));
   if (PVAL(L5) == 0) PSET(L5,(1+DEFAULT_L5));
   if (PVAL(L6) == 0) PSET(L6,(1+DEFAULT_L6));
   if (PVAL(L7) == 0) PSET(L7,(1+DEFAULT_L7));

   PSET(stream,FALSE);
   PSET(curstream,FALSE);

   return 0;
}






/********************************************************/
/*							*/
/*	LCCfini -- finish current language		*/
/*							*/
/********************************************************/


int  LCCfini()
{
   if (strcmp(PSVAL(L1),"$") == 0) PSET(L1,"");
   if (strcmp(PSVAL(L2),"^$") == 0) PSET(L2,"");
   if (strcmp(PSVAL(L3),FCTSRCHSTR) == 0) PSET(L3,"");
   if (PVAL(L4) == DEFAULT_L4+1) PSET(L4,0);
   if (PVAL(L5) == DEFAULT_L5+1) PSET(L5,0);
   if (PVAL(L6) == DEFAULT_L6+1) PSET(L6,0);
   if (PVAL(L7) == DEFAULT_L7+1) PSET(L7,0);

   return 0;
}






/********************************************************/
/*							*/
/*	LCCwtab -- tab forwards/backwards one word	*/
/*							*/
/********************************************************/


int LCCwtab(dir,chr,line)
   INT dir;
   INT *chr;			/* resultant position	*/
   DATA *line;
{
   register INT x;
   register DATA y;
   register INT fg;
   register CHAR ch;

#define TEST(c) (strchr(" \t%^&*()-+=~`{}[]'\"\\;:.,<>/?!",c) != NULL)
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
/*	LCCindent -- compute c automatic indentation	*/
/*							*/
/********************************************************/


int LCCindent()
{
   register long y;
   register INT x,lastsig,nestlevel;
   register CHAR ch;
   register INT strflg,cmmtfg,eosfirst,nestfg;
   register INT fctfg,casefg,i,lblfg,rbrfg,cmafg,lbrfg;
   register INT lastsemi,initidnt;

   y = PDVAL(curline);
   lastsig = -1;
   nestlevel = 0;
   strflg = 0;
   cmmtfg = FALSE;
   eosfirst = FALSE;
   nestfg = -1;
   fctfg = FALSE;
   casefg = FALSE;
   lblfg = FALSE;
   rbrfg = FALSE;
   cmafg = -1;
   lbrfg = FALSE;
   lastsemi = -1;
   initidnt = -1;

   LNget(y);			/* check for initial '{' */
   for (x = 0; LBlinebuf[x] == ' '; ++x);
   if (LBlinebuf[x] != 0) initidnt = x;

   /* Check for initial sequences */
   if (LBlinebuf[x] == '#') return(0);
   else if (LBlinebuf[x] == '}') {
      nestlevel = 1;
      eosfirst = TRUE;
      rbrfg = TRUE;
    }
   else if (LBlinebuf[x] == '{') lbrfg = TRUE;
   else if (strncmp((char *) &LBlinebuf[x],"case",4) == 0) casefg = 1;
   else if (strncmp((char *) &LBlinebuf[x],"default",7) == 0 &&
	       (LBlinebuf[x+7] == ':' || isspace(LBlinebuf[x+7])))
      casefg = 1;
   else if (isalnum(LBlinebuf[x])) {
      while (isalnum(LBlinebuf[x])) ++x;
      while (LBlinebuf[x] == ' ') ++x;
      if (LBlinebuf[x] == ':' && LBlinebuf[x+1] != ':') lblfg = TRUE;
    };

   x = 0;

   for ( ; ; )
    { while (x <= 0) {
	 INT lstrfg = 0;
	 --y;
	 fctfg = FALSE;
	 if (y < 0) goto found;
	 LNget(y);
	 if (LBlinebuf[0] == '#') x = 0;
	 else {
	    for (x = 0; LBlinebuf[x] != 0; ++x) {
	       INT chx = LBlinebuf[x];
	       if (chx == '"' || chx == '\'') {
		  if (x == 0 || LBlinebuf[x-1] == '\\') continue;
		  if (lstrfg == 0) lstrfg = chx;
		  else if (lstrfg == chx) lstrfg = 0;
		}
	       else if (lstrfg == 0 && chx == '/' && LBlinebuf[x+1] == '/') break;
	     };
	  };
       };
      ch = LBlinebuf[--x];

      if (strflg != 0)
       { if (ch == strflg && (x == 0 || LBlinebuf[x-1] != '\\'))
	  { strflg = 0; ch = 'X'; }
	 else continue;
       };
      if (cmmtfg)
       { if (ch == '/' && LBlinebuf[x+1] == '*') cmmtfg = FALSE;
	 continue;
       };

      if (ch == '/' && x > 0 && LBlinebuf[x-1] == '*')
       { cmmtfg = TRUE; --x; continue; };

      if (ch == ':' && LBlinebuf[x-1] == ':') {
	 ch = 'X'; LBlinebuf[x] = 'X' ; LBlinebuf[x-1] = 'X';
       };

      switch(ch)
       { case ' '       :
		break;

	 case '"'       :
	 case '\''      :
		strflg = ch;
		break;

	 case '('       :
		if (lastsemi != y && LBlinebuf[x+1] != ')' &&
		       !isprototype(&LBlinebuf[x+1]))
		   fctfg = TRUE;

	 case '{'       :
		if (lastsig < 0 && lastsemi < 0) casefg = 0;
		if (nestlevel > 0)
		 { --nestlevel;
		   if (nestlevel == 0) lastsig = x;
		 }
		else if (lastsig < 0 && x > 0) nestfg = x;
		else if (x == 0 && lastsig < 0) { nestfg = 0; goto found; }
		else goto found;
		cmafg = -1;
		break;

	 case '}'       :
		if (nestlevel == 0) {
		   if (x == 0) { lastsig = -1; nestfg = -1; goto found; };
		   if (lastsig >= 0) goto found;
		      else eosfirst = TRUE;
		 };
		++nestlevel;
		break;

	 case ')'       :
		++nestlevel;
		break;

	 case ':'       :
		if (nestlevel > 0) break;
		i = 0;
		while (LBlinebuf[i] == ' ') ++i;
		if (strncmp((char *) &LBlinebuf[i],"case",4) != 0 &&
		       strncmp((char *) &LBlinebuf[i],"default",7) != 0) {
		   while (isalnum(LBlinebuf[i])) ++i;
		   while (LBlinebuf[i] == ' ') ++i;
		   if (i == x) x = 0;
		 }
		else {
		   if (nestfg < 0 && lastsig < 0) {
		      lastsig = i;
		      nestfg = 1;
		    }
		   goto found;
		 };
		break;

	 case ';'       :
		lastsemi = y;
		if (nestlevel > 0) break;
		if (lastsig >= 0) goto found;
		eosfirst = TRUE;
		break;

	 case ',' :
		if (nestlevel == 0) cmafg = y;
		break;

	 default	 :
		if (nestlevel > 0) break;
		lastsig = x;
		if (x == 0) {
		   if (fctfg && PVAL(L7) != 1) nestfg = 0;
		   lastsig = -1;
		   if (LBlinebuf[0] == '/' && LBlinebuf[1] == '*' && nestfg < 0) {
		      lastsig = initidnt;
		      eosfirst = TRUE;
		    }
		   goto found;
		 };
		break;
       };
    };

found:
   if (lastsig < 0 && nestfg < 0) x = 0;
    else if (lastsig < 0) x = PVAL(L5)-1;
    else if (nestfg >= 0) x = lastsig+PVAL(L5)-1;
    else if (!eosfirst && (cmafg < 0 || cmafg == y)) x = lastsig+PVAL(L4)-1;
    else x = lastsig;

   if (lbrfg && x == PVAL(L5)-1) x = 0;
   if (rbrfg && x != 0) x += PVAL(L6)-1;
   else if (casefg) x -= PVAL(L5)-1;
   else if (lblfg) x -= PVAL(L5)-1;
   if (x < 0) x = 0;

   return x;
}





/************************************************************************/
/*									*/
/*	isprototype -- check for a prototype function arg list		*/
/*									*/
/************************************************************************/


static BOOL
isprototype(s)
   STRING s;
{
   BOOL haveid,inid;

   inid = FALSE;
   haveid = FALSE;

   while (*s != 0 && *s != ')' && *s != ',') {
      if (isalnum(*s) || *s == '$' || *s == '_') {
	 if (!inid && haveid) return TRUE;
	 inid = TRUE;
	 haveid = TRUE;
       }
      else inid = FALSE;
      ++s;
    };

   return FALSE;
}





/* end of cccclang.c */



