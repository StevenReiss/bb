/********************************************************/
/*							*/
/*		RECOVER.C				*/
/*							*/
/*	This program handles recovery of b temporary	*/
/*	files after a crash.  Such recovery is done in	*/
/*	two parts.  First all temp files in /tmp	*/
/*	are checked and those that are viable b files	*/
/*	are moved to /mnt/preserve.  This is done by	*/
/*	system startup.  For each file moved, the owner */
/*	is sent mail informing him how to restore it.	*/
/*							*/
/*	Secondly,  the program handles user restore	*/
/*	requests, moving files from /mnt/preserve to	*/
/*	his directory, and reconstructing them and the	*/
/*	accompanying buffer files.			*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "linemod.h"
#include "bufmod.h"
#include "tempmod.h"
#include "cmds.h"
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>

#ifdef SYS5
#define index	strchr
#endif




/********************************************************/
/*							*/
/*	Internal Routines				*/
/*							*/
/********************************************************/


static	void	rcvr();
static	void	rcvrtext();
static	void	rcvrbufs();
static	void	movefiles();
static	void	move();





/********************************************************/
/*							*/
/*	Data storage					*/
/*							*/
/********************************************************/


static	struct _HEADER HDR;	/* temp file header blk */
static	INT	oldfid; 	/* file id for temp file*/
static	FILE *	newfid; 	/* file id for new files*/

static	CHAR	rnm[128];	/* recovery name	*/
static	CHAR	nnm[128];	/* new name		*/
static	CHAR	usr[32];	/* user name		*/

	BBUFINFO BI[BFACTNUM+1];	/* buffer data	*/
	BBUFDATA BD[NMCURBF];

#define TEMPDIR "/tmp"          /* original location of temp files */
#define RCVRDIR "/tmp" /* preserve location of temp files */





/********************************************************/
/*							*/
/*	Main program					*/
/*							*/
/********************************************************/


int main(argc,argv)
   INT argc;
   CHAR *argv[];
{
   struct passwd *pwd,*getpwuid();

   if (argc == 1)		/* NO arguments --	*/
    { movefiles();		/* phase 1 move files	*/
      exit(0);
    }

   if (argc != 2)		/* 1 argument -- user	*/
    { fprintf(stderr,"Usage: brecover [id]\n");
      exit(1);
    }

   pwd = getpwuid(getuid());
   strcpy(usr,pwd->pw_name);

   rcvr(argv[1],RCVRDIR);
}





/********************************************************/
/*							*/
/*	rcvr -- actually recover a file 		*/
/*							*/
/********************************************************/


static void rcvr(id,rdir)
   char *id;
   char *rdir;
{
   int fno;

   sprintf(rnm,"%s/#bTMP%s",rdir,id);
   if (access(rnm,6) < 0)
    { fprintf(stderr,"Recovery file non-existent.  Bad id\n");
      exit(1);
    }

   oldfid = open(rnm,O_RDONLY|O_LARGEFILE);
   if (oldfid < 0) abort();
   if (read(oldfid,&HDR,sizeof HDR) != sizeof HDR)
    { fprintf(stderr,"Recovery file error.  Bad header\n");
      exit(1);
    }

   sprintf(nnm,"#bRSTRXXXXXX");
   fno = mkstemp(nnm);
   if ((newfid = fdopen(fno,"w")) == NULL)
    { printf("Can't create output file in current directory. cd and retry.\n");
      exit(1);
    }
   printf("File %s being restored in %s\n",HDR.THDfile,nnm);
   if (HDR.THDchanged) printf("File has not been modified");

   rcvrtext();
   fclose(newfid);
   close(oldfid);
   unlink(rnm);

   sprintf(rnm,"%s/#bBUFS%s",rdir,id);
   if (access(rnm,6) < 0) {
      sprintf(rnm,"%s/#bBUFS%s",rdir,usr);
    }
   if (access(rnm,6) < 0) printf("No buffers associated with session\n");
    else
     { oldfid = open(rnm,O_RDONLY|O_LARGEFILE);
       if (oldfid < 0) abort();
       sprintf(nnm,"#bBUFSXXXXXX");
       fno = mkstemp(nnm);
       if ((newfid = fdopen(fno,"w")) == NULL) abort();
       printf("Buffers restored in %s\n",nnm);
       rcvrbufs();
       fclose(newfid);
       close(oldfid);
       unlink(rnm);
     }

   exit(0);
}





/********************************************************/
/*							*/
/*	rcvrtext -- recover the file text		*/
/*							*/
/********************************************************/


static void rcvrtext()
{
   ADDR lbuf[LNPERLB];
   INT lcnt,ln,nln,i,j;
   BLOCK bp;
   CHAR txt[1024];

   lcnt = HDR.THDlines;
   bp = HDR.THDlblk;

   for (i = 0; lcnt > 0; ++i)
    { ln = (lcnt)*sizeof(ADDR);
      if (ln > LBLKSIZE) ln = LBLKSIZE;
      lseek(oldfid,(long)(unsigned)bp*TBUFSIZE,0);
      nln = read(oldfid,lbuf,ln);
      if (nln != ln) printf("Error reading temp file\n");
      if (nln < 0) exit(1);
      bp += FBPERLB;
      lcnt -= LNPERLB;

      nln /= sizeof(ADDR);
      for (j = 0; j < nln; ++j)
	 if (lbuf[j] == 0) fprintf(newfid,"\n");
	  else
	   { lseek(oldfid,(long)(unsigned) lbuf[j],0);
	     read(oldfid,txt,1024);
	     fprintf(newfid,"%s\n",txt);
	   }
    }
}





/********************************************************/
/*							*/
/*	rcvrbufs -- recover buffers			*/
/*							*/
/********************************************************/


static void rcvrbufs()
{
   INT i,j,k,ch,txt;
   BUFBLOCK blk;

   read(oldfid,BI,sizeof(BI));

   for (i = 0; i < NMBUFS; ++i) if (BI[i].frsttxt > 0)
    { fprintf(newfid,"BF_%c\n",i+'A');
      k = 0;
      txt = BI[i].frsttxt;
      while (txt > 0)
       { lseek(oldfid,(long)(unsigned)txt*BFBLKSZ,0);
	 read(oldfid,&blk,BFBLKSZ);
	 txt = blk.BDnext;
	 for (j = 0; j < BFBLKSZ; ++j)
	  { ch = blk.BDtext[j];
	    if (ch == BFC_EOF) { txt = 0; break; }
	    if (ch >= CM_FIRST || ch < ' ')
	     { fprintf(newfid,"\\%3o",ch);
	       k += 3;
	     }
	    else if (ch == '\\')
	     { fprintf(newfid,"\\\\");
	       k++;
	     }
	    else fprintf(newfid,"%c",ch);
	    if (k++ >= 72) { fprintf(newfid,"\n"); k = 0; }
	  }
       }
      fprintf(newfid,"\n\n");
    }
}





/********************************************************/
/*							*/
/*	movefiles -- first stage of recovery process	*/
/*							*/
/********************************************************/


static void movefiles()
{
   CHAR nms[20480],*pt,*npt,cmd[128];
   FILE *fid,*popen();
   INT trm;

   sprintf(cmd,"echo %s/#bTMP*",TEMPDIR);
   fid = popen(cmd,"r");
   fgets(nms,20480,fid);
   pclose(fid);

   trm = nms[0];
   for (pt = nms; trm != '\n'; pt = npt)
    { for (npt = pt; index("\n\t ",*npt) == 0; ++npt);
      trm = *npt;
      *npt++ = 0;
      move(pt);
    }
}





/********************************************************/
/*							*/
/*	move -- move file to preserve area		*/
/*							*/
/********************************************************/


static void move(fl)
   STRING fl;
{
   struct stat buf;
   INT fuid,uid;
   CHAR id[10];
   CHAR cmd[128],rnm[128];
   FILE *fid,*popen();
   struct passwd *pwd,*getpwuid();
   long tim;
   INT fnum;

   stat(fl,&buf);
   fuid = buf.st_uid;
   tim = buf.st_mtime;
   uid = getuid();
   if (uid != fuid && uid != 0) return;
   strcpy(id,&fl[6+strlen(TEMPDIR)]);

   fnum = open(fl,O_RDONLY|O_LARGEFILE);
   if (fnum < 0) return;
   if (read(fnum,&HDR,sizeof HDR) != sizeof HDR) return;
   close(fnum);
   if (*HDR.THDfile == 0) return;

   if (HDR.THDchanged)
    { if (uid != 0) printf("No changes made to file %s.\n",HDR.THDfile);
      unlink(fl);
      sprintf(cmd,"%s/#bBUFS%s",TEMPDIR,id);
      unlink(cmd);
      return;
    }

   if (uid != 0)
    { rcvr(id,TEMPDIR);
      return;
    }

   pwd = getpwuid(fuid);
   strcpy(usr,pwd->pw_name);

   sprintf(cmd,"mv %s %s/#bTMP%s",fl,RCVRDIR,id);
   system(cmd);
   sprintf(cmd,"%s/#bTMP%s",RCVRDIR,id);
   chown(cmd,buf.st_uid,buf.st_gid);

   sprintf(rnm,"%s/#bBUFS%s",TEMPDIR,id);
   if (access(rnm,6) < 0) {
      sprintf(rnm,"%s/#bBUFS%s",TEMPDIR,usr);
    }
   if (access(rnm,6) >= 0) {
      sprintf(cmd,"mv %s %s/#bBUFS%s",rnm,RCVRDIR,id);
      system(cmd);
      sprintf(cmd,"%s/#bBUFS%s",RCVRDIR,id);
      chown(cmd,buf.st_uid,buf.st_gid);
    }

   pwd = getpwuid(fuid);
   sprintf(cmd,"mail %s >/dev/null",pwd->pw_name);
   fid = popen(cmd,"w");
   fprintf(fid,"\n");
   fprintf(fid,"An editor preserve file has been saved for you\n");
   fprintf(fid,"from the system or editor crash at about %s\n",ctime(&tim));
   fprintf(fid,"The source file was %s.\n",HDR.THDfile);
   fprintf(fid,"You may recover it by typing:\n");
   fprintf(fid,"\tbrecover %s\n\n",id);
   fprintf(fid,"\t\t\t\tThe b editor.\n");
   pclose(fid);
}





/* end of recover.c */
