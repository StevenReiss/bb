/********************************************************/
/*							*/
/*		LISPLANG.C				*/
/*							*/
/*	This module contains the language		*/
/*	specific routines for lisp			*/
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
/*	Internal Routines				*/
/*							*/
/********************************************************/

static	void	LLispnext();
static	void	LLisplast();
static	int	LLispgetin();
static	int	LLispfix1();
static	int	LLispparse();




/********************************************************/
/*							*/
/*	Storage 					*/
/*							*/
/********************************************************/

static INT oldindent, oldcurindent;



/********************************************************/
/*							*/
/*	LLIinit -- initialize for lisp language 	*/
/*							*/
/********************************************************/


void LLIinit()
{
   if (*PSVAL(L1) == 0)
      PSET(L1,"$");
   if (*PSVAL(L2) == 0)
      PSET(L2,"^$");
   if (*PSVAL(L3) == 0)
      PSET(L3,"^[([]d[efm]");

   oldindent = PVAL(indent);
   oldcurindent = PVAL(curindent);

   PSET(indent,TRUE);
   PSET(curindent,TRUE);
}




/********************************************************/
/*							*/
/*	LLIfini -- finish current language		*/
/*							*/
/********************************************************/


void LLIfini()
{
   if (strcmp(PSVAL(L1),"$") == 0) PSET(L1,"");
   if (strcmp(PSVAL(L2),"^$") == 0) PSET(L2,"");
   if (strcmp(PSVAL(L3),"^[([]d[efm]") == 0) PSET(L3,"");

   PSET(indent,(PTR) oldindent);
   PSET(curindent,(PTR) oldcurindent);
}




/********************************************************/
/*							*/
/*	LLIcmds -- language specific commands		*/
/*							*/
/********************************************************/


int LLIcmds(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT cnt,buf;

   cnt = (int)(long) val;
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
    }

   DOstart(buf,cnt);

   return TRUE;
}




/********************************************************/
/*							*/
/*	LLIwtab -- tab +/- one s-expression		*/
/*							*/
/********************************************************/


int LLIwtab(dir,chr,line)
    INT dir;
    INT *chr;			/* resultant position	*/
    DATA *line;
{
    INT x;
    DATA y;

    x = PVAL(curchar);
    y = PDVAL(curline);

    if (dir > 0 && y >= PDVAL(filesize)) return FALSE;
    if (dir < 0 && y == 0 && x == 0) return FALSE;

    if (dir > 0) {
	LLispnext(&x, &y);
    }
    else {
	LLisplast(&x, &y);
    }

    *chr = x;
    *line = y;

    return TRUE;
}




/********************************************************/
/*							*/
/*	LLIindent -- compute lisp automatic indentation */
/*							*/
/********************************************************/


int LLIindent()
{
   register  INT x;

    x = LLispgetin();
    return(x);
}




/********************************************************/
/*							*/
/*	Internal definitions				*/
/*							*/
/********************************************************/


#define LEOL	0
#define LATOM	1

#define MAX_ATOM 10

struct LISPTOK {
    int Ltype,
	Lstart,
	Lend;
};




/********************************************************/
/*							*/
/*	LLispnext -- find start of next s-exp		*/
/*							*/
/********************************************************/


static void
LLispnext(chr,line)
    INT *chr;
    DATA *line;
{
    INT x = *chr;
    DATA y = *line;
    register  INT scnt,pcnt,idx,fnfg;
    struct LISPTOK tbl[256];

    fnfg = FALSE;
    LNget(y);
    LLispparse(LBlinebuf,tbl);

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
		LLispparse(LBlinebuf,tbl);
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
/*	LLisplast -- find start of previous s-exp	*/
/*							*/
/********************************************************/


static void
LLisplast(chr,line)
    INT *chr;
    DATA *line;
{
    INT x = *chr;
    DATA y = *line;
    register  INT scnt,pcnt,idx;
    struct LISPTOK tbl[256];

    LNget(y);
    LLispparse(LBlinebuf,tbl);

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
	    idx = LLispparse(LBlinebuf,tbl);
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
/*	LLispgetin -- find correct indentation		*/
/*							*/
/********************************************************/


static int
LLispgetin()
{
    register  DATA y;
    register  INT lcnt, scnt, pcnt, cnt, i;
    struct LISPTOK tbl[256];

    lcnt = scnt = pcnt = 0;

    y = PDVAL(curline);

    while (--y >= 0) {
	LNget(y);
	cnt = LLispparse(LBlinebuf,tbl);
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
			    return LLispfix1(tbl,i,lcnt);
			if (lcnt > 0 && scnt == 0)
			    return tbl[i].Lstart;
			break;
		    case '(':
			if (scnt == 0 && ++pcnt > 0)
			    return LLispfix1(tbl,i,lcnt);
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
/*	LLispfix1 -- find indentation for '(' and '['   */
/*							*/
/********************************************************/


static int
LLispfix1(tbl, idx, lcnt)
    struct LISPTOK tbl[];
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
/*	LLispparse -- parse a lisp line into tokens	*/
/*							*/
/********************************************************/


static int
LLispparse(pt, tbl)
    char pt[];
    struct LISPTOK tbl[];
{
    register  int i, tbc;
    register  char ch;

    i = -1;
    tbc = -1;
    while ((ch = pt[++i]) != 0 && ch != ';') {
	if (ch != ' ') {
	    ++tbc;
	    tbl[tbc].Lstart = i;
	    if (strchr("([])",ch) != NULL) {
		tbl[tbc].Ltype = ch;
		tbl[tbc].Lend = i;
	    }
	    else {
		tbl[tbc].Ltype = LATOM;
		if (ch == '\'')
		    ch = pt[++i];
		if (ch == '|') {
		    while (((ch = pt[++i]) != '|' || pt[i-1] == '\\')
				    && ch != 0);
		    if (ch == 0) {
			DSerror("Unmatched '|' delimiter");
			--i;
		    }
		}
		if (ch == '"') {
		    while (((ch = pt[++i]) != '"' || pt[i-1] == '\\')
				    && ch != 0);
		    if (ch == 0) {
			DSerror("Unmatched double quote");
			--i;
		    }
		}
		else {
		    --i;
		    while ((ch = pt[++i]) != 0 && strchr("([]) ;",ch) == NULL)
			if (ch == '!' && pt[++i] == 0) --i;
		    --i;
		}
		tbl[tbc].Lend = i;
	    }
	}
    }
    ++tbc;
    tbl[tbc].Ltype = LEOL;
    tbl[tbc].Lstart = tbl[tbc].Lend = i;

    return tbc;
}


/* end of lisplang.c */
