/********************************************************/
/*							*/
/*		LINEMOD.C				*/
/*							*/
/*	This module contains the code for maintaining	*/
/*	the current file as a list of lines.  It keeps	*/
/*	a table of line numbers.  For each line, it	*/
/*	saves the pointer for access into the temporary */
/*	file.						*/
/*							*/
/*	The line number table is segmented into rather	*/
/*	large blocks.  This allows for both dynamic	*/
/*	allocation and unlimited (virtually) growth.	*/
/*	There is a fixed table that shows where each	*/
/*	block begins.  Since these blocks are fixed	*/
/*	size, it is easy to actually find a line	*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "linemod.h"
#include "bufmod.h"
#include "tempmod.h"
#include "lbuff.h"
#include "parms.h"
#include "filemod.h"



/********************************************************/
/*							*/
/*	External routines				*/
/*							*/
/********************************************************/


extern	void	lmove();	/* move long data	*/
extern	void	llmove(unsigned long long *,unsigned long long *,long);       /* move long data       */




/********************************************************/
/*							*/
/*	Internal routines				*/
/*							*/
/********************************************************/


static	void	loadfile();
static	void	dumpfile();
static	void	linins();
static	void	lindel();
static	void	checkblock();




/********************************************************/
/*							*/
/*	Functions/ Parameter definitions		*/
/*							*/
/********************************************************/


#define LBLOCK(line) ((line)>>LNPBLOG)
				/* block # of line	*/
#define LOFFSET(line) ((line)&(LNPERLB-1))
				/* offset in block	*/
#define LBLKSTART(line) ((line)&~(LNPERLB-1))
				/* start of line's block*/
#define LBLKEND(line) ((line)|(LNPERLB-1))
				/* end of line's block  */


#define LTABLE(line) ((ADDR *)(LNlbuff[LBLOCK(line)]+LOFFSET(line)))
				/* address of data(line)*/


#define NMIBLKS 	1	/* # initial line blocks*/
#define NMMARKS 	32	/* # line marks allowed */

#define VOID		-1	/* void line number	*/





/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


	ADDR ** LNlbuff = NULL;   /* pointers to blocks containing */
				  /* the actual addresses in temp  */
				  /* file for each line.	   */
	INT	nmlblks = NMLBLKS;

typedef struct _LBUFF { 	/* line block descriptor*/
   ADDR LBFline[LNPERLB];
} *LBUFF,LBUFFB;

static LBUFFB initlb[NMIBLKS];	/* initial blocks	*/


static	LINE	lastline;	/* max line number	*/
static	BOOL	readyfg = FALSE;/* file is ready	*/
static	BOOL	loadingfg = FALSE; /* loading currently */

static	INT	mrkcnt; 	/* number marks current */
static	LINE	mrksav[NMMARKS];/* mark line buffer	*/
static	INT	mrkid[NMMARKS]; /* id of markers	*/
static	BYTE	mrkfil[NMMARKS];/* file id of markers	*/

extern	INT	TEMPdirtcnt;	/* # writes done counter*/





/********************************************************/
/*							*/
/*	LNinit -- initialize line record for file	*/
/*							*/
/********************************************************/


void LNinit()
{
   static BOOL firsttime = TRUE;
   register INT i,j;
   INT nbk = (PVAL(huge) ? NMLBLKS_HUGE : NMLBLKS);

   if (nbk > nmlblks && LNlbuff != NULL) {
      for (i = NMIBLKS; i < nmlblks; ++i) {
	 if (LNlbuff[i] == NULL) break;
	 else {
	    free(LNlbuff[i]);
	    LNlbuff[i] = NULL;
	  }
       }
      free(LNlbuff);
      LNlbuff = NULL;
    }

   if (LNlbuff == NULL) {
      nmlblks = nbk;
      LNlbuff = (ADDR **) malloc(sizeof(ADDR *) * nmlblks);
      for (i = 0; i < nmlblks; ++i) LNlbuff[i] = NULL;
    }
   else {
      for (i = NMIBLKS; i < nmlblks; ++i) {
	 if (LNlbuff[i] == NULL) break;
	 else {
	    free(LNlbuff[i]);
	    LNlbuff[i] = NULL;
	  }
       }
    }

   TMinit();

   for (i = 0; i < NMIBLKS; ++i)       /* initial blks */
    { LNlbuff[i] = (ADDR *) &initlb[i];
      for (j = 0; j < LNPERLB; ++j)
	 initlb[i].LBFline[j] = VOID;
    };

   lastline = 0;			/* empty	*/
   PSET(filesize,0);

   if (firsttime) {
      mrkcnt = 0;
      for (i = 0; i < NMMARKS; ++i) mrkid[i] = 0;
    };

   loadfile();				/* get input	*/

   readyfg = TRUE;
   firsttime = FALSE;
   TEMPdirtcnt = 0;
}





/********************************************************/
/*							*/
/*	LNclose -- close line areas			*/
/*							*/
/********************************************************/


int LNclose(savefg)
   INT savefg;
{
   if (!readyfg) return FALSE;

   if (savefg) dumpfile();

   TMclose();

   readyfg = FALSE;

   return TRUE;
}





/********************************************************/
/*							*/
/*	LNcheck -- check on user given position 	*/
/*							*/
/********************************************************/


LINE LNcheck(pos)
   LINE pos;
{
   if (pos < 0) pos = 0;
   if (pos > lastline) pos = lastline;

   return pos;
}





/********************************************************/
/*							*/
/*	LNforce -- ensure file goes to given position	*/
/*							*/
/********************************************************/


void LNforce(pos)
   LINE pos;
{
   if (pos < 0) ABORT;

   if (LNlbuff == NULL) return;

   while (pos > lastline) linins(lastline);
}






/********************************************************/
/*							*/
/*	LBUFF routines					*/
/*							*/
/*	These routines are designed to be called from	*/
/*	the lbuff module to move data to or from the	*/
/*	global line buffer and into the temporary file	*/
/*	Routines are supplied to fill the buffer, with	*/
/*	and without deleting the corresponding line;	*/
/*	to insert the buffer into the temp file, and	*/
/*	to replace a given line with the buffer.	*/
/*							*/
/********************************************************/


/********************************************************/
/*							*/
/*	LNget -- fill line buffer with given line	*/
/*							*/
/********************************************************/


INT LNget(ln)
   LINE ln;
{
   register ADDR addr;

   if (ln < 0) ABORT;

   if (ln >= lastline || (addr = *LTABLE(ln)) == 0)
      LBlinebuf[0] = 0; 	/* illegal/null -> null */
    else if (addr == VOID) LBlinebuf[0] = 0;
    else TMget(addr,LBlinebuf); /* else from temp file	*/

   return (ln < lastline);
}





/********************************************************/
/*							*/
/*	LNput -- replace current line			*/
/*							*/
/********************************************************/


int LNput(ln,insfg)
   LINE ln;
   INT insfg;
{
   if (insfg != 0) return LNputin(ln);

   if (ln >= lastline) HSline(HSL_PUT,lastline,VOID);
      else HSline(HSL_PUT,ln,*LTABLE(ln));

   if (ln < 0) ABORT;
   while (ln >= lastline) linins(lastline);

   MNreplace(ln,LBlinebuf);

   *LTABLE(ln) = TMput(LBlinebuf);

   return TRUE;
}





/********************************************************/
/*							*/
/*	LNputin -- insert given line at end of file	*/
/*							*/
/********************************************************/


int LNputin(ln)
   LINE ln;
{
   if (ln < 0) ABORT;

   HSline(HSL_INS,ln,(LINE)0);

   while (ln > lastline) linins(lastline);
   linins(ln);

   MNreplace(ln,LBlinebuf);

   *LTABLE(ln) = TMput(LBlinebuf);

   return TRUE;
}




/********************************************************/
/*							*/
/*	LNgetdel -- delete current line 		*/
/*							*/
/********************************************************/


INT LNgetdel(ln)
   LINE ln;
{
   register ADDR addr;

   if (ln < 0) ABORT;

   HSline(HSL_DEL,ln,*LTABLE(ln));

   if (ln >= lastline) LBlinebuf[0] = 0;
    else
     { if ((addr = *LTABLE(ln)) == 0) LBlinebuf[0] = 0;
	  else TMget(addr,LBlinebuf);
       lindel(ln);
     };

   return (ln < lastline);
}





/********************************************************/
/*							*/
/*	LNundo -- undo line changes			*/
/*							*/
/********************************************************/


void LNundo(typ,line,addr)
   HISTLINE typ;
   LINE line;
   ADDR addr;
{
   if (addr == 0xffffffff) addr = VOID;

   switch(typ)
    { case HSL_PUT	:
		if (addr == VOID)
		 { while (line < lastline) lindel(line);
		   break;
		 };
		*LTABLE(line) = addr;
		break;

      case HSL_INS	:
		lindel(line);
		break;

      case HSL_DEL	:
		linins(line);
		*LTABLE(line) = addr;
		break;
    };
}






/********************************************************/
/*							*/
/*	Markers 					*/
/*							*/
/*	The following routines supply a 'smart' marking */
/*	facility that takes modifications to the file	*/
/*	into account.  It allows the user to specify	*/
/*	up to 256 markers (actual limit is given in	*/
/*	this modules header).  Each mark has a line	*/
/*	number associated with it.  The editor attempts */
/*	to insure that the marker remains pointing to	*/
/*	that line regardless of what else happens to	*/
/*	the file, or if the user switches file.  The	*/
/*	code is not fail proof however, and will not	*/
/*	save the position under all circumstances.	*/
/*							*/
/********************************************************/


/********************************************************/
/*							*/
/*	LNmark -- save marker for current line		*/
/*							*/
/********************************************************/


void LNmark(id,ln)
   INT id;			/* marker id		*/
   LINE ln;			/* line number to store */
{
   register INT i;

   for (i = 0; i < mrkcnt; ++i) /* find place for id	*/
      if (mrkid[i] == id) break;
   if (i == mrkcnt) ++mrkcnt;

   HSlmark(mrkid[i],mrksav[i],mrkfil[i]);

   mrkid[i] = id;		/* save marker		*/
   mrksav[i] = ln;
   mrkfil[i] = PVAL(fileid);
}





/********************************************************/
/*							*/
/*	LNgetmark -- get marker value given id		*/
/*							*/
/********************************************************/


LINE
LNgetmark(id)
   INT id;			/* marker id to go to	*/
{
   register INT i;

   for (i = 0; i < mrkcnt; ++i) /* find marker in table */
      if (mrkid[i] == id) break;
   if (i == mrkcnt) return -1;
   if (mrkfil[i] != PVAL(fileid)) return -1;

   return mrksav[i];		/* return resultant line*/
}





/********************************************************/
/*							*/
/*	LNunmark -- unmark given id			*/
/*							*/
/********************************************************/


void LNunmark(id)
   INT id;			/* id of marker to del	*/
{
   register INT i;

   for (i = 0; i < mrkcnt; ++i) /* find id in table	*/
      if (mrkid[i] == id) break;
   if (i == mrkcnt) return;

   --mrkcnt;
   for ( ; i < mrkcnt; ++i)
    { mrkid[i] = mrkid[i+1];
      mrksav[i] = mrksav[i+1];
      mrkfil[i] = mrkfil[i+1];
    };
}





/********************************************************/
/*							*/
/*	loadfile, dumpfile -- handle i/o of file	*/
/*							*/
/********************************************************/


static void
loadfile()		 /* input source file	 */
{
   loadingfg = TRUE;

   LBmove(FLget,0l,0l,
	  LNputin,0l,0l,
	  LBF_DPARM|LBF_DINCP|LBF_INTAB|LBF_STREAM|LBF_CRLF|LBF_FMT1);

   loadingfg = FALSE;
}





static void
dumpfile()		 /* output source file	 */
{
   loadingfg = TRUE;

   LBmove(LNget,0l,0l,
	  FLput,0l,0l,
	  LBF_SPARM|LBF_SINCP|LBF_OUTAB|LBF_FMT2);

   loadingfg = FALSE;
}






/********************************************************/
/*							*/
/*	linins -- insert line in file			*/
/*							*/
/********************************************************/


static void
linins(lnum)
   LINE lnum;
{
   register LINE ptr,bptr,xxx;
   register INT i;

   if (LOFFSET(lastline) == 0)	/* start new block	*/
      checkblock(LBLOCK(lastline));

   ptr = lastline++;
   for ( ; ; )			/* move line list up	*/
    { bptr = LBLKSTART(ptr);
      if (bptr < lnum) bptr = lnum;
      if (bptr != ptr) {
	 xxx = (ptr - bptr);
	 llmove(LTABLE(bptr),LTABLE(bptr+1),xxx);
       }
      if (bptr == lnum) break;
      *LTABLE(bptr) = *LTABLE(bptr-1);
      ptr = bptr-1;
    };

   *LTABLE(lnum) = 0;		/* empty resultant line */

   MNinsert(lnum);

   if (!loadingfg)
      for (i = 0; i < mrkcnt; ++i)
	 if (mrksav[i] >= lnum && mrkfil[i] == PVAL(fileid)) ++mrksav[i];

   PSET(filesize,lastline);

   return;
}





/********************************************************/
/*							*/
/*	lindel -- delete line from file 		*/
/*							*/
/********************************************************/


static void
lindel(lnum)
   LINE lnum;			/* line to delete	*/
{
   register LINE bptr,ptr;
   register INT i;

   if (lnum >= lastline) return;

   MNdelete(lnum);

   ptr = lnum;
   for ( ; ; )
    { bptr = LBLKEND(ptr);
      if (bptr > lastline) bptr = lastline;
      if (bptr > ptr)
	 llmove(LTABLE(ptr+1),LTABLE(ptr),bptr-ptr);
      if (bptr == lastline) break;
      *LTABLE(bptr) = *LTABLE(bptr+1);
      ptr = bptr+1;
    };

   --lastline;
   *LTABLE(lastline) = 0;

   if (!loadingfg)
      for (i = 0; i < mrkcnt; ++i)
	 if (mrksav[i] > lnum && mrkfil[i] == PVAL(fileid)) --mrksav[i];

   PSET(filesize,lastline);

   return;
}





/********************************************************/
/*							*/
/*	checkblock -- prepare a new line block		*/
/*							*/
/********************************************************/


static void
checkblock(id)
   INT id;			/* block number 	*/
{
   register INT i;
   register LBUFF pt;

   if (LNlbuff == NULL) return;

   if (LNlbuff[id] == NULL)
    { pt = (LBUFF) malloc(LBLKSIZE);
      LNlbuff[id] = (ADDR *) pt;
      for (i = 0; i < LNPERLB; ++i)
	 pt->LBFline[i] = VOID;
    };

   return;
}





/* end of linemod.c */
