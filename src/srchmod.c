/********************************************************/
/*							*/
/*		SRCHMOD.C				*/
/*							*/
/*	This module contains the entry to search for	*/
/*	a given string or regular expression in the	*/
/*	current file.					*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "parms.h"
#include "lbuff.h"
#include "linemod.h"



/********************************************************/
/*							*/
/*	Definitions for regular expressions		*/
/*							*/
/********************************************************/

#define CBRA	1
#define CCHR	2
#define CDOT	4
#define CCL	6
#define NCCL	8
#define CDOL	10
#define CEOF	11
#define CKET	12
#define CBACK	18

#define CSTAR	01

#define LBSIZE	BUFSIZ
#define ESIZE	256
#define NBRA	9

static	CHAR	expbuf[ESIZE];
static	INT	circf;
static	CHAR	*braslist[NBRA];
static	CHAR	*braelist[NBRA];
static	unsigned char bittab[] = {
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128
};





/********************************************************/
/*							*/
/*	Forward and external routines			*/
/*							*/
/********************************************************/


extern	STRING	match();	/* fast string match	*/
static	STRING	rematch();	/* reg expr match	*/
static	int	recompile();
static	int	advance();



/********************************************************/
/*							*/
/*	SEarch -- main entry for searching		*/
/*							*/
/********************************************************/


int SEarch(pat,dir,refg,line,chr)
   STRING pat;			/* pattern		*/
   INT dir;			/* direction (+n/-n)	*/
   BOOL refg;			/* reg expr if true	*/
   LINE *line;			/* result line		*/
   INT *chr;			/* result char		*/
{
   register LINE y;
   register INT x;
   register INT a,b;
   register STRING pt,rpt;

   x = PVAL(curchar);		/* current position	*/
   y = PDVAL(curline);
   if (y >= PDVAL(filesize) && dir < 0) y = PDVAL(filesize)-1;

   if (refg)
    { circf = 0;
      if (!recompile(pat)) return FALSE;
    };

   PSET(srching,TRUE);

   while (y >= 0 && y < PVAL(filesize) && PVAL(srching))
    { LNget(y);
      if (x > 0 && x > (int) strlen((char *) LBlinebuf)) x = strlen((char *) LBlinebuf);
      a = -1;
      if (dir < 0)
       { b = 0;
	 do
	  { pt = (STRING) &LBlinebuf[b];
	    if (b > 0 && LBlinebuf[b-1] == 0) rpt = NULL;
	       else if (refg) rpt = rematch(pt);
	       else rpt = match(pt,pat);
	    if (rpt != NULL && x != -1 && rpt >= (STRING)(&LBlinebuf[x]))
	       rpt = NULL;
	    if (rpt != NULL)
	     { a = (rpt-((STRING)(LBlinebuf)));
	       if (circf) rpt = NULL; else b = a+1;
	     };
	  }
	 while (rpt != NULL);
       }
      else
       { pt = (STRING) &LBlinebuf[x+1];
	 if (x >= 0 && LBlinebuf[x] == 0) rpt = NULL;
	    else if (refg && circf && x >= 0) rpt = NULL;
	    else if (refg) rpt = rematch(pt);
	    else rpt = match(pt,pat);
	 if (rpt != NULL) a = rpt-(STRING)LBlinebuf;
       };
      if (a >= 0)
       { if (dir > 0) --dir; else ++dir;
	 if (dir == 0)
	  { *line = y;
	    *chr = a;
	    PSET(srching,FALSE);
	    return TRUE;
	  };
	 x = a;
       }
      else x = -1;
      if (dir > 0) y++; else y--;
    };

   PSET(srching,FALSE);

   return FALSE;
}







/********************************************************/
/*							*/
/*	recompile -- compile regular expression 	*/
/*							*/
/********************************************************/


static int
recompile(astr)
char *astr;
{
	register int c;
	register char *ep, *sp;
	char *cstart;
	char *lastep;
	int cclcnt;
	char bracket[NBRA], *bracketp;
	int closed;
	char numbra;
	char neg;

	ep = expbuf;
	for (c = 0; c < ESIZE; ++c) expbuf[c] = 0;
	sp = astr;
	lastep = 0;
	bracketp = bracket;
	closed = numbra = 0;
	if (*sp == '^') {
		circf++;
		sp++;
	}
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		if ((c = *sp++) != '*')
			lastep = ep;
		switch (c) {

		case '\0':
			*ep++ = CEOF;
			return TRUE;

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep==0 || *lastep==CBRA || *lastep==CKET)
				goto defchar;
			*lastep |= CSTAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			if(&ep[17] >= &expbuf[ESIZE])
				goto cerror;
			*ep++ = CCL;
			neg = 0;
			if((c = *sp++) == '^') {
				neg = 1;
				c = *sp++;
			}
			cstart = sp;
			do {
				if (c=='\0')
					goto cerror;
				if (c=='-' && sp>cstart && *sp!=']') {
					for (c = sp[-2]; c<*sp; c++)
						ep[c>>3] |= bittab[c&07];
					sp++;
				}
				ep[c>>3] |= bittab[c&07];
			} while((c = *sp++) != ']');
			if(neg) {
				for(cclcnt = 0; cclcnt < 16; cclcnt++)
					ep[cclcnt] ^= -1;
				ep[0] &= 0376;
			}

			ep += 16;

			continue;

		case '\\':
			if((c = *sp++) == '(') {
				if(numbra >= NBRA) {
					goto cerror;
				}
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if(c == ')') {
				if(bracketp <= bracket) {
					goto cerror;
				}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;
			}

			if(c >= '1' && c <= '9') {
				if((c -= '1') >= closed)
					goto cerror;
				*ep++ = CBACK;
				*ep++ = c;
				continue;
			}

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
    cerror:
	DSerror("Error in regular expression");

	return FALSE;
}

/********************************************************/
/*							*/
/*	rematch -- test for regular expression match	*/
/*							*/
/********************************************************/


static STRING
rematch(p1)
register STRING p1; {
	register STRING p2;
	register int c;
		p2 = expbuf;
		if (circf) {
			if (advance(p1, p2))
				goto found;
			goto nfound;
		}
		/* fast check for first character */
		if (*p2==CCHR) {
			c = p2[1];
			do {
				if (*p1!=c)
					continue;
				if (advance(p1, p2))
					goto found;
			} while (*p1++);
			goto nfound;
		}
		/* regular algorithm */
		do {
			if (advance(p1, p2))
				goto found;
		} while (*p1++);
	nfound:
		return(0);
	found:
		return(p1);
}





/********************************************************/
/*							*/
/*	advance -- match next elt of reg expr		*/
/*							*/
/********************************************************/


static int
advance(lp, ep)
register char *lp, *ep;
{
	register char *curlp;
	char c;
	char *bbeg;
	int ct;

	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp=='\0')
			continue;
		return(0);

	case CEOF:
		return(1);

	case CCL:
		c = *lp++ & 0177;
		if(ep[c>>3] & bittab[c & 07]) {
			ep += 16;
			continue;
		}
		return(0);
	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CBACK:
		bbeg = braslist[*ep];
		if (braelist[*ep]==0)
			return(0);
		ct = braelist[*ep++] - bbeg;
		if(strncmp(bbeg, lp, ct)) {
			lp += ct;
			continue;
		}
		return(0);

	case CBACK|CSTAR:
		bbeg = braslist[*ep];
		if (braelist[*ep]==0)
			return(0);
		ct = braelist[*ep++] - bbeg;
		curlp = lp;
		while(strncmp(bbeg, lp, ct))
			lp += ct;
		while(lp >= curlp) {
			if(advance(lp, ep))	return(1);
			lp -= ct;
		}
		return(0);


	case CDOT|CSTAR:
		curlp = lp;
		while (*lp++);
		goto star;

	case CCHR|CSTAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;

	case CCL|CSTAR:
		curlp = lp;
		do {
			c = *lp++ & 0177;
		} while(ep[c>>3] & bittab[c & 07]);
		ep += 16;
		goto star;

	star:
		if(--lp == curlp) {
			continue;
		}

		if(*ep == CCHR) {
			c = ep[1];
			do {
				if(*lp != c)
					continue;
				if(advance(lp, ep))
					return(1);
			} while(lp-- > curlp);
			return(0);
		}

		do {
			if (advance(lp, ep))
				return(1);
		} while (lp-- > curlp);
		return(0);

	default:
		ABORT;
	}
}





/* end of srchmod.c */
