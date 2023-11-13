/********************************************************/
/*							*/
/*		LBUFF.C 				*/
/*							*/
/*   Ths module contains the routines for moving data	*/
/*   between various portions of the editor.  All data	*/
/*   is moved via the line buffer, using a source rout- */
/*   ine to put it there and a target routine to get it */
/*   out.  The data can be formatted in any number of	*/
/*   ways inbetween.  The main routine for this purpose */
/*   is LBmove. 					*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "lbuff.h"
#include "parms.h"
#include "langmod.h"



/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


	TEXT	LBlinebuf[LINESIZE];	/* line buffer	*/

static	FCT	getfct; 	/* source for move	*/
static	DATA	getcnt; 	/* source count 	*/
static	DATA	getparm;	/* source parameter	*/
static	FCT	putfct; 	/* target for move	*/
static	DATA	putcnt; 	/* target count 	*/
static	DATA	putparm;	/* target parameter	*/

static	LBFLAGS flags;		/* flags for cur move	*/

static	INT	done;		/* done current move fg */
static	INT	ocount; 	/* actual output count	*/
static	INT	icount; 	/* actual input count	*/

#define MAXEDITS  5		/* max # concurrent edits */

typedef LBSTATUS (*LBFCT)();

static	LBFCT	edfuns[MAXEDITS]; /* editing routines	*/
static	INT	numedits;






/********************************************************/
/*							*/
/*	Forward routine definitions			*/
/*							*/
/********************************************************/


static	LBSTATUS LBtabin();	/* elim tabs on input	*/
static	LBSTATUS LBtabout();	/* insert output tabs	*/
static	LBSTATUS LBeolspace();	/* remove spaces as eol */
static	LBSTATUS LBcrlf();	/* handle cr-lf at eol	*/
static	LBSTATUS LBsplit();	/* split lines		*/
static	int	 LBget();
static	void	 LBput();




/********************************************************/
/*							*/
/*	LBmove -- move data around editor		*/
/*							*/
/********************************************************/


int LBmove(src,srcp,srcl,dst,dstp,dstl,fgs)
   FCT	 src;			/* source location	*/
   DATA  srcp;			/* source parameter	*/
   DATA  srcl;			/* source max count	*/
   FCT	 dst;			/* target location	*/
   DATA  dstp;			/* target parameter	*/
   DATA  dstl;			/* target max count	*/
   LBFLAGS fgs; 		/* move flags		*/
{
   register INT i;
   LBSTATUS sts[MAXEDITS+1];

   flags = fgs; 	/* set global move variables	*/
   LBlinebuf[0] = 0;
   getfct = src; getcnt = srcl; getparm = srcp;
   putfct = dst; putcnt = dstl; putparm = dstp;
   done = FALSE;
   ocount = 0;

   numedits = 0;
   if ((flags & LBF_INTAB) && !PVAL(curtabin))
      edfuns[numedits++] = LBtabin;
   if ((flags & LBF_OUTAB) && PVAL(curtabout))
      edfuns[numedits++] = LBtabout;
   if ((flags & LBF_OUTAB) && !PVAL(curtabout) && !PVAL(eolspace))
      edfuns[numedits++] = LBeolspace;
   if ((flags & LBF_CRLF) && !PVAL(curcrlf))
      edfuns[numedits++] = LBcrlf;
   if ((flags & LBF_STREAM) && PVAL(curstream))
      edfuns[numedits++] = LBsplit;
   if ((flags & LBF_FMT0) && LAformats[0] != NULL)
      edfuns[numedits++] = (LBFCT) LAformats[0];
   if ((flags & LBF_FMT1) && LAformats[1] != NULL)
      edfuns[numedits++] = (LBFCT) LAformats[1];
   if ((flags & LBF_FMT2) && LAformats[2] != NULL)
      edfuns[numedits++] = (LBFCT) LAformats[2];
   if ((flags & LBF_FMT3) && LAformats[3] != NULL)
      edfuns[numedits++] = (LBFCT) LAformats[3];
   if ((flags & LBF_FMT4) && LAformats[4] != NULL)
      edfuns[numedits++] = (LBFCT) LAformats[4];

   do
    { sts[0] = LBget() ? LSTS_CONTINUE : LSTS_DONE;
      for (i = 0; i >= 0; )
       { if (i >= numedits)
	  { if (sts[i] != LSTS_DONE) LBput(); }
	 else
	  { sts[i+1] = (*(edfuns[i]))(sts[i],putparm);
	    if (sts[i+1] != LSTS_PASS)
	     { ++i; continue; };
	  };

	 while (--i >= 0 && sts[i+1] != LSTS_CONTINUE);

	 if (i >= 0 && sts[i] != LSTS_DONE) sts[i] = LSTS_NEXT;
       };
    } while (!done);

   return ocount;
}





/********************************************************/
/*							*/
/*	LBget -- get source line			*/
/*							*/
/********************************************************/


static int
LBget()
{
   register INT sts;

   if ((flags & LBF_SMAX) && getcnt == 0) ABORT;	/* source limit */

   if ((flags & LBF_SFRST)) sts = (*getfct)(getparm,icount);
      else if ((flags & LBF_SPARM)) sts = (*getfct)(getparm);
      else sts = (*getfct)();

   if (!sts && !(flags&LBF_SIGN))
    { done = TRUE;
      return FALSE;
    };

   ++icount;
   if ((flags & LBF_SINCP)) ++getparm;
   if ((flags & LBF_SMAX) && getcnt > 0 && --getcnt == 0) done = TRUE;

   return TRUE;
}





/********************************************************/
/*							*/
/*	LBput -- place data line into proper target	*/
/*							*/
/********************************************************/


static void
LBput()
{
   if ((flags & LBF_DMAX) && putcnt == 0) ABORT;	/* target cnt limit */

   if ((flags & LBF_DFRST)) (*putfct)(putparm,ocount);
      else if ((flags & LBF_DPARM)) (*putfct)(putparm);
      else (*putfct)();

   ++ocount;
   if ((flags & LBF_DINCP)) ++putparm;

   if ((flags & LBF_DMAX) && putcnt > 0 && --putcnt == 0) done = TRUE;
}






/********************************************************/
/*							*/
/*	LBtabin -- remove tabs on input 		*/
/*							*/
/********************************************************/


static LBSTATUS
LBtabin(sts,pm)
   LBSTATUS sts;
   DATA pm;
{
   TEXT nline[LINESIZE];
   register INT ct;
   register TEXT *pta,*ptb,ch,qfg;

   if (sts == LSTS_DONE) return LSTS_DONE;

   if (strchr((char *) LBlinebuf,'\t') == NULL) return LSTS_LAST;

   pta = LBlinebuf;
   ptb = nline;
   ct = 0;
   qfg = '\0';

   while ((ch = *pta++) != 0 && ct < LINESIZE-1) {
      if (ch != '\t' || qfg != '\0') {
	 *ptb++ = ch;
	 ++ct;
	 /* See if quoting should be turned on */
	 if (qfg == '\0') {
	    /* If ch is first char or previous char is not a backslash */
	    if (((pta == &LBlinebuf[1]) || pta[-2] != '\\') &&
		   (ch == '"' || ch == '\'' || ch == '`')) qfg = ch;
	  }
	 /* See if it should be turned off */
	 else if (pta[-2] != '\\' && ch == qfg) qfg = '\0';

	 /* This should last until end of line */
	 if (isdigit(ch) && (*pta == 'h' || *pta == 'H')) qfg = '\n';
       }
      else {
	 do { *ptb++ = ' '; ++ct; }
	 while ((ct&07) != 0);
       };
    };

   if (ct >= LINESIZE-1)
      DSerror("Line too long with tabs -- truncated");

   *ptb = 0;

   strcpy((char *) LBlinebuf,(char *) nline);

   return LSTS_LAST;
}





/********************************************************/
/*							*/
/*	LBtabout -- eliminate spaces on output		*/
/*							*/
/********************************************************/


static LBSTATUS
LBtabout(sts,pm)
   LBSTATUS sts;
   DATA pm;
{
   register TEXT *pta,*ptb,ch;
   register INT spct,ct,qfg;

   if (sts == LSTS_DONE) return LSTS_DONE;

   pta = ptb = LBlinebuf;
   ct = spct = 0;
   qfg = FALSE;

   while ((ch = *pta++) != 0)
    { if ((ct&07) == 0 && spct > 0 && !qfg)
       { if (spct == 1) *ptb++ = ' ';
	    else *ptb++ = '\t';
	 spct = 0;
       };
      ++ct;
      if (ch == ' ') ++spct;
       else
	 { while (spct-- > 0) *ptb++ = ' ';
	   spct = 0;
	   if (ch == '"' || ch == '\'' || ch == '`') qfg |= 1;
	   if (isdigit(ch) && (*pta == 'h' || *pta == 'H')) qfg |= 1;
	   *ptb++ = ch;
	 };
    };

   if (spct > 0 && PVAL(eolspace))
      while(spct-- > 0) *ptb++ = ' ';

   *ptb = 0;

   return LSTS_LAST;
}





/********************************************************/
/*							*/
/*	LBeolspace -- eliminate spaces at eol		*/
/*							*/
/********************************************************/


static LBSTATUS
LBeolspace(sts,pm)
   LBSTATUS sts;
   DATA pm;
{
   register INT ct;

   if (sts == LSTS_DONE) return LSTS_DONE;

   ct = strlen((char *) LBlinebuf);
   while (ct > 0 && LBlinebuf[ct-1] == ' ')
      LBlinebuf[--ct] = 0;

   return LSTS_LAST;
}






/********************************************************/
/*							*/
/*	LBcrlf -- eliminate cr at end of line		*/
/*							*/
/********************************************************/


static LBSTATUS
LBcrlf(sts,pm)
   LBSTATUS sts;
   DATA pm;
{
   register INT ct;

   if (sts == LSTS_DONE) return LSTS_DONE;

   ct = strlen((char *) LBlinebuf);
   if (ct > 0 && LBlinebuf[ct-1] == '\r')
      LBlinebuf[--ct] = 0;

   return LSTS_LAST;
}






/********************************************************/
/*							*/
/*	LBsplit -- split line for stream mode		*/
/*							*/
/********************************************************/


static LBSTATUS
LBsplit(sts,line)
   LBSTATUS sts;
   DATA line;
{
   static TEXT spline[LINESIZE];
   register TEXT *tp,*ntp;
   register INT ln;

   if (sts == LSTS_DONE) return LSTS_DONE;

   if (sts != LSTS_NEXT) strcpy((char *) spline,(char *) LBlinebuf);

   if ((int) strlen((char *) spline) < PVAL(curwidth))
    { if (sts == LSTS_NEXT) strcpy((char *) LBlinebuf,(char *) spline);
      return LSTS_LAST;
    };

   tp = &spline[PVAL(curwidth)-1];
   while (tp >= spline && *tp != ' ') --tp;
   if (tp < spline) tp = &spline[PVAL(curwidth)-1];
   ntp = tp;
   while (ntp >= spline && *ntp == ' ') --ntp;
   ln = (++ntp)-spline;
   strncpy((char *) LBlinebuf,(char *) spline,ln);
   LBlinebuf[ln] = 0;
   our_strcpy((char *) spline,(char *) ++tp);

   if (PDVAL(curline) > line) PSET(curline,PDVAL(curline)+1);
    else if (PDVAL(curline) == line && PVAL(curchar) >= ln)
       { PSETI(curchar,PVAL(curchar)-(((PINT)tp)-((PINT)spline))+1);
	 PSET(curline,PDVAL(curline)+1);
       };

   if (!PVAL(curreadonly)) PSET(nochange,FALSE);

   return LSTS_CONTINUE;
}






/* end of lbuff.c */

