/********************************************************/
/*							*/
/*		FILEMOD.C				*/
/*							*/
/*	This file contains the code for managing the	*/
/*   actual user input and output file.  It provides	*/
/*   calls to open/close/read next line/write next	*/
/*   line.						*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "filemod.h"
#include "parms.h"
#include "lbuff.h"
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>



/********************************************************/
/*							*/
/*	External Routines				*/
/*							*/
/********************************************************/


extern	void	CRinset();	/* set up crypting	*/
extern	int	CRinchr();	/* encrypt input	*/
extern	int	CRotchr();	/* encrypt output	*/



/********************************************************/
/*							*/
/*	Internal Routines				*/
/*							*/
/********************************************************/


static	int	badfile();
static	void	readfile();
static	void	writefile();
static	void	savebackup();
static	void	makedir();
static	void	close_input_file();



/********************************************************/
/*							*/
/*	Static storage					*/
/*							*/
/********************************************************/


#define FILEBUFSZ IOPGSIZE

extern	ADDR *	LNlbuff[];	/* current line buffer	*/

static	INT	infid;		/* input file fid	*/
static	INT	otfid;		/* output file fid	*/
static	INT	bufid;		/* backup file fid	*/

static	INT	inprotect;	/* file protection bits */

static	CHAR	backupname[256];/* backup name		*/

static	TEXT	iobuf[FILEBUFSZ];	/* i/o buffer	*/
static	int	iobufct;	/* chars left in buffer */
static	TEXT *	iobufp; 	/* i/o buffer pointer	*/
static	BOOL	eofg = TRUE;	/* eof flag		*/
static	BOOL	readyfg = FALSE;/* file is ready flag	*/




/********************************************************/
/*							*/
/*	FLinit -- initialize for new input file 	*/
/*							*/
/********************************************************/


FLSTATUS
FLinit(newfg)
   INT newfg;		/* is a new file allowed??	*/
{
   struct stat64 sbuf;		  /* file status buffer   */
   CHAR tmpnam[FNSIZE];
   extern int errno;
   BOOL ronly;

   strcpy(tmpnam,"#bBUTXXXXXX");

   otfid = bufid = -1;
   ronly = TRUE;
   infid = open(PSVAL(filename),O_RDWR|O_LARGEFILE);
   if (infid >= 0) ronly = FALSE;
   else infid = open(PSVAL(filename),O_RDONLY|O_LARGEFILE);

   if (infid < 0)		/* open input file	*/
    { if (access(PSVAL(filename),0) >= 0) return FL_FAIL;
      if (!newfg) return FL_NEW;

      eofg = TRUE;		/* start a new file	*/
      inprotect = umask(0);
      umask(inprotect);
      inprotect = 0666 & ~inprotect;
      iobufct = 0;
      readyfg = TRUE;
      if (PVAL(curcrypt)) CRinset(PSVAL(filename));
      PSET(readonly,FALSE);
      PSET(nochange,FALSE);

      MNopen();

      return access(PSVAL(directory),3) < 0 ? FL_READ : FL_OK;
    };

   fstat64(infid,&sbuf);	  /* check up on old file */
   if (badfile(infid,&sbuf))
    { close_input_file(FALSE);
      return FL_BAD;
    };

#ifndef USE_FLOCK
   if (!ronly && !PVAL(nlock) && lockf(infid,F_TLOCK,0) < 0) {
      newfg = errno;
      close_input_file(FALSE);
      return FL_LOCK;
    };
#else
#ifdef LOCK_EX
#ifndef SYS5
   if (!ronly && !PVAL(nlock) && flock(infid,LOCK_EX|LOCK_NB) < 0) {
      newfg = errno;
      close_input_file(FALSE);
      return FL_LOCK;
    };
#endif
#endif
#endif

   eofg = FALSE;		/* set up old file	*/
   inprotect = sbuf.st_mode & 0777;

   if (PVAL(curcrypt)) CRinset(PSVAL(filename));

   if (PVAL(curcopy) && PVAL(backup) > 0 && sbuf.st_size < MAX_BACKUP_SIZE) {
      sprintf(backupname,"%s/%s",PSVAL(tdirect),tmpnam);
      bufid = mkstemp(backupname);
      if (bufid < 0)
	 DSerror("Can't create backup file; no backup saved");
      fchmod(bufid,inprotect);
    };

   iobufct = 0;
   readyfg = TRUE;

   MNopen();

   if (ronly) return FL_READ;

   return (access(PSVAL(filename),2) < 0) ||
	  (!PVAL(curcopy) && access(PSVAL(directory),3) < 0) ? FL_READ :
	  FL_OK;
}




/********************************************************/
/*							*/
/*	FLget -- read next line into line buffer	*/
/*							*/
/********************************************************/


INT
FLget()
{
   register INT ch;		/* current character	*/
   register INT lsiz;		/* line size		*/
   register TEXT *lp;		/* line buffer pointer	*/
   BOOL crypt;
   BOOL crlf;
   BOOL hadcr;

   lsiz = 0;
   lp = LBlinebuf;

   crypt = PVAL(curcrypt);
   crlf = PVAL(curcrlf);
   hadcr = FALSE;

   do {
      if (iobufct <= 0 && !eofg) readfile();
      if (eofg) break;
      while (iobufct-- > 0) {
	 ch = *iobufp++;
	 if (crypt) ch = CRinchr(ch&0xff);
	 ch &= 0x7f;
	 if (ch == '\n') goto done;
	 if (ch == 0) continue;
	 if (hadcr) {
	    ++iobufct;
	    --iobufp;
	    goto done;
	  }
	 if (crlf && ch == '\r') hadcr = TRUE;
	 *lp++ = ch;
	 if (++lsiz >= LINESIZE-1) {
	    DSerror("Line too long -- split");
	    goto done;
	  }
       }
    }
   while (!eofg);

done:
   if (hadcr) --lp;
   *lp++ = 0;
   if (eofg && lsiz == 0) lsiz = -1;

   return (lsiz >= 0);
}





/********************************************************/
/*							*/
/*	FLoutput -- open output file; backup as needed	*/
/*							*/
/********************************************************/


FLSTATUS
FLoutput()
{
   if (!readyfg) return FL_FAIL;

   if (!eofg) ABORT;		/* only input or output */
   close_input_file(TRUE);

   if (bufid >= 0) close(bufid);
   if (!PVAL(curcopy) && PVAL(backup) > 0)
    { strcpy(backupname,PSVAL(filename));
      bufid = 0;
    };

   if (bufid >= 0) savebackup();

   if (!PVAL(curcopy)) unlink(PSVAL(filename));
   otfid = open(PSVAL(filename),O_CREAT|O_WRONLY|O_TRUNC|O_LARGEFILE,inprotect);

   if (otfid < 0) return FL_FAIL;

   iobufct = 0;
   iobufp = iobuf;

   return FL_OK;
}





/********************************************************/
/*							*/
/*	FLput -- write line buffer to output file	*/
/*							*/
/********************************************************/


INT FLput()
{
   register TEXT ch,nch;	/* current character	*/
   register TEXT *lp;		/* line buffer pointer	*/

   lp = LBlinebuf;
   do
    { if (iobufct == FILEBUFSZ) writefile();
      ++iobufct;
      ch = nch = *lp++;
      if (ch == 0) nch = '\n';
      if (PVAL(curcrypt)) nch = CRotchr(nch);
      *iobufp++ = nch;
    }
   while (ch != 0);

   return TRUE;
}





/********************************************************/
/*							*/
/*	FLclose -- close all files			*/
/*							*/
/********************************************************/


void FLclose()
{
   if (!readyfg) return;

   if (infid >= 0) close_input_file(TRUE);

   if (bufid > 0)
    { close(bufid);
      unlink(backupname);
    };
   bufid = -1;

   if (otfid >= 0)
    { writefile();		/* output last buffer load	*/
      close(otfid);
      otfid = -1;
    };

   readyfg = FALSE;
}





/********************************************************/
/*							*/
/*	badfile -- insure nice ascii file		*/
/*							*/
/********************************************************/


static int
badfile(fid,sbufp)
   INT fid;			/* file id		*/
   struct stat *sbufp;		/* file status buffer	*/
{
   register INT i;
   unsigned short magic;

   switch (sbufp->st_mode&S_IFMT)
    { case S_IFBLK :
      case S_IFCHR :
      case S_IFDIR : return TRUE;	/* bad device file */
      default	   : break;
    };

   if (PVAL(curcrypt)) return FALSE;

#ifndef apollo
   i = read(fid,&magic,sizeof(magic));	/* check magic #   */
   lseek(fid,0l,0);
   if (i == sizeof(magic)) {
#ifndef vax
      swab((char *) &magic,(char *) &magic,2);
#endif
      switch(magic)
       { case 0405 :
	 case 0407 :
	 case 0410 :
	 case 0411 :
	 case 0413 :
	 case 0177545 :
	 case 0177555 : return TRUE;	/* bad magic #	   */
	 default   : break;
       };
      if (magic & 0100200) return TRUE; /* non-ascii file  */
    };
#endif

   return FALSE;			/* all ok	*/
}





/********************************************************/
/*							*/
/*	readfile -- read next buffer from input file	*/
/*							*/
/********************************************************/


static void
readfile()
{
   iobufct = read(infid,iobuf,FILEBUFSZ);
   if (iobufct <= 0)
      eofg = TRUE;
   iobufp = iobuf;

   if (bufid > 0 && iobufct > 0)
      write(bufid,iobuf,iobufct);
}





/********************************************************/
/*							*/
/*	writefile -- write iobuffer to output file	*/
/*							*/
/********************************************************/


static void
writefile()
{
   if (iobufct == 0) return;

   if (write(otfid,iobuf,iobufct) < 0)
    { PSET(ioerr,TRUE);
      DSabort("I/O error on file write");
    };

   iobufp = iobuf;
   iobufct = 0;
}





/********************************************************/
/*							*/
/*	savebackup -- save backup file			*/
/*							*/
/********************************************************/


static void
savebackup()
{
   register INT i,ln;
   CHAR buname[FNSIZE], buname1[FNSIZE];
   SHORT nln;
   INT status;

   bufid = -1;

   sprintf(buname1,"%s/bBACKUP",PSVAL(directory));
   for (i = 0; i < 3; ++i)	/* find backup directory*/
    { if (access(buname1,0) < 0) makedir(buname1);
      if (access(buname1,6) >= 0) break;
      if (i == 0)
       { sprintf(buname1,"%s/bBACKUP",getenv("HOME"));
	 DSerror("Can't use local bBACKUP; using home directory");
       };
    };
   if (i == 3)
    { DSerror("Can't access bBACKUP; no backup saved");
      return;
    };

   sprintf(buname,"%s/%s.0",buname1,PSVAL(filetail));
   ln = strlen(buname)-1;		/* find backup file name	*/
   for (i = 0; i < PVAL(backup); ++i)
    { buname[ln] = '0'+i;
      if (access(buname,4) < 0) goto found;
    };
   buname[ln] = '0';
   for (i = 1; i < PVAL(backup); ++i)
    { strcpy(buname1,buname);
      buname[ln] = '0'+i;
      rename(buname,buname1);
    };

found:
   if (rename(backupname,buname) < 0) {
      int f1,f2,ln;
      char buf[1024];

      f1 = open(backupname,O_RDONLY|O_LARGEFILE);
      f2 = open(buname,O_CREAT|O_WRONLY|O_LARGEFILE,inprotect);
      while ((ln = read(f1,buf,1024)) > 0)
	 write(f2,buf,ln);
      close(f1);
      close(f2);
      if (f2 >= 0) unlink(backupname);
    };
}




/********************************************************/
/*							*/
/*	makedir -- create a directory			*/
/*							*/
/********************************************************/


static void
makedir(nm)
   TEXT *nm;			/* directory name	*/
{
   CHAR cmd[FNSIZE+16];
   SHORT nln;
   INT status;

#ifndef NO_MKDIR
   mkdir((const char *) nm,0777);
#else
   sprintf(cmd,"mkdir %s",nm);
   local_system(cmd,0);
#endif
}




/************************************************************************/
/*									*/
/*	close_input_file -- close the input file			*/
/*									*/
/************************************************************************/


static void
close_input_file(lk)
   BOOL lk;
{
   if (infid < 0) return;

   if (lk) {
#ifndef USE_FLOCK
      lseek(infid,0l,0);
      lockf(infid,F_ULOCK,0);
#else
      flock(infid,LOCK_UN);
#endif
    };

   close(infid);
   infid = -1;
}




/* end of filemod.c */
