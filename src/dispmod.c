/********************************************************/
/*							*/
/*		DISPMOD.C				*/
/*							*/
/*	This module contains routines for managing the	*/
/*	display.  It controls both input and output,	*/
/*	doing input via the input module and output	*/
/*	via the terminal module.  Editor requests to	*/
/*	update the display all must go through this	*/
/*	module first					*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#define DISPMOD

#include "global.h"
#include "editor.h"
#include "lbuff.h"
#include "parms.h"
#include "cmds.h"
#include <signal.h>
#include "linemod.h"



/********************************************************/
/*							*/
/*	Routine Declarations				*/
/*							*/
/********************************************************/


static	int	DSput();	/* display line 	*/
static	void	outmsg();
static	void	dispinfo();
static	void	getflags();
static	void	eofdisplay();
static	void	dispfinish();

extern	int	local_system();



/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


static	BOOL	dispready = FALSE; /* display is ready	*/

static	INT	nummarks = 0;	   /* number of markers */





/********************************************************/
/*							*/
/*	Tables						*/
/*							*/
/********************************************************/


			/* flags for display parms	*/
#define CLFstring 0x1	/*    string value		*/
#define CLFnumber 0x2	/*    numeric value		*/
#define CLFign	  0x4	/*    ignore on flag output	*/


static struct { 	/* display line parameter flags */
   INT CLparm;		/*    parm id			*/
   STRING CLname;	/*    normal name		*/
   STRING CLshort;	/*    abbreviated name		*/
   INT CLflags; 	/*    describing flags		*/
   PARMVAL CLval;	/*    current value		*/
 }
CLTBL[] = {
   { P_filename    , ""         , ""     , CLFstring|CLFign , (PARMVAL) 0 },
   { P_curreadonly , "Readonly" , "RD"   , 0                , (PARMVAL)-1 },
   { P_curstream   , "Stream"   , "ST"   , 0                , (PARMVAL)-1 },
   { P_curlanguage , "Lang="    , "LA="  , CLFstring        , (PARMVAL) 0 },
   { P_copmode	   , "Add-Char" , "ADCH" , 0                , (PARMVAL)-1 },
   { P_lopmode	   , "Insert"   , "INS"  , 0                , (PARMVAL)-1 },
   { P_marking	   , "Marking"  , "MK"   , 0                , (PARMVAL)-1 },
   { P_curscreen   , ""         , ""     , CLFnumber|CLFign , (PARMVAL)-1 },
   { P_curmargin , ""         , ""     , CLFnumber|CLFign , (PARMVAL)-1 },
   { P_filesize  , ""         , ""     , CLFnumber|CLFign , (PARMVAL)-1 },

   { -1,0,0,0,(PARMVAL)0 }
};

				/* force command line to be redrawn	*/
#define BUGCMD CLTBL[0].CLval = (PARMVAL)0





/********************************************************/
/*							*/
/*	DSinit -- initialize display i/o		*/
/*							*/
/********************************************************/


void DSinit()
{
   CHAR buf[24];
   register STRING pt;
   register INT i;

   if (dispready) return;

   INinit();

   TRnew(PSVAL(terminal));	/* set up terminal output */
   TRclear();

   if (!PVAL(slow) && TRinfo("s")) PSET(slow,TRUE);
#ifndef FLEXWIN
   if (PVAL(termwidth) == 0) PSET(termwidth,TRinfo("w"));
      else {
	 sprintf(buf,"S%d",PVAL(termwidth));
	 TRinfo(buf);
       };
   if (PVAL(termlength) == 0) PSET(termlength,TRinfo("l"));
      else {
	 sprintf(buf,"L%d",PVAL(termlength));
	 TRinfo(buf);
       };
#else
   i = TRinfo("w");
   if (PVAL(termwidth) != i) PSETI(termwidth,i);
   i = TRinfo("l");
   if (PVAL(termlength) != i) PSETI(termlength,i);
#endif

   buf[0] = 'b'; buf[1] = CHR_BS; buf[2] = 0;
   TRinfo(buf);

   if (PVAL(slow)) DSslowdisp();

   BUGCMD;

   dispready = TRUE;
   nummarks = 0;

   pt = PSVAL(interminal);
   if (strcmp(pt,"=") == 0 || *pt == 0) pt = PSVAL(terminal);
   INsetup(pt,"-e-lc-r-n");     /* set up terminal input        */

// INsetup(pt,"-e-lc-r-n");     /* set up terminal input        */
}





/********************************************************/
/*							*/
/*	DSfini -- restore normal terminal i/o		*/
/*							*/
/********************************************************/


void DSfini()
{
   dispready = FALSE;

   TRwinset(0);

   if (PVAL(endclear)) TRclear();
    else
     { TRgoto(0,PVAL(termlength)-1);
       TRprint(PVAL(termlength)-1,"");
       TRdisp();
     };

   TRend();

   INdone();
}





/********************************************************/
/*							*/
/*	DSreset -- reinitialize display 		*/
/*							*/
/********************************************************/


void DSreset()
{
   register INT i,wd;

   if (PVAL(silent)) return;

   if (!dispready) DSinit();
      else {
	 wd = PVAL(curwindow);
	 if (wd == 1 || wd == 2)
	   {
	      PSET(curwindow,3);
	      TRclear();
	      if (*PSVAL(winmark) != 0)
		 for (i = 0; i < PVAL(curwidth); ++i)
		    TRins(i,0,PSVAL(winmark));
	      PSETI(curwindow,wd);
	   }
	 TRclear();
	 i = TRinfo("w");
	 if (PVAL(termwidth) != i) PSETI(termwidth,i);
	 i = TRinfo("l");
	 if (PVAL(termlength) != i) PSETI(termlength,i);
       };

   DSwrite();

   DSshow();
}





/********************************************************/
/*							*/
/*	DSlreset -- rewrite current line		*/
/*							*/
/********************************************************/


void DSlreset()
{
   register DATA lnum;

   lnum = PDVAL(curline)-PDVAL(curscreen);

   LBlinebuf[0] = 0;
   DSput(lnum);
   DSshow();

   DSline(PDVAL(curline));
}






/********************************************************/
/*							*/
/*	DSerror -- display an error message		*/
/*							*/
/********************************************************/


void DSerror(msg)
   STRING msg;
{
   register BOOL dlfg,blfg;

   if (PVAL(silent)) return;

   dlfg = TRUE;
   blfg = TRUE;

   while (*msg == '-')
    { switch (*++msg)
       { case 'd' : dlfg = !dlfg; break;
	 case 'b' : blfg = !blfg; break;
	 case 'k' : BUGCMD; break;
	 default  : ABORT;
       };
      ++msg;
    };

   if (*msg != 0) outmsg(msg);
   if (blfg) TRbells(PVAL(bells));
   if (dlfg) sleep(PVAL(errordelay));

   dispfinish();
}






/********************************************************/
/*							*/
/*	DSreply -- ask user a string type question	*/
/*							*/
/********************************************************/


void DSreply(msg,buf,sz,tbl)
   STRING msg;
   TEXT buf[];
   INT sz;
   TEXT *tbl;
{
   register INT ln,wd;

   if (!dispready) ABORT;

   wd = TRwinset(0);

   if (!PVAL(silent))
    { TRprint(PVAL(termlength)-1,msg);
      TRdisp();
      ln = strlen(msg);
      TRgoto(ln,PVAL(termlength)-1);
    };

   if ((PINT)(tbl) == -1) INline(buf,sz,NULL,TRUE);
      else INline(buf,sz,tbl,PVAL(silent));

   if (wd != 0) TRwinset(wd);

   BUGCMD;

   dispfinish();
}





/********************************************************/
/*							*/
/*	DSquery -- ask user a yes/no type question	*/
/*							*/
/********************************************************/


int DSquery(msg,rsps)
   STRING msg;
   TEXT * rsps;
{
   register INT ln,rs,wd;
   register TEXT ch;
   register TEXT * pt;

   if (!dispready) ABORT;
   wd = TRwinset(0);

   PSET(silent,FALSE);

   TRprint(PVAL(termlength)-1,msg);
   ln = strlen(msg);
   TRdisp();
   TRgoto(ln,PVAL(termlength)-1);

   ch = INchar();
   if (ch >= ' ' && ch < CM_FIRST) TRtype(ch,FALSE);

   rs = 0;
   pt = rsps;
   while (*pt != 0 && *pt != ch)
    { ++rs; ++pt; };

   if (*pt == 0) rs = -1;

   BUGCMD;

   if (wd != 0) TRwinset(wd);

   dispfinish();

   return rs;
}





/********************************************************/
/*							*/
/*	DSabort -- print message and abort		*/
/*							*/
/********************************************************/


void DSabort(msg)
   STRING msg;
{
   PSET(silent,FALSE);

   DSerror(msg);
   EDabort("System abort",FALSE);
}





/********************************************************/
/*							*/
/*	DSwrite -- rewrite whole screen 		*/
/*							*/
/********************************************************/


void DSwrite()
{
   if (!dispready) return;
   DSfrom(PDVAL(curscreen));
}





/********************************************************/
/*							*/
/*	DSfrom -- rewrite screen from line		*/
/*							*/
/********************************************************/


void DSfrom(line)
   LINE line;
{
   register DATA dloc,dlen;

   dloc = line-PDVAL(curscreen);
   dlen = PDVAL(curlength)-dloc;

   LBmove(LNget,line,0l,
	  DSput,dloc,dlen,
	  LBF_SPARM|LBF_SINCP|LBF_SIGN|LBF_DMAX|LBF_DPARM|LBF_DINCP|LBF_FMT0);

   if (PVAL(ateof)) eofdisplay(PVAL(ateof));
}





/********************************************************/
/*							*/
/*	DSline -- rewrite single line			*/
/*							*/
/********************************************************/


void DSline(line)
   LINE line;
{
   register DATA dloc,dlen;
   register INT ln;

   ln = line-PDVAL(curscreen);
   if (ln < 0 || ln >= PVAL(curlength)) return;

   dloc = ln;
   dlen = 1;

   LBmove(LNget,line,0l,
	  DSput,dloc,dlen,
	  LBF_SPARM|LBF_SINCP|LBF_SIGN|LBF_DMAX|LBF_DPARM|LBF_DINCP|LBF_FMT0);
}





/********************************************************/
/*							*/
/*	DSshow -- update display to show additions	*/
/*							*/
/********************************************************/


void DSshow()
{
   register INT fg;
   static DATA fsz;

   if (PVAL(silent)) return;

   fg = (PDVAL(filesize)+1-PDVAL(curscreen) < PVAL(curlength));
   if (PVAL(ateof) != fg)
    { PSETI(ateof,fg);
      fsz = PDVAL(filesize);
      eofdisplay(fg);
    }
   else if (fg && fsz != PDVAL(filesize) && PVAL(eofmark) == EOF_LINE)
    { fsz = PDVAL(filesize);
      DSwrite();
      eofdisplay(fg);
    };

   dispinfo();

   dispfinish();
}





/********************************************************/
/*							*/
/*	DSsystem -- do a system call			*/
/*							*/
/********************************************************/


void DSsystem(cmd,dspfg)
   STRING cmd;
   INT dspfg;
{
   /*register*/ CHAR in;	/* changed by mjb */
   register INT wd,pr,i;


   if (PVAL(priority) < 20) pr = 20-PVAL(priority);
      else pr = 0;

   wd = TRwinset(0);
   INdone();
   TRclear();
   TRend();
   dispready = FALSE;

#ifdef SIGSTOP
   if (strcmp(cmd,"*STOP*") == 0)
      kill(getpid(),SIGSTOP);
   else
#endif
    { if (dspfg) printf("$ %s\n",cmd);

      local_system(cmd,pr);
/*    TRgoto(0,PVAL(termlength)-1);	*/

      if (dspfg) {
	 printf("\nType return when ready");
	 fflush(stdout);
	 read(0,&in,1);
       }
    }

   DSinit();

   if (wd != 0) {
      TRwinset(3);
      TRclear();
      if (*PSVAL(winmark) != 0)
	 for (i = 0; i < PVAL(curwidth); ++i)
	 TRins(i,0,PSVAL(winmark));
    }

   PSETI(curwindow,wd);
}





/********************************************************/
/*							*/
/*	DSmark -- set marker on display 		*/
/*	DSunmark -- reset marker			*/
/*							*/
/********************************************************/


void DSmark()
{
   register INT mkid;

   mkid = ('d'<<8)+(nummarks++);
   LNmark(mkid,PDVAL(curline));
   TRcmark();
}





void DSunmark()
{
   register DATA line;
   register INT sln,mkid;

   if (nummarks <= 0) return;

   mkid = ('d'<<8)+(--nummarks);
   line = LNgetmark(mkid);
   LNunmark(mkid);
   sln = (line-PDVAL(curscreen));
   if (sln >= 0 && sln < PDVAL(curlength)) DSline(line);
}





/********************************************************/
/*							*/
/*	DSslowdisp -- set slow display size		*/
/*							*/
/********************************************************/


void DSslowdisp()
{
   register INT i,j;

   if (!PVAL(slow)) return;

   i = PVAL(brief);
   j = 1+TRinfo("d");

   while (i > j) j = 1+TRinfo("d");
   while (i < j) j = -1+TRinfo("-d");
}






/********************************************************/
/*							*/
/*	DSput -- put line onto the display		*/
/*							*/
/*	This routine is called indirectly through	*/
/*	LBmove to display the current line.		*/
/*							*/
/********************************************************/


static int
DSput(lnum)
   DATA lnum;
{
   register INT len,line;

   line = lnum;
   len = strlen((char *) LBlinebuf);

   if (len < PVAL(curmargin)) TRprint(line,"");
      else TRinsn(0,line,&LBlinebuf[PVAL(curmargin)],PVAL(curwidth));

   return TRUE;
}





/********************************************************/
/*							*/
/*	DStype -- type single charater			*/
/*							*/
/********************************************************/


void DStype(ch,insfg)
   INT ch;
   BOOL insfg;
{
   if (PVAL(silent)) return;

   if (ch == CHR_BSDEL) ch = CHR_BS;

   TRtype(ch,insfg);
}






/********************************************************/
/*							*/
/*	DSpos -- reset position only			*/
/*							*/
/********************************************************/


void DSpos()
{
   register INT x,y;

   if (PVAL(silent)) return;

   x = PVAL(curchar)-PVAL(curmargin);
   y = PDVAL(curline)-PDVAL(curscreen);

   TRgoto(x,y);
}






/********************************************************/
/*							*/
/*	outmsg -- write error message to user		*/
/*							*/
/********************************************************/

static void
outmsg(msg)
   STRING msg;
{
   register INT wd;

   if (dispready)
    { wd = TRwinset(0);
      TRprint(PVAL(termlength)-1,msg);
      TRdisp();
      TRgoto(strlen(msg),PVAL(termlength)-1);
      if (wd != 0) TRwinset(wd);
    }
   else
      printf("%s: %s\n",PSVAL(editor),msg);
}





/********************************************************/
/*							*/
/*	dispinfo -- write control line			*/
/*							*/
/********************************************************/


static void
dispinfo()
{
   static CHAR svstr[2*FNSIZE];
   CHAR numstr[32],flgstr[128],fnstr[256];
   register INT i,j,k,wd;
   CHAR * cs;

   for (i = 0; CLTBL[i].CLparm >= 0; ++i)	/* check if changes	*/
    { if (CLTBL[i].CLflags&CLFstring)
       { if (CLTBL[i].CLval == NULL ||
	     strcmp((STRING)(CLTBL[i].CLval),
		    (STRING)(PARMS[CLTBL[i].CLparm].PMvalue)) != 0)
	    break;
       }
      else if (CLTBL[i].CLval != PARMS[CLTBL[i].CLparm].PMvalue) break;
    };
   if (CLTBL[i].CLparm < 0) return;

   j = 0;					/* save new values	*/
   for (i = 0; CLTBL[i].CLparm >= 0; ++i)
    { if (CLTBL[i].CLflags & CLFstring)
       { CLTBL[i].CLval = (PARMVAL)(&svstr[j]);
	 strcpy(&svstr[j],(STRING)PARMS[CLTBL[i].CLparm].PMvalue);
	 j += strlen(&svstr[j])+1;
       }
      else CLTBL[i].CLval = PARMS[CLTBL[i].CLparm].PMvalue;
    };

   if (PVAL(curmargin) > 0)			/* get basic strings	*/
      sprintf(numstr,"%ld (%ld) / %ld",PDVAL(curscreen)+1,PVAL(curmargin),
		PDVAL(filesize));
    else
      sprintf(numstr,"%ld / %ld",PDVAL(curscreen)+1,PDVAL(filesize));
   strcpy(fnstr,PSVAL(filename));
   getflags(flgstr,FALSE);

   for (i = 0;					/* insure that it fits	*/
	(int)(strlen(fnstr)+strlen(numstr)+strlen(flgstr)+4) > PVAL(curwidth);
	++i)
      switch(i)
       { case 0 : getflags(flgstr,TRUE); break;
	 case 1 : sprintf(fnstr,".../%s",PSVAL(filetail)); break;
	 case 2 : strcpy(flgstr,"..."); break;
	 case 3 : numstr[0] = 0; break;
	 case 4 :
	    cs = PSVAL(filetail);
	    j = strlen(cs);
	    sprintf(fnstr,"...%s",&cs[j-16]);
	    break;
	 case 5 : fnstr[0] = 0; break;
	 case 6 : numstr[0] = 0; break;
	 case 7 : return;
       };

   j = PVAL(curwidth)-strlen(fnstr)-strlen(numstr)-strlen(flgstr);
   i = strlen(fnstr);				/* format line		*/
   if ((j&01) != 0)
    { fnstr[i++] = ' ';
      j--;
    };
   for (k = 0; k < j/2; ++k) fnstr[i++] = ' ';
   strcpy(&fnstr[i],flgstr);
   i = strlen(fnstr);
   for (k = 0; k < j/2; ++k) fnstr[i++] = ' ';
   strcpy(&fnstr[i],numstr);

   wd = TRwinset(0);
   TRprint(PVAL(termlength)-1,fnstr);
   if (wd != 0) TRwinset(wd);
}





/********************************************************/
/*							*/
/*	getflags -- construct flag string for cmd line	*/
/*							*/
/********************************************************/


static void
getflags(str,shortfg)
   STRING str;
   INT shortfg;
{
   register INT i,j,frst;
   CHAR buf[16];

   frst = TRUE;
   str[0] = 0;

   for (i = 0; CLTBL[i].CLparm >= 0; ++i)
      if (!(CLTBL[i].CLflags & CLFign))
       { j = 0;
	 if (CLTBL[i].CLflags&CLFstring)
	  { if (*((STRING)(CLTBL[i].CLval)) != 0) j = 1; }
	  else if (CLTBL[i].CLflags&CLFnumber && CLTBL[i].CLval != 0) j = 2;
	  else if (CLTBL[i].CLval) j = 3;
	 if (j != 0)
	  { if (!frst) strcat(str," ");
	    frst = FALSE;
	    strcat(str,shortfg ? CLTBL[i].CLshort : CLTBL[i].CLname);
	    switch(j)
	     { case 1  : strcat(str,(STRING)CLTBL[i].CLval);
			 break;
	       case 2  : sprintf(buf,"%ld",(long) CLTBL[i].CLval);
			 strcat(str,buf);
			 break;
	       default : break;
	     };
	  };
       };
}






/********************************************************/
/*							*/
/*	eofdisplay -- display end of file marker	*/
/*							*/
/********************************************************/


static void
eofdisplay(fg)
   INT fg;
{
   register INT ln;
   register STRING eofmsg;

   if (PVAL(eofmark) == 0) return;
   eofmsg = "[END OF FILE]";

   ln = (PVAL(eofmark) == EOF_PAGE) ? PVAL(curlength)-1 :
				      PDVAL(filesize)+1-PDVAL(curscreen);
   if (ln >= PVAL(curlength)) fg = FALSE;

   if (fg)
    { TRprint(ln,eofmsg);
      TRmark(0,ln,strlen(eofmsg)-1,ln,TRUE);
    }
   else
    { PSET(ateof,FALSE);
      if (PVAL(eofmark) == EOF_PAGE) DSline(PDVAL(curscreen)+ln);
	 else DSwrite();
    };
}





/********************************************************/
/*							*/
/*	dispfinish -- actually update screen		*/
/*							*/
/********************************************************/


static void
dispfinish()
{
   register INT x,y;

   if (!dispready) return;
   if (PVAL(silent)) return;

   x = PVAL(curchar)-PVAL(curmargin);
   y = PDVAL(curline)-PDVAL(curscreen);

   TRdisp_goto(x,y);
}





/********************************************************/
/*							*/
/*	INCLEAN -- user requested abort 		*/
/*							*/
/********************************************************/


void INCLEAN()
{
   EDabort("User abort",TRUE);
}





/********************************************************/
/*							*/
/*	INVERIFY -- user wants signal verified		*/
/*							*/
/********************************************************/


int INVERIFY(id)
   INT id;
{
   register INT rs;
   register STRING msg;
   static TEXT rsps[] = {
		CM_EXIT , 'y' , 'Y' ,'n' , 'N' , 'a' , 'A' , '\32' , 0
		 };
   static TEXT slowrsps[] = { CM_EXIT, 'Y', 0 };

   msg = PVAL(srching) ? "Interrupt.  Stop search (Y or N)? " :
	 PVAL(douse)   ? "Interrupt.  Stop do (Y or N)? "     :
			 "Interrupt.  Abort (Y or N)? "       ;

   rs = DSquery(msg,(PVAL(slow) ? slowrsps : rsps));

   switch(rs)
    { case 1 :			/* yY */
      case 2 :
      case 5 :
      case 6 :
		if (PVAL(srching)) PSET(srching,FALSE);
		   else if (PVAL(douse)) DOstop();
		   else return TRUE;
		break;

      case 0 :			/* ^z */
      case 7 :
		EDabort("Interrupt abort",MAYBE);

      default : 		/* nN... */
		break;
    };

   dispinfo();
   dispfinish();

   return FALSE;
}






/********************************************************/
/*							*/
/*	ILLCHR -- produce message for illegal input	*/
/*							*/
/********************************************************/


void ILLCHR()
{
   DSerror("-dIllegal escape sequence");
}





/* end of dispmod.c */
