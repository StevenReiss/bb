/********************************************************/
/*							*/
/*		TEMP.C					*/
/*							*/
/*	This file contains the code for maintaining	*/
/*   the temporary file.  This file contains the	*/
/*   current version of the file being edited in a	*/
/*   somewhat abstract form (a list of line numbers	*/
/*   along with text pointers into the file), It is	*/
/*   designed to be periodically updated on disk so as	*/
/*   to facilitate crash recovery.			*/
/*							*/
/*	The basic ideas have been taken from Berkeley's */
/*   ex editor temporary file management routines.  The */
/*   code here is somewhat expanded since a screen	*/
/*   editor should keep the whole screen (and probably	*/
/*   two or three of them) in core at one time.  Thus	*/
/*   an arbitrary (parametrized) number of buffers are	*/
/*   provided for.					*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "tempmod.h"
#include "parms.h"
#include "linemod.h"
#include "bufmod.h"

#include <time.h>



/********************************************************/
/*							*/
/*	Function/parameter definitions			*/
/*							*/
/********************************************************/


#define THBLKS	((sizeof(HEADER)+TBUFSIZE-1)/TBUFSIZE)
					  /* # hdr blks */
#define FIRST	THBLKS			  /* first free */


			/* get addr of next blk start	*/
#define NEXTBLK(addr) ((addr+TBUFSIZE)&~(TBUFSIZE-1))

			/* get block number given addr	*/
#define BLOCKNO(addr) (addr >> TBUFSZLG)

			/* get offset given address	*/
#define OFFSET(addr)  (addr & (TBUFSIZE-1))

			/* get next block number	*/
#define NEXTBNO(addr) ((addr+TBUFSIZE-1)>>TBUFSZLG)






/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


struct _HEADER HEADER;		/* header in file	*/

				/* buffers for I/O	*/
static	TEXT	allbufs[(TIBUFS+1)*TBUFSIZE+IOPGSIZE-1];


	INT	TEMPdirtcnt = 0;/* # writes done counter*/
static	INT	hitcount = 0;	/* lru usage counter	*/

static	BLOCK	iblock[TIBUFS]; /* input block ids	*/
static	TEXT *	ibuff[TIBUFS];	/* input buffer locs	*/
static	INT	hitin[TIBUFS];	/* lru counters for bufs*/
static	BOOL	ichng[TIBUFS];	/* buf modified flags	*/

static	CHAR	tfname[FNSIZE]; /* temp file name	*/
static	INT	tfile;		/* temp file fid	*/
static	ADDR	tline;		/* next free temp block */
static	BOOL	havetmp = FALSE;/* temp in use flag	*/
static	INT	maxbno = 0;	/* max block number	*/

static	INT	nleft;		/* space left in buffer */






/********************************************************/
/*							*/
/*	Constants					*/
/*							*/
/********************************************************/


#define VOID	-1		/* void block pointer	*/

#define READ	FALSE		/* read operation	*/
#define WRITE	TRUE		/* write operation	*/

#define SAVEFREE 10		/* # free blocks for save */




/********************************************************/
/*							*/
/*	Forward/External function definitions		*/
/*							*/
/********************************************************/


#ifndef __STDC__
extern		read(); 	/* physical read	*/
extern		write();	/* physical write	*/
#endif

static	TEXT *	getblock();	/* get a temp file blk	*/
static	void	blkio();
static	void	savetemp();
static	void	tmpioerr();

extern	void	DSabort();



/********************************************************/
/*							*/
/*	TMinit -- initialize temporary file		*/
/*							*/
/********************************************************/


void TMinit()
{
   register INT i;
   register TEXT *bp;

   if (havetmp) TMclose();

				/* set up physical bufs */
   bp = (TEXT *)(((long)(allbufs+IOPGSIZE-1))&(~(IOPGSIZE-1)));
   for (i = 0; i < TIBUFS; ++i)
    { bp += TBUFSIZE;
      ibuff[i] = bp;
    };

   TEMPdirtcnt = 0;		/* set up buffers	*/
   hitcount = 0;
   for (i = 0; i < TIBUFS; ++i)
    { iblock[i] = VOID;
      ichng[i] = FALSE;
      hitin[i] = -1;
    };
   tline = FIRST*TBUFSIZE;
   maxbno = 0;

				/* open temp file	*/
   strcpy(tfname,PSVAL(tdirect));
   strcat(tfname,"/#bTMPXXXXXX");
   tfile = mkstemp(tfname);
   if (tfile < 0) tmpioerr();
   havetmp = TRUE;
   close(tfile);

   tfile = open(tfname,O_RDWR|O_LARGEFILE);
   if (tfile < 0) tmpioerr();

   for (i = 0; i < FIRST; ++i)
      blkio(i,ibuff[0],write);
   maxbno = FIRST-1;
}





/********************************************************/
/*							*/
/*	TMclose -- close temporary file 		*/
/*							*/
/********************************************************/


void TMclose()
{
   if (!havetmp) return;

   close(tfile);
   unlink(tfname);
   havetmp = FALSE;
}





/********************************************************/
/*							*/
/*	TMget -- get line given data address		*/
/*							*/
/********************************************************/


void TMget(addr,whr)
   ADDR addr;		/* data address (from put) of line */
   TEXT *whr;		/* location to put data 	   */
{
   register TEXT *bp,*lp;
   register INT nl;

   lp = whr;
   bp = getblock(addr,READ);
   nl = nleft;

   while((*lp++ = *bp++))	 /* transfer data	 */
      if (--nl == 0)
       { bp = getblock(addr = NEXTBLK(addr),READ);
	 nl = nleft;
       };
}





/********************************************************/
/*							*/
/*	TMput -- store text in temp file; return addr	*/
/*		 to use for future gets 		*/
/*							*/
/********************************************************/


ADDR
TMput(whr)
   TEXT *whr;			/* location to get data */
{
   register TEXT *bp,*lp;
   register INT nl;
   register ADDR tl;

   TEMPdirtcnt++;
   lp = whr;
   tl = tline;
   bp = getblock(tl,WRITE);
   nl = nleft;

   while ((*bp++ = *lp++))	  /* transfer data	  */
      if (--nl == 0)
       { tl = NEXTBLK(tl);
	 bp = getblock(tl,WRITE);
	 nl = nleft;
       };

   tl = tline;			/* update eof pointer	*/
   tline += (lp-whr);

   return tl;
}





/********************************************************/
/*							*/
/*	TMsync -- save temp file on disk if necessary	*/
/*							*/
/********************************************************/


void TMsync()
{
   if (TEMPdirtcnt >= PVAL(savecount) && PVAL(savecount) > 0)
      TMsave();
}





/********************************************************/
/*							*/
/*	TMsave -- force save of temp file on disk	*/
/*							*/
/********************************************************/


void TMsave()
{
   if (!PVAL(savetemp)) return;       /* unless no saves */
   if (!havetmp) return;

   savetemp();
}





/********************************************************/
/*							*/
/*	getblock -- get core address for file address	*/
/*							*/
/*   This routine takes a file address and returns the	*/
/*   corresponding address of the data in core.  The	*/
/*   resultant pointer is valid for (nleft) [the rest	*/
/*   or the buffer] characters.  The flag iof [READ|	*/
/*   WRITE] is used for maintaining the buffer. 	*/
/*							*/
/********************************************************/


static TEXT *
getblock(addr,iof)
   ADDR addr;			/* file address 	*/
   INT	iof;			/* i/o flag (READ|WRITE)*/
{
   register INT bno,off,i;
   register INT minhit,minbuf;

   bno = BLOCKNO(addr); 	/* get file location	*/
   off = OFFSET(addr);
   if (bno >= TNMBLKS)
      DSabort("Temporary file too long");
   nleft = TBUFSIZE-off;

   for (i = 0; i < TIBUFS; ++i) /* input buffers	*/
      if (bno == iblock[i])
       { ichng[i] |= iof;
	 hitin[i] = ++hitcount;
	 return (ibuff[i]+off);
       };

   minhit = hitin[0];		/* get buf using LRU	*/
   minbuf = 0;
   for (i = 1; i < TIBUFS; ++i)
      if (hitin[i] < minhit)
       { minhit = hitin[i];
	 minbuf = i;
       };
   i = minbuf;

   if (ichng[i])
    { if (iblock[i] > maxbno) maxbno = iblock[i];
      blkio(iblock[i],ibuff[i],write);
    };
   ichng[i] = iof;
   iblock[i] = bno;
   if (bno <= maxbno) blkio(bno,ibuff[i],read);
      else
       { if (maxbno+1 != bno || !iof) ABORT;
	 maxbno = bno;
	 blkio(bno,ibuff[i],write);
       };

   hitin[i] = ++hitcount;

   return (ibuff[i]+off);
}






/********************************************************/
/*							*/
/*	blkio -- read/write single block to temp file	*/
/*							*/
/********************************************************/


static void
blkio(bno,buf,iofct)
   INT bno;			/* block number 	*/
   TEXT *buf;			/* buffer location	*/
   INT (*iofct)();		/* read/write function	*/
{
   long long int off = bno;
   off *= TBUFSIZE;

   lseek64(tfile,off,0);
   if ((*iofct)(tfile,buf,TBUFSIZE) != TBUFSIZE)
      tmpioerr();
}





/********************************************************/
/*							*/
/*	savetemp -- save temp file on disk		*/
/*							*/
/*   This routine is called to insure that all incore	*/
/*   portions of the temp file are synchronized with	*/
/*   their disk counterparts by writing out whatever	*/
/*   has changed.					*/
/*							*/
/********************************************************/


static void
savetemp()
{
   register INT i,ln,lcnt;
   register BLOCK bp;
   long long int off;
   INT wln;

   TEMPdirtcnt = 0;

   for (i = 0; i < TIBUFS; ++i) /* save buffers 	*/
      if (ichng[i])
       { blkio(iblock[i],ibuff[i],write);
	 ichng[i] = 0;
       };

				/* save line blocks	*/
   lcnt = HEADER.THDlines = PVAL(filesize);
   bp = HEADER.THDlblk = NEXTBNO(tline)+SAVEFREE;

   for (i = 0; lcnt > 0; ++i)
    { ln = (lcnt+1)*sizeof(ADDR);
      if (ln > LBLKSIZE) ln = LBLKSIZE;
      off = bp;
      off *= TBUFSIZE;

      lseek64(tfile,off,0);

      wln = write(tfile,LNlbuff[i],ln);
      if (wln != ln)
	 tmpioerr();
      bp += FBPERLB;
      lcnt -= (LBLKSIZE/sizeof(ADDR));
    };

				/* save buffer and filename info */
   strcpy(HEADER.THDfile,PSVAL(filename));

				/* set up header	*/
   time(&HEADER.THDtime);
   HEADER.THDuser = getuid();
   HEADER.THDchanged = PVAL(nochange);

   lseek(tfile,0l,0);
   if (write(tfile,(char *)&HEADER,sizeof HEADER) != sizeof HEADER)
      tmpioerr();

   BFsync();
}







/********************************************************/
/*							*/
/*	tmpioerr -- report i/o error on temp file	*/
/*							*/
/********************************************************/


static void
tmpioerr()
{
   DSabort("I/O error on temporary file");
}





/* end of temp.c */
