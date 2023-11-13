/********************************************************/
/*							*/
/*		XMLLANG.C				*/
/*							*/
/*	This module contains the language		*/
/*	specific routines for xml			*/
/*							*/
/********************************************************/
/*	Copyright 2004 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "langmod.h"
#include "lbuff.h"
#include "parms.h"
#include "cmds.h"
#include "bufmod.h"



/********************************************************/
/*							*/
/*	Internal Routines				*/
/*							*/
/********************************************************/

static		LXmlnext();
static		LXmllast();
static		LXmlgetin();
static		LXmlfix1();
static		LXmlparse();



/********************************************************/
/*							*/
/*	Storage 					*/
/*							*/
/********************************************************/

static INT oldindent, oldcurindent;



/********************************************************/
/*							*/
/*	LXMinit -- initialize for lisp language 	*/
/*							*/
/********************************************************/


LXMinit()
{
   if (*PSVAL(L1) == 0)
      PSET(L1,"$");
   if (*PSVAL(L2) == 0)
      PSET(L2,"^$");
   if (*PSVAL(L3) == 0)
      PSET(L3,"^<");

   oldindent = PVAL(indent);
   oldcurindent = PVAL(curindent);

   PSET(indent,TRUE);
   PSET(curindent,TRUE);
}




/********************************************************/
/*							*/
/*	LXMfini -- finish current language		*/
/*							*/
/********************************************************/


LXMfini()
{
   if (strcmp(PSVAL(L1),"$") == 0) PSET(L1,"");
   if (strcmp(PSVAL(L2),"^$") == 0) PSET(L2,"");
   if (strcmp(PSVAL(L3),"^<") == 0) PSET(L3,"");

   PSET(indent,oldindent);
   PSET(curindent,oldcurindent);
}




/********************************************************/
/*							*/
/*	LXMcmds -- language specific commands		*/
/*							*/
/********************************************************/


LXMcmds(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT cnt,buf;

   cnt = (INT) val;
   if (cnt == 0) cnt = 1;

   switch(cmd) {
      case CM_1LANG : buf = BF_K; break;
      case CM_2LANG : buf = BF_L; break;
      case CM_3LANG : buf = BF_M; break;
      case CM_4LANG : buf = BF_N; break;
      case CM_5LANG : buf = BF_O; break;
      case CM_6LANG : buf = BF_P; break;
      case CM_7LANG : buf = BF_Q; break;
      case CM_8LANG : buf = BF_R; break;
      case CM_9LANG : buf = BF_S; break;
      case CM_0LANG : buf = BF_T; break;
    }

   DOstart(buf,cnt);

   return TRUE;
}




/********************************************************/
/*							*/
/*	LXMwtab -- tab +/- one xml unit 		*/
/*							*/
/********************************************************/


LXMwtab(dir,chr,line)
   INT dir;
   INT *chr;		       /* resultant position   */
   DATA *line;
{
   INT x;
   DATA y;

   x = PVAL(curchar);
   y = PDVAL(curline);

   if (dir > 0 && y >= PDVAL(filesize)) return FALSE;
   if (dir < 0 && y == 0 && x == 0) return FALSE;

   if (dir > 0) {
      LXmlnext(&x, &y);
    }
   else {
      LXmllast(&x, &y);
    }

   *chr = x;
   *line = y;

   return TRUE;
}




/********************************************************/
/*							*/
/*	LXMindent -- compute lisp automatic indentation */
/*							*/
/********************************************************/


LXMindent()
{
   register  INT x;

   x = LXmlgetin();

   return(x);
}




/********************************************************/
/*							*/
/*	Internal definitions				*/
/*							*/
/********************************************************/


#define XEOL	     0			/*  end of line */
#define XOPEN	     1			/* <		*/
#define XCLOSE	     2			/* >		*/
#define XFINISH      3			/* />		*/
#define XOPENEND     4			/* </		*/
#define XOPENCMMT    5			/* <!-- 	*/
#define XENDCMMT     6			/* -->		*/
#define XOPENCMD     7			/* <?		*/
#define XCLOSECMD    8			/* ?>		*/
#define XOPENDTD     9			/* <!		*/
#define XID	     10 		/* identifier	*/
#define XSTRING      11 		/* string	*/
#define XTEXT	     12 		/* user text	*/
#define XOPENCDATA   13 		/*		*/
#define XCLOSECDATA  14 		/*		*/


#define XMODESTART   100		/* outside xml	*/
#define XINELEMENT   101		/* inside element */
#define XINCMMT      102		/* inside cmmt	*/
#define XINCMD	     103		/* inside cmd	*/
#define XINDTD	     104		/* inside dtd	*/
#define XINSTRING    105		/* inside string */
#define XINTEXT      106		/* inside text */
#define XINCDATA     107		/* inside cdata */


#define MAX_ATOM 10

struct XMLTOK {
   int Xtype;
   int Xstart;
   int Xend;
   int Xmode;
};




/********************************************************/
/*							*/
/*	LXmlnext -- find start of next xml item 	*/
/*							*/
/********************************************************/


static
LXmlnext(chr,line)
   INT *chr;
   DATA *line;
{
   INT x = *chr;
   DATA y = *line;
   register  INT scnt,pcnt,idx,fnfg;
   struct XMLTOK tbl[256];

   fnfg = FALSE;
   LNget(y);
   LXmlparse(LBlinebuf,tbl);

   if (tbl[0].Lstart > x)
      idx = -1;
   else {
      for (idx = 0; tbl[idx].Ltype != LEOL && tbl[idx+1].Lstart <= x; ++idx)
	 ;
      if (tbl[idx].Ltype == LEOL)
	 --idx;
    }

   scnt = pcnt = 0;

   if (idx >= 0 && tbl[idx].Lstart == x) {
      if (tbl[idx].Ltype == '(')
	 pcnt = 1;
      else if (tbl[idx].Ltype == '[')
	 scnt = 1;
    }
   if (pcnt == 1 && tbl[idx].Lstart == 0)
      fnfg = TRUE;

   ++idx;
   while (pcnt != 0 || scnt != 0 || tbl[idx].Ltype == LEOL) {
      switch (tbl[idx].Ltype) {
	 case '(':
	    if (scnt == 0)
	       ++pcnt;
	    break;
	 case '[':
	    ++scnt;
	    break;
	 case ')':
	    if (scnt == 0)
	       --pcnt;
	    break;
	 case ']':
	    if (scnt != 0)
	       --scnt;
	    else if (!fnfg) {
	       pcnt = 0;
	       --idx;
	     }
	    else
	       pcnt = 0;
	    break;
	 case LEOL:
	    if (++y >= PDVAL(filesize))
	       return;
	    LNget(y);
	    LXmlparse(LBlinebuf,tbl);
	    idx = -1;
	    break;
	 default:
	    break;
       }
      if (idx >= 0 && tbl[idx].Ltype != LEOL) {
	 *line = y;
	 *chr = tbl[idx].Lend+1;
       }
      ++idx;
    }
   *chr = tbl[idx].Lstart;
   *line = y;
   return;
}




/********************************************************/
/*							*/
/*	LXmllast -- find start of previous s-exp       */
/*							*/
/********************************************************/


static
LXmllast(chr,line)
   INT *chr;
   DATA *line;
{
   INT x = *chr;
   DATA y = *line;
   register  INT scnt,pcnt,idx;
   struct XMLTOK tbl[256];

   LNget(y);
   LXmlparse(LBlinebuf,tbl);

   for (idx = 0; tbl[idx].Lstart < x && tbl[idx].Ltype != LEOL; ++idx)
      ;

   scnt = pcnt = 0;

   do {
      while (--idx < 0) {
	 if (--y < 0) {
	    *chr = 0;
	    return;
	  }
	 *line = y;
	 LNget(y);
	 idx = LXmlparse(LBlinebuf,tbl);
       }

      if (idx == 0 && tbl[0].Lstart == 0 &&
	     tbl[0].Ltype != ']' &&
	     tbl[0].Ltype != ')')
	 break;

      switch (tbl[idx].Ltype) {
	 case ')':
	    if (scnt == 0)
	       --pcnt;
	    break;
	 case ']':
	    --scnt;
	    break;
	 case '[':
	    if (++scnt == 0)
	       pcnt = 0;
	    break;
	 case '(':
	    if (scnt == 0)
	       ++pcnt;
	    break;
	 default:
	    break;
       }
    } while (scnt < 0 || pcnt < 0);

      *chr = tbl[idx].Lstart;
      return;
}




/********************************************************/
/*							*/
/*	LXmlgetin -- find correct indentation	       */
/*							*/
/********************************************************/


static
LXmlgetin()
{
   register  DATA y;
   register  INT lcnt, scnt, pcnt, cnt, i;
   struct XMLTOK tbl[256];

   lcnt = scnt = pcnt = 0;

   y = PDVAL(curline);

   while (--y >= 0) {
      LNget(y);
      cnt = LXmlparse(LBlinebuf,tbl);
      if (cnt != 0) {
	 for (i = cnt-1; i >= 0; --i) {
	    switch (tbl[i].Ltype) {
	       case ']':
		  scnt--;
		  pcnt = 0;
		  break;
	       case ')':
		  if (scnt == 0)
		     pcnt--;
		  break;
	       case '[':
		  if (++scnt > 0)
		     return LXmlfix1(tbl,i,lcnt);
		  if (lcnt > 0 && scnt == 0)
		     return tbl[i].Lstart;
		  break;
	       case '(':
		  if (scnt == 0 && ++pcnt > 0)
		     return LXmlfix1(tbl,i,lcnt);
		  if (lcnt >0 && scnt == 0 && pcnt == 0)
		     return tbl[i].Lstart;
		  break;
	       default:
		  break;
	     }
	  }
	 if (lcnt++ == 0 && scnt == 0 && pcnt == 0)
	    return tbl[0].Lstart;
	 if (tbl[0].Lstart == 0 && tbl[0].Ltype != ')')
	    break;
       }
    }
   if (pcnt < 0)
      DSerror("Unbalanced right parenthesis");

   return 0;
}




/********************************************************/
/*							*/
/*	LXmlfix1 -- find indentation for '(' and '['   */
/*							*/
/********************************************************/


static
LXmlfix1(tbl, idx, lcnt)
   struct XMLTOK tbl[];
   int idx, lcnt;
{
   register  int len;

   if (tbl[idx+1].Ltype == LEOL)
      return tbl[idx].Lstart+1;
   if (tbl[idx+1].Ltype == LATOM) {
      len = tbl[idx+1].Lend - tbl[idx+1].Lstart + 1;
      if (((PVAL(curwidth)-tbl[idx].Lstart)/MAX_ATOM) >= len &&
	     tbl[idx+2].Ltype != LEOL)
	 return tbl[idx+2].Lstart;
      return tbl[idx].Lstart+2;
    }
   return tbl[idx+1].Lstart;
}




/********************************************************/
/*							*/
/*	LXmlparse -- parse a xml line into tokens	*/
/*							*/
/********************************************************/


static
LXmlparse(char * pt, struct XMLTOK [] tbl)
{
   int i, tbc;
   char ch;

   i = -1;
   tbc = -1;
   while ((ch = pt[++i]) != 0) {
      if (ch != ' ') {
	 ++tbc;
	 tbl[tbc].Lstart = i;
	 if (strchr("=",ch) != NULL) {
	    tbl[tbc].Ltype = ch;
	  }
	 else if (ch == '<') {
	    if (pt[i+1] == '?') {
	       tbl[tbc].Ltype = STARTQ;
	       ++i;
	     }
	    else if (pt[i+1] == '/') {
	       tbl[tbc].Ltype = STARTE;
	       ++i;
	     }
	    else if (pt[i+1] == '!') {
	       if (pt[i+2] == '-' && pt[i+3] == '-') {
		  tbl[tbc].Ltype = STARTC;
		  i += 3;
		}
	       else if (strncmp(&pt[i],"<![CDATA[",9) == 0) {
		  i += 9;
		  tbl[tbc].Ltype = STARTQ;
		}
	       else {
		  ++i;
		  tbl[tbc].Ltype = STARTA;
		}
	     }
	    else tbl[tbc].Ltype = STARTX;
	  }
	 else if (ch == '>') {
	    tbl[tbc].Ltype = ENDX;
	  }
	 else if (ch == '?' && pt[i+1] == '>') {
	    i++;
	    tbl[tbc].Ltype = ENDQ;
	  }
	 else if (ch == '-' && pt[i+1] == '-' && pt[i+2] == '!' && pt[i+3] == '>') {
	    i += 3;
	    tbl[tbc].Ltype = ENDC;
	  }
	 else if (ch == '!' && pt[i+1] == '>') {
	    i++;
	    tbl[tbc].Ltype = ENDA;
	  }
	 else if (ch == ']' && pt[i+1] == ']' && pt[i+2] == '>') {
	    i += 2;
	    tbl[tbc].Ltype = ENDQ;
	  }
	 else if (ch == '/' && pt[i+1] == '>') {
	    i++;
	    tbl[tbc].Ltype = ENDE;
	  }
	 else {
	    tbl[tbc].Ltype = LATOM;
	    if (ch == '"' || ch == '\'') {
	       int qch = ch;
	       while (((ch = pt[++i]) != qch && ch != 0);
	       if (ch == 0) {
		  DSerror("Unmatched quote");
		  --i;
		}
	       tbl[tbc].Ltype = LSTRING;
	     }
	    else {
	       --i;
	       while ((ch = pt[++i]) != 0 && strchr("([]) ;",ch) == NULL)
		  if (ch == '!' && pt[++i] == 0) --i;
	       --i;
	     }
	  }
	 tbl[tbc].Lend = i;
       }
    }
   ++tbc;
   tbl[tbc].Ltype = LEOL;
   tbl[tbc].Lstart = tbl[tbc].Lend = i;

   return tbc;
}


/* end of lisplang.c */
