/********************************************************/
/*							*/
/*		BUFMOD.C				*/
/*							*/
/*	This module processes the internal buffers	*/
/*	used for storing deleted or picked text and	*/
/*	macros.  Buffers are stored as a linked list	*/
/*	of blocks of <BFTEXTSZ>.  End of line and	*/
/*	end of text are indicated by special chars	*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "bufmod.h"
#include "cmds.h"
#include "parms.h"
#include <pwd.h>
#include <sys/stat.h>



/********************************************************/
/*							*/
/*	Parameters and constant definitions		*/
/*							*/
/********************************************************/


#define VOID	-1	/* void block address		*/


#define BFFRSTTEXT ((sizeof(BI)+BFBLKSZ-1)/BFBLKSZ)
			/* first data block in file	*/

#define NO_PID	-1
#define WRITE_PID -2

#define NOBUF	'\377'



/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


static	BBUFINFO BI[BFHDRNUM];	/* buffer information	*/

static	BBUFDATA BD[NMCURBF];	/* active buffers	*/

static	INT	 bufusecnt = 0; /* next active buffer	*/

static	BOOL	buffileok = FALSE; /* buffer file setup */
static	BOOL	initing = FALSE;	/* doing initialization */
static	CHAR	tempname[FNSIZE];  /* name of temp file for buffers */
static	INT	tempid; 	   /* file id of temp file	    */

static	INT	ourpid; 	/* pid of this process	*/
static	BOOL	share;		/* shared buffers flag	*/

static	BOOL	bfchng = FALSE; /* buffer file has changed */




/********************************************************/
/*							*/
/*	Routine Declarations				*/
/*							*/
/********************************************************/

#ifndef __STDC__
extern		write();	/* i/o functions	*/
extern		read();
#endif

static	void	checkbuf();
static	void	resetbuf();
static	void	savebuf();
static	INT	nextchr();
static	void	savechr();
static	void	retrieve();
static	ADDR	nextblk();
static	void	blkio();
static	void	bufioerr();
static	void	buflock();
static	void	sharebuf();



/********************************************************/
/*							*/
/*	BFinit -- initialize buffers			*/
/*							*/
/********************************************************/


void BFinit(fl,fg)
   STRING fl;			/* file to initialize from */
   BOOL fg;
{
   register INT i;
   register BOOL new;
   register STRING shf;
   struct passwd * pw;

   if (!fg && !buffileok) return;

   if (fl == NULL || *fl == 0) fl = PSVAL(bufinfile);
   share = PVAL(share);
   shf = PSVAL(useshare);
   if (share) ourpid = getpid();

   new = TRUE;

   if (buffileok) BFfini();

   if (!share) {
      sprintf(tempname,"%s/#bBUFSXXXXXX",PSVAL(tdirect));
      tempid = mkstemp(tempname);
      if (tempid < 0) bufioerr();
      fchmod(tempid,0600);
      close(tempid);
      tempid = open(tempname,O_RDWR|O_LARGEFILE);
      if (tempid < 0) bufioerr();
    }
   else {
      if (shf != NULL && *shf != 0) strcpy(tempname,shf);
      else {
	 pw = getpwuid(getuid());
	 if (pw == NULL) sprintf(tempname,"%s/#bBUFSxxx",PSVAL(tdirect));
	 else sprintf(tempname,"%s/#bBUFS%s",PSVAL(tdirect),pw->pw_name);
       };
      tempid = open(tempname,O_RDWR|O_LARGEFILE);
      if (tempid < 0) {
	 tempid = open(tempname,O_RDWR|O_CREAT|O_LARGEFILE,0600);
	 if (tempid < 0) bufioerr();
       }
      else {
	 new = FALSE;
       };
      buflock(TRUE,-1);
    };

   buffileok = TRUE;

   for (i = 0; i < BFACTNUM; ++i)
      BI[i].frsttxt = VOID;
   BI[BF_FREE].frsttxt = VOID;
   BI[BF_FREE].curtxt = BFFRSTTEXT;
   BI[BF_FREE].pid = NO_PID;
   BI[BF_FREE].id = NOBUF;

   if (share) {
      BI[BF_USECT].frsttxt = VOID;
      BI[BF_USECT].curtxt = 0;
      BI[BF_USECT].pid = NO_PID;
      BI[BF_USECT].id = NOBUF;
      if (!new) sharebuf(BF_USECT);
      BI[BF_USECT].curtxt += 1;
    };

   for (i = 0; i < BFACTNUM; ++i)
    { BI[i].id = NOBUF;
      BI[i].txtidx = -1;
      BI[i].curtxt = VOID;
      BI[i].pid = NO_PID;
    };

   for (i = 0; i < NMCURBF; ++i)
    { BD[i].bufid = NOBUF;
      BD[i].usecnt = -1;
      BD[i].writefg = FALSE;
    };

   if (!share) BFsync();
   else {
      if (new) {
	 lseek(tempid,(long)0,0);
	 write(tempid,BI,BFFRSTTEXT*BFBLKSZ);
       };
      buflock(FALSE,BF_USECT);
    };

   if (!share || new) {
      initing = TRUE;
      if (fl != NULL && *fl != 0) retrieve(fl);
      initing = FALSE;
    };

   bfchng = FALSE;
}






/********************************************************/
/*							*/
/*	BFfree -- release buffer contents		*/
/*							*/
/********************************************************/


void BFfree(txt)
   ADDR txt;
{
   BUFBLOCK blk;
   register ADDR id;

   if (txt == VOID) return;

   buflock(TRUE,-1);

   for (id = txt; ; id = blk.BDnext)
    { blkio(id,&blk,read);
      if (blk.BDnext == VOID) break;
    };

   sharebuf(BF_FREE);

   blk.BDnext = BI[BF_FREE].frsttxt;
   blkio(id,&blk,write);
   BI[BF_FREE].frsttxt = txt;

   buflock(FALSE,BF_FREE);

   bfchng = TRUE;
}






/********************************************************/
/*							*/
/*	BFset -- set buffer for reading 		*/
/*							*/
/********************************************************/


void BFset(buf)
   INT buf;		/* buffer to set up		*/
{
   if (!buffileok) BFinit(NULLSTR,TRUE);

   bfchng = TRUE;

   switch(buf)
    { case BF_SINK    : 	/* handle special bufs	*/
      case BF_SPACE   :
      case BF_NEWLINE :
      case BF_STAR    :
		BI[buf].txtidx = -1;
		break;

      default	      :
		sharebuf(buf);
		checkbuf(buf);	/* insure normal buf is active	*/
		BD[BI[buf].id].usecnt = bufusecnt++;
		resetbuf(buf);	/* and then rewind it		*/
		break;
    };
}





/********************************************************/
/*							*/
/*	BFget -- return next character from buffer	*/
/*							*/
/********************************************************/


INT
BFget(buf)
   INT buf;		/* buffer to use		*/
{
   if (!buffileok) BFinit(NULLSTR,TRUE);

   switch(buf)
    { case BF_SINK	:		/* SINK buffer	*/
		return BFC_EOF;

      case BF_SPACE	:		/* SPACES	*/
      case BF_NEWLINE	:		/* NEWLINES	*/
		if (BI[buf].txtidx++ < 0)
		   return (buf == BF_SPACE) ? ' ' : BFC_EOL;
		 else return BFC_EOF;
      case BF_STAR	:
		if (BI[buf].txtidx++ < 0) return '*';
		 else return BFC_EOF;

      default		:
		checkbuf(buf);	/* insure normal buf active */
		return nextchr(buf);	/* return next chr  */
    };
}





/********************************************************/
/*							*/
/*	BFclear -- clear buffer for writing		*/
/*							*/
/********************************************************/


void BFclear(buf)
   INT buf;		/* buffer to use		*/
{
   register BUFINFO bi;

   if (!buffileok) BFinit(NULLSTR,TRUE);

   bfchng = TRUE;

   switch(buf)
    { case BF_SINK	:		/* sink buffer	*/
		break;

      case BF_SPACE	:		/* spaces	*/
      case BF_NEWLINE	:		/* and lines	*/
      case BF_STAR	:		/* and stars	*/
		ABORT;			/* are readonly */

      default		:
		bi = &BI[buf];		/* cur buffer info */
		if (!initing) {
		   buflock(TRUE,-1);
		   sharebuf(buf);
		   HSbuf(buf,bi->frsttxt);
		   buflock(FALSE,-1);
		 };
		bi->curtxt = VOID;
		bi->txtidx = -1;
		bi->frsttxt = VOID;
		bi->pid = WRITE_PID;
		checkbuf(buf);		/* make empty buffer active */
		BD[bi->id].usecnt = ++bufusecnt;
		resetbuf(buf);		/* and insure rewound	*/
		break;
    };
}





/********************************************************/
/*							*/
/*	BFputch -- save character in buffer		*/
/*							*/
/********************************************************/


void BFputch(ch,buf)
   TEXT ch;		/* character to store		*/
   INT buf;		/* buffer to store it in	*/
{
   if (!buffileok) BFinit(NULLSTR,TRUE);

   if (buf == BF_SINK) return;

   checkbuf(buf);	/* insure buffer is active	*/
   savechr(buf,ch);
}





/********************************************************/
/*							*/
/*	BFrepch -- replace last char saved in buffer	*/
/*							*/
/********************************************************/


void BFrepch(ch,buf)
   TEXT ch;		/* character to store		*/
   INT buf;		/* buffer to store it in	*/
{
   register BUFINFO bi;

   if (!buffileok) BFinit(NULLSTR,TRUE);

   if (buf == BF_SINK) return;

   checkbuf(buf);	/* insure buffer is active	*/

   bi = &BI[buf];
   --bi->txtptr;
   --bi->txtidx;

   savechr(buf,ch);
}





/********************************************************/
/*							*/
/*	BFsave -- save buffer into given file		*/
/*							*/
/********************************************************/


void BFsave(file)
   STRING file;
{
   register FILE *otf;
   register INT i,j;
   register TEXT ch;
   char buffer[BUFSIZ];

   if (!buffileok) BFinit(NULLSTR,TRUE);

   if (file != NULL && file[0] == '~') file = EDfixfilename(file);

   if (file != NULL && *file != 0 && (otf = fopen(file,"w")) != NULL) {
      setbuf(otf,buffer);
      for (i = 0; i < NMBUFS; ++i) {
	 sharebuf(i);
	 if (BI[i].frsttxt != VOID) {
	    fprintf(otf,"BF_%c\n",i+'A');
	    BFset(i);
	    j = 0;
	    while ((ch = BFget(i)) != BFC_EOF) {
	       if (ch >= CM_FIRST || ch < ' ') {
		  fprintf(otf,"\\%3o",ch);
		  j += 3;
		}
	       else if (ch == '\\') {
		  fprintf(otf,"\\\\");
		  ++j;
		}
	       else fprintf(otf,"%c",ch);
	       if (j++ >= 72) { fprintf(otf,"\n"); j = 0; };
	     };
	    fprintf(otf,"\n\n");
	  };
       };
      fclose(otf);
      setbuf(otf,NULL);
    };
}





/********************************************************/
/*							*/
/*	BFsync -- insure buffer file is up to date	*/
/*							*/
/********************************************************/


void BFsync()
{
   register INT i;

   if (!bfchng || !buffileok) return;
   if (share) return;

   for (i = 0; i < NMCURBF; ++i)
      if (BD[i].bufid != NOBUF) savebuf(BD[i].bufid);

   lseek(tempid,(long)0,0);
   write(tempid,BI,BFFRSTTEXT*BFBLKSZ);
   bfchng = FALSE;
}






/********************************************************/
/*							*/
/*	BFfini -- finish up with buffer activity	*/
/*							*/
/********************************************************/


void BFfini()
{
   if (!buffileok) return;

   if (*PSVAL(bufoutfile) != 0) BFsave(PSVAL(bufoutfile));

   if (!share) {
      unlink(tempname);
    }
   else {
      buflock(TRUE,-1);
      sharebuf(BF_USECT);
      BI[BF_USECT].curtxt -= 1;
      if (BI[BF_USECT].curtxt == 0) unlink(tempname);
      else buflock(FALSE,BF_USECT);
    };

   close(tempid);

   buffileok = FALSE;
}





/********************************************************/
/*							*/
/*	BFundo -- undo buffer set, restore buffer	*/
/*							*/
/********************************************************/


void BFundo(buf,text)
   INT buf;
   INT text;
{
   register BUFINFO bi;
   register BUFDATA bd;

   checkbuf(buf);
   savebuf(buf);

   sharebuf(buf);

   bi = &BI[buf];
   BFfree(bi->frsttxt);
   bi->curtxt = VOID;
   bi->txtidx = -1;
   bi->frsttxt = text;
   bi->pid = WRITE_PID;
   if (bi->id != NOBUF)
    { bd = &BD[bi->id];
      bd->writefg = FALSE;
    };

   BFset(buf);
}






/********************************************************/
/*							*/
/*	checkbuf -- insure that buffer is active	*/
/*							*/
/********************************************************/


static void
checkbuf(buf)
   INT buf;		/*  buffer to make active	*/
{
   register BUFINFO bi;
   register BUFDATA bd;
   register INT i,j;

   bi = &BI[buf];

   if (bi->id != NOBUF && BD[bi->id].bufid != buf) abort();

   if (bi->id != NOBUF) return;     /* buffer is active     */

   j = 0;			/* LRU scheme to get buffer */
   if (BD[j].bufid != NOBUF) {
      for (i = 1; i < NMCURBF; ++i)
	 if (BD[i].bufid == NOBUF) { j = i; break; }
	    else if (BD[i].usecnt < BD[j].usecnt) j = i;
    }
   bd = &BD[j];
   bi->id = j;

   if (bd->bufid != NOBUF)	 /* release old contents */
    { savebuf(bd->bufid);
      BI[bd->bufid].id = NOBUF;
    };
   bd->writefg = FALSE;

   if (bi->curtxt != VOID)
    { blkio(bi->curtxt,&bd->data,read);
      bi->txtptr = &(bd->data.BDtext[bi->txtidx]);
    };
   bd->bufid = buf;
}





/********************************************************/
/*							*/
/*	resetbuf -- rewind active buffer		*/
/*							*/
/********************************************************/


static void
resetbuf(buf)
   INT buf;			/* buffer to rewind	*/
{
   register BUFINFO bi;
   register BUFDATA bd;

   bi = &BI[buf];
   if (bi->id == NOBUF) ABORT;
   bd = &BD[bi->id];

   savebuf(buf);		/* insure old info saved */

   if (bi->curtxt != bi->frsttxt)
    { bi->curtxt = bi->frsttxt;
      if (bi->curtxt != VOID)
	 blkio(bi->curtxt,&bd->data,read);
    };

   bi->txtidx = 0;
   bi->txtptr = &(bd->data.BDtext[0]);
   bd->writefg = FALSE;
}





/********************************************************/
/*							*/
/*	savebuf -- put in core into into temp file	*/
/*							*/
/********************************************************/


static void
savebuf(buf)
   INT buf;
{
   register BUFINFO bi;
   register BUFDATA bd;

   bi = &BI[buf];
   bd = &BD[bi->id];

   if (bd->writefg && bi->curtxt >= 0) {
      blkio(bi->curtxt,&bd->data,write);
    };
}





/********************************************************/
/*							*/
/*	nextchr -- get next character from buffer	*/
/*							*/
/********************************************************/


static INT
nextchr(buf)
   INT buf;			/* buffer to use	*/
{
   register INT ch;
   register BUFINFO bi;
   register BUFDATA bd;

   bi = &BI[buf];
   if (bi->id == NOBUF) ABORT;	     /* buffer must be active */
   bd = &BD[bi->id];

   while (bi->curtxt != VOID && (ch = *bi->txtptr) == 0)
    { bi->curtxt = bd->data.BDnext;
      if (bi->curtxt != VOID)
       { blkio(bi->curtxt,&bd->data,read);
	 bi->txtptr = &(bd->data.BDtext[0]);
       };
    };
   if (bi->curtxt == VOID) return BFC_EOF;

   bi->txtidx++;
   bi->txtptr++;

   return ch;
}





/********************************************************/
/*							*/
/*	savechr -- save character in buffer		*/
/*							*/
/********************************************************/


static void
savechr(buf,ch)
   INT buf;			/* buffer to use	*/
   INT ch;			/* character to save	*/
{
   register BUFINFO bi;
   register BUFDATA bd;

   if (ch == 0) return; 	/* cant store nulls	*/

   bi = &BI[buf];
   if (bi->id == NOBUF) ABORT;	     /* buffer must be active */
   bd = &BD[bi->id];

   if (bi->txtidx >= BFTEXTSZ) ABORT;
   if (bi->txtidx == BFTEXTSZ-1)
    { bd->data.BDnext = nextblk();
      blkio(bi->curtxt,&bd->data,write);
      bi->curtxt = bd->data.BDnext;
      bi->txtidx = -1;
    }
   else if (bi->curtxt == VOID)
    { bi->curtxt = nextblk();
      if (bi->frsttxt == VOID) bi->frsttxt = bi->curtxt;
      bi->txtidx = -1;
    };

   if (bi->txtidx < 0)
    { bd->data.BDnext = VOID;
      bi->txtidx = 0;
      bi->txtptr = &(bd->data.BDtext[0]);
    };

   bd->writefg = TRUE;
   *(bi->txtptr++) = ch;
   *(bi->txtptr) = 0;
   ++(bi->txtidx);
}





/********************************************************/
/*							*/
/*	retrieve -- get buffer saved in file		*/
/*							*/
/********************************************************/


static void
retrieve(file)
   STRING file;
{
   register FILE *inf;
   register TEXT ch;
   register INT buf;
   register TEXT *pt;
   TEXT line[256];
   CHAR buffer[BUFSIZ];

   if (file[0] == '~') file = EDfixfilename(file);

   inf = fopen(file,"r");
   if (inf == NULL)
    { DSerror("Cannot open buffer file");
      return;
    };
   setbuf(inf,buffer);

   for ( ; ; )
    { buf = -1;
      do {
	 if (fgets((char *) line,256,inf) == NULL) break;
	 if (strncmp((char *) line,"BF_",3) == 0 && line[4] == '\n')
	  { buf = line[3]-'A';
	    if (buf < 0 || buf >= NMBUFS) buf = -1;
	  };
       }
      while (buf < 0);
      if (buf < 0) break;			/* eof */

      BFclear(buf);
      while (fgets((char *) line,256,inf) != NULL && line[0] != '\n')
       { pt = line;
	 while ((ch = *pt++) != 0)
	  { if (ch == '\n') continue;
	    if (ch == '\\' && *pt == '\\') ++pt;
	     else if (ch == '\\')
	      { ch = *pt++ - '0';
		ch = (ch<<3) + *pt++ - '0';
		ch = (ch<<3) + *pt++ - '0';
	      };
	    BFputch(ch,buf);
	  };
       };
      BFputch(BFC_EOF,buf);
      BFset(buf);
    };

   fclose(inf);
   setbuf(inf,NULL);

   BFsync();
}






/********************************************************/
/*							*/
/*	nextblk -- get next free block in file		*/
/*							*/
/********************************************************/


static ADDR
nextblk()
{
   BUFBLOCK blk;
   register ADDR id;

   buflock(TRUE,-1);

   sharebuf(BF_FREE);

   if ((id = BI[BF_FREE].frsttxt) != VOID)
    { blkio(id,&blk,read);
      BI[BF_FREE].frsttxt = blk.BDnext;
    }
   else id = BI[BF_FREE].curtxt++;

   buflock(FALSE,BF_FREE);

   return id;
}





/********************************************************/
/*							*/
/*	blkio -- write block to buffer file		*/
/*							*/
/********************************************************/


static void
blkio(bno,addr,fct)
   ADDR bno;
   CHAR *addr;
   INT (*fct)();
{
   long long int off = bno;
   int sts;

   off *= BFBLKSZ;

   lseek64(tempid,off,0);
   sts = (*fct)(tempid,addr,BFBLKSZ);

   if (sts != BFBLKSZ) bufioerr();
}





/********************************************************/
/*							*/
/*	bufioerr -- report i/o error for buffer file	*/
/*							*/
/********************************************************/


static void
bufioerr()
{
   DSabort("Error in buffer file I/O");
}





/********************************************************/
/*							*/
/*	buflock -- lock buffer file in shared mode	*/
/*							*/
/********************************************************/


static void
buflock(fg,buf)
   BOOL fg;
   INT buf;
{
   if (!share) return;

   if (fg) {
#ifndef USE_FLOCK
      lseek(tempid,0,0);
      lockf(tempid,F_LOCK,0);
#else
      flock(tempid,LOCK_EX);
#endif
    };

   if (buf >= 0) {
      long long int off = buf;
      off *= sizeof(BBUFINFO);
      BI[buf].pid = ourpid;
      lseek64(tempid,off,0);
      write(tempid,&BI[buf],sizeof(BBUFINFO));
    };

   if (!fg || buf >= 0) {
#ifndef USE_FLOCK
      lseek(tempid,0,0);
      lockf(tempid,F_ULOCK,0);
#else
      flock(tempid,LOCK_UN);
#endif
    };
}





/********************************************************/
/*							*/
/*	sharebuf -- handle buffer sharing		*/
/*							*/
/********************************************************/


static void
sharebuf(buf)
   INT buf;
{
   register BUFINFO bi;
   register INT fst;
   BBUFINFO temp;

   if (!share) return;

   bi = &BI[buf];

   if (bi->pid == WRITE_PID) {
      bi->pid = ourpid;
      buflock(TRUE,buf);
    }
   else {
      long long int off = buf;
      off *= sizeof(BBUFINFO);
      lseek64(tempid,off,0);
      read(tempid,&temp,sizeof(BBUFINFO));
      if (temp.pid != ourpid && temp.pid != NO_PID) {
	 if (bi->id != NOBUF && buf < BFACTNUM) BD[bi->id].bufid = NOBUF;
	 BI[buf] = temp;
	 bi->id = NOBUF;
	 if (buf < BFACTNUM) bi->curtxt = VOID;
       };
    };
}





/* end of bufmod.c */
