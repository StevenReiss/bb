/********************************************************/
/*							*/
/*		PARMS.C 				*/
/*							*/
/*	This file contains the code for managing both	*/
/*   parameter setting and usage as well as the editor	*/
/*   control file, and parameter display.		*/
/*							*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "bufmod.h"
#include "parms.h"
#include "parmdef.h"
#include "filemod.h"
#include "linemod.h"
#include "langmod.h"
#include <stdio.h>



/********************************************************/
/*							*/
/*	External Routines				*/
/*							*/
/********************************************************/


extern	CHAR	INscan();	/* translate input char */
extern	void	DSinit();
extern	void	DSslowdisp();
extern	int	TRwinset();
extern	void	TRclear();
extern	void	TRins();
extern	int	TRwindow();

extern	void	null_strncpy();
extern	int	mesg();



/********************************************************/
/*							*/
/*	Data type definitions				*/
/*							*/
/********************************************************/


typedef enum {
	PSR_SAVE,		/* save old file	*/
	PSR_RSTR,		/* restore previous file*/
	PSR_RCVR		/* recover last file	*/
} PSRTYPE;






/********************************************************/
/*							*/
/*	Variable storage				*/
/*							*/
/********************************************************/


#define PARMWDSZ 256		/* max word size of parm input */


static	BOOL	startfg = FALSE;/* indicates not first parm */
static	BOOL	fileset;	/* file change rqd	*/
static	BOOL	filereset;	/* file reset rqd	*/
static	BOOL	dispset;	/* reset display rqd	*/
static	BOOL	dispinit;	/* display initial required */
static	BOOL	havefile;	/* file was given	*/
static	BOOL	firsttime = TRUE;/* indicates initialization of all */
static	BOOL	flipfile = FALSE;/* currently exchanging files	*/





/********************************************************/
/*							*/
/*	Internal Routines				*/
/*							*/
/********************************************************/


#define VAL(id)      PARMS[id].PMvalue
#define NUMVAL(id)   ((PINT)(PARMS[id].PMvalue))
#define DATVAL(id)   ((DATA)(PARMS[id].PMvalue))
#define STRVAL(id)   ((STRING)(PARMS[id].PMvalue))

#define DFLT(id)     PARMS[id].PMdflt
#define NUMDFLT(id)  ((PINT)(PARMS[id].PMdflt))
#define DATDFLT(id)  ((DATA)(PARMS[id].PMdflt))
#define STRDFLT(id)  ((STRING)(PARMS[id].PMdflt))

#define SDFLT(id)    PARMS[id].PMsdflt
#define NUMSDFLT(id) ((PINT)(PARMS[id].PMsdflt))
#define DATSDFLT(id) ((DATA)(PARMS[id].PMsdflt))
#define STRSDFLT(id) ((STRING)(PARMS[id].PMsdflt))


#define ISTERM(c) (strchr(" \t\n",c) != 0)

static	STRING	wildcard();
static	void	parmstart();
static	void	parmend();
static	int	parmdoset();
static	int	parmchk();
static	void	filedoset();
static	void	filedoreset();
static	void	openfile();
static	int	closefile();
static	void	saverstr();
static	void	outparm();
static	int	setstr();
static	int	setnum();
static	int	cvtint();
static	int	setflag();
static	int	cvtflag();
static	int	setbufp();
static	int	cvtbuf();
static	int	setbfct();
static	DATA	cvtbfct();
static	int	PRflname();
static	int	PRdirchk();
static	int	PRdisplay();
static	int	PRscrchk();
static	int	PRwindow();
static	int	PRmesg();
static	int	PRlanguage();
static	int	PRchange();
static	int	PRbuffer();





/********************************************************   ENTRY ROUTINE */
/*							*/
/*	Pinit -- initialize parameter module		*/
/*							*/
/********************************************************/


void Pinit()
{
   register INT i;

   parmstart();

   for (i = 0; i < NUMPARM; ++i)	/* reset all parms	*/
    { if (PARMS[i].PMflags & PMFenv)
       { PARMS[i].PMflags &= ~PMFenv;
	 PARMS[i].PMdflt = (PARMVAL) getenv(PARMS[i].PMdflt);
	 if (PARMS[i].PMdflt == 0) PARMS[i].PMdflt = "";        /* ddj */
       };
      if (!(PARMS[i].PMflags & PMFninit)) parmdoset(i,"",TRUE);
      PARMS[i].PMflags &= ~PMFset;
    };
}





/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Parm -- entry to set a parameter		*/
/*							*/
/********************************************************/


void Parm(text,multfg)
   TEXT *text;			/* parameter string	*/
   BOOL multfg; 		/* multiple parameters flag */
{
   CHAR word[PARMWDSZ],eqpart[PARMWDSZ];
   CHAR emsg[128];
   register STRING wpt;
   register CHAR ch;
   register INT i,pfg,efg,ln;

   parmstart();

   while(text != NULL)
    { pfg = efg = FALSE;
      wpt = word;
      word[0] = 0;
      eqpart[0] = 0;
      while (*text != 0 && ISTERM(*text)) ++text;

      if (*text == '-') { pfg = TRUE; ++text; };

      for (i = 0; i < PARMWDSZ-1; ++i)
       { ch = *text++;
	 if (ch == '\\') { --text; *wpt++ = INscan(&text); }
	    else if (ch == 0) { text = NULL; break; }
	    else if (ch == '\n') break;
	    else if (multfg && ISTERM(ch)) break;
	    else if (ch == '=' && !efg)
	     { *wpt = 0;
	       wpt = eqpart;
	       pfg = efg = TRUE;
	     }
	    else *wpt++ = ch;
       };

      if (word[0] == 0 && eqpart[0] == 0 && text == NULL) break;

      if (i >= PARMWDSZ-1)
       { DSerror("Parameter too long");
	 text = NULL;
       };

      *wpt = 0;
      if (word[0] == 0 && eqpart[0] != 0)
	 DSerror("Illegal parameter string");
       else if (word[0] == 0) continue;

      if (pfg)
       { ln = strlen(word);
	 for (i = 0; i < NUMPARM; ++i)
	    if (ln >= PARMS[i].PMnmln && PARMS[i].PMnmln > 0 &&
		  strncmp(word,PARMS[i].PMname,ln) == 0)
	       break;
	 if (i >= NUMPARM || PARMS[i].PMflags&PMFnouser)
	  { sprintf(emsg,"Parameter '%s' not found",word);
	    DSerror(emsg);
	  }
	 else
	  { if (PARMS[i].PMfluse >= 0 && PARMS[i].PMflags&PMFdelay)
	     { parmdoset(i,eqpart,efg);
	       i = PARMS[i].PMfluse;
	     };
	    parmdoset(i,eqpart,efg);
	  };
       }
      else if (!havefile)
       { havefile = TRUE;
	 parmdoset(P_nextfile,word,TRUE);
       }
      else
       { if ((int)(strlen(PSVAL(afterfile))+strlen(word)+1) <= 10240)
	  { strcat(PSVAL(afterfile),word);
	    strcat(PSVAL(afterfile)," ");
	  }
	 else DSerror("Too many subsequent files");
       };
    };
}





/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Pgetctrl -- read in control area		*/
/*							*/
/********************************************************/


void Pgetctrl()
{
   register STRING dir;
   CHAR   ctrlname[FNSIZE],xnline[PMXSIZE+16];
   register FILE *fid;
   register INT i,j;
   CHAR buf[BUFSIZ];

   dir = (STRING) (PVAL(localcontrol) ? "." : PSVAL(home));
   sprintf(ctrlname,"%s/bCONTROL",dir);

   fid = fopen(ctrlname,"r");
   if (fid != NULL)
    { setbuf(fid,buf);
      while (fgets(xnline,PMXSIZE+16,fid) != NULL) Parm(xnline,FALSE);
      fclose(fid);
      PSET(nextfile,"");
      for (i = 0; i < NUMPARM; ++i)
       { if ((j = PARMS[i].PMfluse) >= 0 &&
		  PARMS[i].PMflags&PMFdelay) {
	    if (PARMS[j].PMflags&PMFset) Pset(i,VAL(j));
	    PARMS[j].PMflags &= ~PMFset;
	  };
       };
      fileset = FALSE;
      filereset = TRUE;
    }
   else
    { for (i = 0; i < NUMPARM; ++i)
       { if ((j = PARMS[i].PMfluse) >= 0 &&
		  PARMS[i].PMflags&PMFdelay) {
	    if (PARMS[j].PMflags&PMFset) Pset(i,VAL(j));
	    PARMS[j].PMflags &= ~PMFset;
	  };
       };
    };

   PSET(Pfileid,1);
   PSET(Qfileid,2);
   PSET(Xfileid,3);
}





/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Psetctrl -- output the control area		*/
/*							*/
/********************************************************/


void Psetctrl()
{
   register STRING dir;
   CHAR   ctrlname[FNSIZE];
   CHAR buf[BUFSIZ];
   register INT i;
   register FILE *fid;

   if (PVAL(nocontrol)) return;

   dir = (STRING)(PVAL(localcontrol) ? "." : PSVAL(home));
   sprintf(ctrlname,"%s/bCONTROL",dir);

   fid = fopen(ctrlname,"w");
   if (fid == NULL)
    { DSerror("Can't write control file");
      return;
    };
   setbuf(fid,buf);

   for (i = 0; i < NUMPARM; ++i)
      if (PARMS[i].PMflags & (PMFctrl|PMFfprop))
	 outparm(i,fid);

   fclose(fid);
}





/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Parmdo -- Actually use parameter settings	*/
/*							*/
/********************************************************/

void Parmdo()
{
   if (startfg) parmend();

   firsttime = FALSE;
}





/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Pset -- set parameter with internal value	*/
/*							*/
/********************************************************/


int Pset(parm,val)
   INT parm;			/* to parameter 	*/
   PARMVAL val; 		/* actual value 	*/
{
   if (!startfg) parmstart();

   switch(PARMS[parm].PMtype)
    { case PMTYSTR : if (!(PARMS[parm].PMflags&PMFnhist))
			HSparmstr(parm,STRVAL(parm));
		     our_strcpy(STRVAL(parm),(STRING) val);
		     break;
      default	   : if (!(PARMS[parm].PMflags&PMFnhist))
			HSparm(parm,VAL(parm));
		     VAL(parm) = val;
		     break;
    };

   return parmchk(parm);
}





/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Pcmdarg -- handle command argument parameter	*/
/*							*/
/********************************************************/


int Pcmdarg(parm,permfg,argfg,arg,val)
   INT parm;			/* parameter id number	*/
   INT permfg;			/* permanent flag	*/
   INT argfg;			/* argument given flag	*/
   STRING arg;			/* actual argument	*/
   PARMVAL *val;		/* resultant value	*/
{
   CHAR oldstr[PMXSIZE];
   register PARMVAL oldval;

   if ((PARMS[parm].PMflags & PMFcmd) == 0) ABORT;

   if (PARMS[parm].PMtype == PMTYSTR)
    { if (!permfg && argfg && *arg != 0)
       { *val = arg;
	 return TRUE;
       };
      our_strcpy(oldstr,STRVAL(parm));
      oldval = (PARMVAL) oldstr;
    }
   else oldval = VAL(parm);

   if (!parmdoset(parm,arg,argfg)) return FALSE;

   *val = VAL(parm);

   if (!permfg) Pset(parm,oldval);

   return TRUE;
}






/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Pwriteall -- write all parameters that should	*/
/*	be written (i.e. no PMFndisp flag) to the	*/
/*	given file.					*/
/*							*/
/********************************************************/


void Pwriteall(fid)
   FILE *fid;
{
   register INT i;

   for (i = 0; i < NUMPARM; ++i)
      if (!(PARMS[i].PMflags&PMFndisp) && PARMS[i].PMnmln > 0)
	 outparm(i,fid);
}






/*******************************************************   ENTRY ROUTINE */
/*							*/
/*	Pfini -- finish up with parameters;  This	*/
/*	routine closes the current file and saves	*/
/*	information about it and current parameter	*/
/*	settings in the control file.			*/
/*							*/
/********************************************************/


int Pfini()
{
   PRmesg(-1);

   if (! closefile()) return FALSE;

   Psetctrl();

   return TRUE;
}





/*******************************************************   General routine */
/*							*/
/*	parmstart -- start of parameter string		*/
/*							*/
/********************************************************/


static void
parmstart()
{
   if (startfg) return;

   startfg = TRUE;
   filereset = dispset = havefile = FALSE;
   fileset = dispinit = firsttime;
}





/*******************************************************   General routine */
/*							*/
/*	parmend -- end of parameter string		*/
/*							*/
/********************************************************/


static void
parmend()
{
   if (!startfg) return;

   if (dispinit)
    { if (!firsttime) DSfini();
      DSinit();
    };

   if (fileset)
    { filedoset();
      filereset = FALSE;
    };

   if (filereset)
    { filedoreset();
    };

   if (dispset) DSwrite();

   startfg = FALSE;
}





/*******************************************************   General routines */
/*							*/
/*	parmdoset -- actually set parameter value	*/
/*							*/
/********************************************************/


static INT
parmdoset(pm,eqfld,eqfg)
   INT pm;			/* parameter id 	*/
   STRING eqfld;		/* value to set 	*/
   INT eqfg;			/* value present flag	*/
{
   register INT fg;

   switch (PARMS[pm].PMtype)
    { case PMTYSTR   : fg = setstr(pm,eqfld,eqfg); break;
      case PMTYNUM   : fg = setnum(pm,eqfld,eqfg); break;
      case PMTYFLAG  : fg = setflag(pm,eqfld,eqfg); break;
      case PMTYBUF   : fg = setbufp(pm,eqfld,eqfg); break;
      case PMTYBFCT  : fg = setbfct(pm,eqfld,eqfg); break;
      default	     : ABORT;
    };

   if (fg) fg = parmchk(pm);

   return fg;
}





/*******************************************************   General routine */
/*							*/
/*	parmchk -- general parameter checks		*/
/*							*/
/********************************************************/


static int
parmchk(pm)
   INT pm;			/* parameter id 	*/
{
   PARMS[pm].PMflags |= PMFset;

   if (PARMS[pm].PMflags&PMFfile) fileset = TRUE;
   if (PARMS[pm].PMflags&(PMFreset|PMFdelay)) filereset = TRUE;
   if (PARMS[pm].PMflags&PMFdisp) dispset = TRUE;
   if (PARMS[pm].PMflags&PMFdinit) dispinit = TRUE;

   if (PARMS[pm].PMcall != NULL)
      return (*PARMS[pm].PMcall)(pm);
    else return TRUE;
}





/*******************************************************   File routines */
/*							*/
/*	filedoset -- set up a new file			*/
/*							*/
/********************************************************/


static void
filedoset()
{
   register INT i,lastwindow,wd;
   static int fileid = 10;

   if (!firsttime && !closefile())
    { dispset = TRUE;
      return;
    };
   firsttime = FALSE;
   PRwindow(P_window);		/* initialize windows	*/
   lastwindow = PVAL(curwindow);

   if (*PSVAL(nextfile) == 0) saverstr(PSR_RSTR);
    else
     { saverstr(PSR_SAVE);
       PSET(filename,PSVAL(nextfile));
       PSET(nextfile,"");
       for (i = 0; i < NUMPARM; ++i)
	  if (PARMS[i].PMflags&PMFfldflt)
	   { Pset(i,DFLT(i));
	     PARMS[i].PMflags &= ~PMFset;
	   };
       PSETI(fileid,fileid);
       fileid++;
     };

   openfile();
   HSclear();			/* remove if possible	*/

   if (PVAL(window) <= 1 && PVAL(curwindow) != 0) PSET(curwindow,0);
   if (PVAL(window) == 2)
    { wd = TRwinset(3);
      if (wd == 0)
       { TRclear();
	 if (*PSVAL(winmark) != 0)
	    for (i = 0; i < PVAL(curwidth); ++i)
	       TRins(i,0,PSVAL(winmark));
       }
      if (lastwindow == 0) PSET(curwindow,2);
      else PSET(curwindow,3-lastwindow);
    }
}





/*******************************************************   File routines */
/*							*/
/*	filedoreset -- reopen current file		*/
/*							*/
/********************************************************/


static void
filedoreset()
{
   register INT fg;

   if (!firsttime && !PVAL(nochange) && PVAL(savequery))
    { fg = DSquery("Restart or save? ","rR");
      if (fg >= 0) PSET(nochange,TRUE);
    };

   PSET(savequery,TRUE);

   if (!firsttime && !closefile())
    { dispset = TRUE;
      return;
    };

   firsttime = FALSE;

   openfile();

   HSclear();		/* remove if possible		*/
}





/*******************************************************   File routines */
/*							*/
/*	openfile -- actually open the requested file	*/
/*							*/
/********************************************************/


static void
openfile()	 /* open the current file	 */
{
   register INT i,j;
   register FLSTATUS sts;
   CHAR nam[FNSIZE];
   register STRING pt;
   register BOOL lngfg;

   lngfg = (PVAL(autolang) && !(PARMS[P_language].PMflags&PMFset));

   dispset = TRUE;
   for (i = 0; i < NUMPARM; ++i)       /* reset parms  */
      if ((j = PARMS[i].PMfluse) >= 0 && PARMS[i].PMflags&(PMFdelay|PMFfdset))
       { if (PARMS[j].PMflags&PMFset) Pset(i,VAL(j));
	    else Pset(j,VAL(i));
	 PARMS[j].PMflags &= ~PMFset;
       };

   PSET(nochange,TRUE);

   do					/* open file	*/
    { sts = FLinit(PVAL(newfile));
      switch(sts)
       { case FL_OK	: break;
	 case FL_NEW	: i = DSquery("File doesn't exist. Create (yes, no, rename or abort)? ",
					(TEXT *) "ynra");
			  switch(i)
			   { case 0  : PSET(newfile,TRUE);
				       break;
			     case 1  : sts = FL_FAIL;
				       break;
			     default :
			     case 2  : nam[0] = 0;
				       DSreply("Enter file name: ",
						   nam,FNSIZE,NULL);
				       i = strlen(nam);
				       nam[--i] = 0;
				       if (i == 0) sts = FL_FAIL;
					  else parmdoset(P_filename,nam,TRUE);
				       break;
			     case 3  : EDabort("No file",TRUE);
			   };
			  break;
	 case FL_BAD	: DSerror("Can't edit a non-text file or directory");
			  sts = FL_FAIL;
			  break;
	 case FL_LOCK	: DSerror("File is currently being edited");
			  sts = FL_FAIL;
			  break;
	 case FL_FAIL	: DSerror("Can't open file");
			  break;
	 case FL_READ	: if (!PVAL(curreadonly))
			   { DSerror("-bAccess is read-only");
			     PARMS[P_curreadonly].PMvalue = (PARMVAL) TRUE;
			     PARMS[P_readonly].PMvalue = (PARMVAL) TRUE;
			   };
			  sts = FL_OK;
			  break;
       };
      if (sts == FL_FAIL)
       { saverstr(PSR_RCVR);
	 sts = FL_NEW;
       };
    }
   while (sts != FL_OK);

   if (lngfg && (pt = LAauto()) != NULL)
    { PSET(curlanguage,pt);
      PSET(language,pt);
      PARMS[P_language].PMflags &= ~PMFset;
    };

   LNinit();				/* set up lines */
}





/*******************************************************   File routines */
/*							*/
/*	closefile -- close current file 		*/
/*							*/
/********************************************************/


static INT
closefile()
{
   if (PVAL(curreadonly)||PVAL(nochange))
    { LNclose(FALSE);
    }
   else
    { if (FLoutput() != FL_OK)
       { DSerror("Can't open output file");
	 return FALSE;
       };
      LNclose(TRUE);
    };

   FLclose();

   return TRUE;
}





/*******************************************************   Save/rstr routines */
/*							*/
/*	saverstr -- save/restore current file parms	*/
/*							*/
/********************************************************/


static void
saverstr(fg)
   PSRTYPE fg;
{
   register INT tyfg,i,cs,use;
   register INT a,b,c,d;
   static CHAR lastsave;
   CHAR temp[PMXSIZE];
   register PARMVAL val;

   tyfg = PVAL(alternate) ? PMFaltf : PMFprev;

   flipfile = TRUE;

   switch(fg)
    { case PSR_SAVE	:
		cs = 0;
		lastsave = 1;
		break;

      case PSR_RSTR	:
		cs = 1;
		lastsave = 0;
		break;

      case PSR_RCVR	:
		if (lastsave == 1) cs = 2;
		 else if (lastsave == 0) cs = 1;
		 else cs = 3;
		lastsave = -1;
		break;
    };

   for (i = 0; i < NUMPARM; ++i)	/* a to b to c	*/
      if (PARMS[i].PMflags&tyfg)
       { use = PARMS[i].PMfluse;
	 for (d = 0; d < NUMPARM; ++d)
	    if (PARMS[d].PMflags&PMFstkd && PARMS[d].PMfluse == use) break;
	 if (d >= NUMPARM) d = -1;

	 b = i;
	 switch(cs)
	  { case 0 :			/* SAVE 	*/
		a = use;
		c = d;
		break;

	    case 1 :			/* LAST FILE	*/
		a = c = use;
		break;

	    case 2 :			/* Recover stacked */
		a = d;
		c = use;
		break;

	    case 3 :			/* recover default */
		c = use;
		a = -1;
		break;
	  };
	 if (PARMS[b].PMtype == PMTYSTR)
	  { our_strcpy(temp,STRVAL(b)); val = (PARMVAL) temp; }
	  else val = VAL(b);
	 if (a >= 0) Pset(b,VAL(a));
	    else Pset(b,DFLT(b));
	 if (c >= 0) Pset(c,val);
       };

   flipfile = FALSE;
   PRscrchk(P_curline);
   PRscrchk(P_curchar);
}







/*******************************************************   Save/rstr routines */
/*							*/
/*	outparm -- save parameter in file		*/
/*							*/
/********************************************************/


static void
outparm(pm,otf)
   INT pm;
   FILE *otf;
{
   register STRING pt;
   CHAR buf[32];
   register INT bno;

   pt = buf;
   switch(PARMS[pm].PMtype)
    { case PMTYSTR	:
		pt = STRVAL(pm);
		break;

      case PMTYNUM	:
      case PMTYFLAG	:
		sprintf(buf,"%ld",NUMVAL(pm));
		break;

      case PMTYBUF	:
		if (NUMVAL(pm) < 0 || NUMVAL(pm) == BADBUF) return;
		buf[0] = 'a'+NUMVAL(pm);
		buf[1] = 0;
		break;

      case PMTYBFCT	:
		bno = BFCBUF(VAL(pm));
		if (bno == BADBUF || BFCCOUNT(VAL(pm)) <= 0) return;
		sprintf(buf,"%c%d",'a'+bno,BFCCOUNT(VAL(pm)));
		break;

      default		:
		pt = "";
		break;
    };

   fprintf(otf,"%s=%s\n",PARMS[pm].PMname,pt);
}






/*******************************************************   Parm values */
/*							*/
/*	string parameter routines			*/
/*							*/
/********************************************************/


static INT
setstr(pm,eq,eqfg)	 /* set string parameter */
   INT pm;			/*    parameter id	*/
   STRING eq;			/*    value		*/
   INT eqfg;			/*    value flag	*/
{
   register int ln;

   if (!(PARMS[pm].PMflags&PMFnhist)) HSparmstr(pm,STRVAL(pm));

   if (eqfg && *eq == 0) our_strcpy(STRVAL(pm),STRDFLT(pm));
   else if (eqfg)
    { ln = strlen(eq);
      if (ln >= PARMS[pm].PMsize)
       { DSerror("Parameter string truncated");
	 ln = PARMS[pm].PMsize-1;
       };
      null_strncpy(STRVAL(pm),eq,ln);
    }
   else our_strcpy(STRVAL(pm),STRSDFLT(pm));

   return TRUE;
}





/*******************************************************   Parm values */
/*							*/
/*	numeric parameter routines			*/
/*							*/
/********************************************************/


static INT
setnum(pm,eq,eqfg)	 /* set numeric parm	 */
   INT pm;			/*    parameter id	*/
   STRING eq;			/*    parameter value	*/
   INT eqfg;			/*    value flag	*/
{
   register INT val;

   val = eqfg && *eq == 0 ? NUMDFLT(pm) :
	 eqfg		  ? cvtint(eq) :
			    NUMSDFLT(pm);

   if (val < 0 || val < PARMS[pm].PMlbd || val > PARMS[pm].PMubd)
    { DSerror("Illegal numeric parameter");
      return FALSE;
    };

   if (!(PARMS[pm].PMflags&PMFnhist)) HSparm(pm,VAL(pm));

   VAL(pm) = (PARMVAL)((PINT) val);

   return TRUE;
}





static int
cvtint(str)	     /* convert str to int   */
   STRING str;
{
   register INT val,ch;

   val = 0;
   while ((ch = *str++) != 0)
    { if (isdigit(ch)) val = val*10+(ch-'0');
	 else return -1;
    };

   return val;
}





/*******************************************************   Parm values */
/*							*/
/*	flag parameter routines 			*/
/*							*/
/********************************************************/


static INT
setflag(pm,eq,eqfg)		/* set flag parameter	*/
   INT pm;			/*    parameter id	*/
   STRING eq;			/*    flag value	*/
   INT eqfg;			/*    value present fg	*/
{
   register INT val;

   val = eqfg && *eq == 0	       ? NUMDFLT(pm) :
	 eqfg			       ? cvtflag(eq) :
	 (PARMS[pm].PMflags&PMFnoflip) ? TRUE :
					 ! NUMVAL(pm);

   if (val < 0)
    { DSerror("Illegal flag parameter");
      return FALSE;
    };

   if (!(PARMS[pm].PMflags&PMFnhist)) HSparm(pm,VAL(pm));

   VAL(pm) = (PARMVAL)((PINT) val);

   return TRUE;
}





static int
cvtflag(str)	     /* scan flag value      */
   STRING str;
{
   return
      strchr("yYtT1",*str) != 0 ? TRUE :
      strchr("nNfF0",*str) != 0 ? FALSE :
				 -1;
}





/*******************************************************   Parm values */
/*							*/
/*	buffer id parameter routines			*/
/*							*/
/********************************************************/


static INT
setbufp(pm,eq,eqfg)	  /* buffer id set	  */
   INT pm;			/*    parameter id	*/
   STRING eq;			/*    buffer name	*/
   INT eqfg;			/*    name present	*/
{
   register INT val;

   val = eqfg && *eq == 0 ? NUMDFLT(pm) :
	 !eqfg		  ? NUMSDFLT(pm) :
	 strlen(eq) == 1  ? cvtbuf(*eq) :
			    -1;

   if (val < 0 || val < PARMS[pm].PMlbd || val > PARMS[pm].PMubd)
    { DSerror ("Illegal buffer identifier");
      return FALSE;
    };

   if (!(PARMS[pm].PMflags&PMFnhist)) HSparm(pm,VAL(pm));

   VAL(pm) = (PARMVAL)((PINT) val);

   return TRUE;
}





static int
cvtbuf(ch)	     /* get buffer id	     */
   CHAR ch;
{
   return
      isupper(ch) ? ch-'A' :
      islower(ch) ? ch-'a' :
		    -1	   ;
}





/*******************************************************   Parm values */
/*							*/
/*	buffer/count parameter routines 		*/
/*							*/
/********************************************************/


#define BADVAL 0x7fffff

static INT
setbfct(pm,eq,eqfg)	 /* set buffer/count parm*/
   INT pm;			/*    parameter id	*/
   STRING eq;			/*    set string	*/
   INT eqfg;			/*    string given flag */
{
   register INT val,ct,bf;

   val = eqfg && *eq == 0 ? NUMDFLT(pm)  :
	 ! eqfg 	  ? NUMSDFLT(pm) :
			    cvtbfct(eq)  ;

   if (val < 0 && eqfg && *eq != 0)
    { DSerror("Illegal buffer identifier or count");
      return FALSE;
    };

   if ((ct = BFCCOUNT(val)) == BADVAL) ct = BFCCOUNT(NUMVAL(pm));
   if ((bf = BFCBUF(val)) == BADBUF) bf = BFCBUF(NUMVAL(pm));

   if (!(PARMS[pm].PMflags&PMFnhist)) HSparm(pm,VAL(pm));

   VAL(pm) = (PARMVAL) BFCMAKE(bf,ct);

   return TRUE;
}





static DATA
cvtbfct(str)	     /* get buf/count value  */
   STRING str;			/*    source string	*/
{
   register INT val,ch,buf,vlct;

   buf = BADBUF;
   val = 0;
   vlct = 0;

   while ((ch = *str++) != 0)
    { if (isdigit(ch))
       { val = val*10+(ch-'0');
	 vlct++;
       }
      else if (isupper(ch) && buf == BADBUF) buf = ch-'A';
      else if (islower(ch) && buf == BADBUF) buf = ch-'a';
      else if ((ch == '@' || ch == '`') && buf == BADBUF) buf = BADBUF;
      else return -1;
    };

   if (vlct == 0) val = BADVAL;
   if (val == BADVAL && buf == BADBUF) return -1;

   return BFCMAKE(buf,val);
}





/*******************************************************   Postprocessing */
/*							*/
/*	PRflname -- new filename post processing	*/
/*							*/
/********************************************************/


static INT
PRflname(pm)
   INT pm;		/* either directly or via a new */
{			/* directory or tail		*/
   register STRING pt;
   register INT ln;
   CHAR buf[FNSIZE*2];

   if (pm != P_filename)
    { if (pm == P_directory)	/*    as directory	*/
       { ln = strlen(PSVAL(directory));
	 if (ln == 0) return FALSE;
	 if (ln == 0) our_strcpy(PSVAL(directory),".");
	    else if (PSVAL(directory)[ln-1] == '/')
		    PSVAL(directory)[ln-1] = 0;
       }
      else			/*    as file tail	*/
       { if (strlen(PSVAL(filetail)) == 0) return FALSE;
	 if (strchr(PSVAL(filetail),'/') != NULL)
	  { DSerror("Illegal file tail");
	    return FALSE;
	  };
       };
      our_strcpy(PSVAL(filename),PSVAL(directory));
      strcat(PSVAL(filename),"/");
      strcat(PSVAL(filename),PSVAL(filetail));
    }
   else if (strlen(PSVAL(filename)) == 0) return FALSE;

   pt = strrchr(PSVAL(filename),'/');
   if (pt != NULL)
    { ++pt;
      ln = ((long) pt) - ((long) PSVAL(filename))-1;
      null_strncpy(PSVAL(directory),PSVAL(filename),ln);
    }
   else
    { pt = PSVAL(filename);
      our_strcpy(PSVAL(directory),".");
    };
   our_strcpy(PSVAL(filetail),pt);

   if ((pt = wildcard(PSVAL(filename),buf)) != NULL)
    { PSET(filename,pt);
      return TRUE;
    };

   if (access(PSVAL(directory),1) < 0 && havefile)
    { DSerror("Bad output directory.  Reset before using");
      return FALSE;
    };

   return TRUE;
}





/*******************************************************   Postprocessing */
/*							*/
/*	wildcard -- convert wildcard file name		*/
/*							*/
/********************************************************/


static STRING
wildcard(name,buf)
   STRING name;
   STRING buf;
{
   register CHAR ch;
   register STRING pt;
   register BOOL fg;
   CHAR iobuf[BUFSIZ];
   CHAR ermsg[FNSIZE*2];
   register FILE *pid;
   FILE * local_popen();

   fg = FALSE;
   pt = name;
   if (*name == '~') fg = TRUE;
   else while ((ch = *pt++) != 0 && !fg)
      if (ch == '\\' && *pt != 0) ++pt;
	 else if (strchr("*?[",ch) != NULL) fg = TRUE;
   if (!fg) return NULL;

   sprintf(buf,"(set nonomatch; echo %s)",name);
   pid = (FILE *) local_popen(buf,"r");
   if (pid == NULL) ABORT;
   setbuf(pid,iobuf);
   if (fgets(buf,FNSIZE,pid) == NULL) ABORT;
   if (strncmp(buf,"You have mail",13) == 0 ||
	  strncmp(&buf[2],"You have mail",13) == 0)
      fgets(buf,FNSIZE,pid);
   buf[strlen(buf)-1] = 0;
   pclose(pid);
   setbuf(pid,NULL);

   pt = buf;
   if (strchr("%$>",buf[0]) != NULL && buf[1] == ' ') pt = &buf[2];
   while ((ch = *pt++) != 0)
      if (ch == '\\' && *pt != 0) ++pt;
	 else if (ch == ' ')
	  { sprintf(ermsg,"Ambiguous wild card name '%s'",buf);
	    DSerror(ermsg);
	    *--pt = 0;
	  }
	 else if (strchr("*?[",ch) != NULL)
	  { DSerror("Unknown wild card file name");
	    return NULL;
	  };

   return buf;
}





/*******************************************************   Postprocessing */
/*							*/
/*	PRdirchk -- check user directory for valid	*/
/*							*/
/********************************************************/


static INT
PRdirchk(pm)
   INT pm;
{
   if (pm == P_tdirect)
    { if (access(STRVAL(pm),3) < 0)
       { DSerror("Bad temporary directory");
	 our_strcpy(STRVAL(pm),STRDFLT(pm));
	 return FALSE;
       };
    };

   return TRUE;
}





/*******************************************************   Postprocessing */
/*							*/
/*	PRdisplay -- reset after display parm change	*/
/*							*/
/********************************************************/


static int
PRdisplay(pm)	 /* reset display required	 */
   INT pm;
{
   register INT actlen,i;

   if (PVAL(window) <= 1 || PVAL(curwindow) == 0) actlen = PVAL(termlength)-2;
    else if (PVAL(curwindow) == 1) actlen = (PVAL(termlength)-2-1+1)/2;
    else actlen = (PVAL(termlength)-2-1)/2;

   if (pm == P_brief)
    { DSslowdisp();
    }
   else if (PVAL(termlength) != 0 && (pm == P_length || pm == P_termlength))
    { if (PVAL(length) == 0 || PVAL(length) > actlen)
	 PSETI(curlength,actlen);
       else PSET(curlength,PVAL(length));
      i = (INT)(PVAL(curlength)/3);
      PSETI(Aline,i);
      PRscrchk(P_curline);
      if (pm == P_termlength) dispinit = TRUE;
    }
   else if (PVAL(termwidth) != 0 && (pm == P_width || pm == P_termwidth))
    { if (PVAL(width) == 0 || PVAL(width) > PVAL(termwidth))
	 PSET(curwidth,PVAL(termwidth));
       else PSET(curwidth,PVAL(width));
      PRscrchk(P_curchar);
      if (pm == P_termwidth) {
	 dispinit = TRUE;
       };
    }
   else if (pm == P_terminal) dispinit = TRUE;

   return 0;
}






/*******************************************************   Postprocessing */
/*							*/
/*	PRscrchk -- check parameters for screen display */
/*							*/
/********************************************************/


static int
PRscrchk(pm)
   INT pm;
{
   register INT i;

   if (pm == P_curchar && !flipfile)
    { if (PVAL(curchar) < 0) VAL(P_curchar) = 0;
      if (PVAL(curchar) >= LINESIZE) VAL(P_curchar) = (PARMVAL) LINESIZE-1;
      if (PVAL(curchar) < PVAL(curmargin))
       { i = PVAL(curchar) - 5;
	 dispset = TRUE;
	 VAL(P_curmargin) = (PARMVAL)((PINT) (i < 0 ? 0 : i));
       };
      if (PVAL(curwidth) > 0 && PVAL(curchar) >= PVAL(curmargin)+PVAL(curwidth))
       { i = PVAL(curchar)+5;
	 if (i >= LINESIZE) i = LINESIZE-1;
	 dispset = TRUE;
	 VAL(P_curmargin) = (PARMVAL) (i-PVAL(curwidth));
       };
    }
   else if (pm == P_curline && !flipfile)
    { if (PVAL(curline) < 0) VAL(P_curline) = (PARMVAL) 0;
      if (PVAL(curline) < PVAL(curscreen))
       { i = PVAL(curline) - 3;
	 dispset = TRUE;
	 VAL(P_curscreen) = (PARMVAL)((PINT) (i < 0 ? 0 : i));
       };
      if (PVAL(curlength) > 0 && PVAL(curline) >= PVAL(curscreen)+PVAL(curlength))
       { i = PVAL(Aline);
	 if (i <= 0) i = 7;
	 if (i >= PVAL(curlength)) i = PVAL(curlength)-1;
	 i += PVAL(curline);
	 dispset = TRUE;
	 VAL(P_curscreen) = (PARMVAL) (i-PVAL(curlength));
       };
      LNforce(PDVAL(curline));
    }
   else if (pm == P_curmargin && !flipfile)
    { if (PVAL(curmargin) < 0) VAL(P_curmargin) = (PARMVAL) 0;
      if (PVAL(curmargin) >= LINESIZE-PVAL(curwidth))
	 VAL(P_curmargin) = (PARMVAL) (LINESIZE-1-PVAL(curwidth));
      if (PVAL(curchar) < PVAL(curmargin))
	 VAL(P_curchar) = (PARMVAL) PVAL(curmargin);
      if (PVAL(curwidth) > 0 &&
	     PVAL(curchar) >= PVAL(curmargin)+PVAL(curwidth))
	 VAL(P_curchar) = (PARMVAL) (PVAL(curmargin)+PVAL(curwidth)-1);
    }
   else if (pm == P_curscreen && !flipfile)
    { if (PVAL(curscreen) < 0) VAL(P_curscreen) = (PARMVAL) 0;
      if (PVAL(curscreen) > PDVAL(filesize)+3)
	 VAL(P_curscreen) = (PARMVAL) (PDVAL(filesize)+3);
      if (PVAL(curline) < PVAL(curscreen))
       { VAL(P_curline) = (PARMVAL) PVAL(curscreen);
	 PSET(curchar,0);
       };
      if (PVAL(curlength) > 0 &&
	     PVAL(curline) >= PVAL(curscreen)+PVAL(curlength))
       { VAL(P_curline) = (PARMVAL)
		(PVAL(minustop) ? PVAL(curscreen) :
				  PVAL(curscreen)+PVAL(curlength)-1);
	 PSET(curchar,0);
       };
    }
   else if (pm == P_line)
    { if (PDVAL(line) > 0) VAL(P_line) = (PARMVAL) (PDVAL(line)-1);
    }
   else if (pm == P_screen)
    { if (PDVAL(screen) > 0) VAL(P_screen) = (PARMVAL) (PDVAL(screen)-1);
    };

   return TRUE;
}





/*******************************************************   Postprocessing */
/*							*/
/*	PRwindow -- handle changing window		*/
/*							*/
/********************************************************/


static int
PRwindow(pm)
   INT pm;
{
   static BOOL winset = FALSE;
   register INT ln;

   if (pm == P_window)
    { if (PVAL(window) <= 1) PSET(curwindow,0);
      else if (firsttime) return TRUE;
       else if (!winset)
	{ ln = (PVAL(termlength)-2-1+1)/2;
	  TRwindow(0,0,PVAL(termwidth)-1,ln-1);
	  TRwindow(0,ln+1,PVAL(termwidth)-1,PVAL(termlength)-3);
	  TRwindow(0,ln,PVAL(termwidth)-1,ln);
	  winset = TRUE;
	};
      return TRUE;
    };

   if (pm == P_curwindow)
    { TRwinset(PVAL(curwindow));
      PRdisplay(P_length);
      PRdisplay(P_width);
      return TRUE;
    };

   return 0;
}






/*******************************************************   Postprocessing */
/*							*/
/*	PRmesg -- handle changes to `mesg`              */
/*							*/
/********************************************************/


static int
PRmesg(pm)
   INT pm;
{
   static INT initmesg = -1;
   static INT mesgfg = -1;
   CHAR buf[16];

   if (pm == P_mesg)
    { if (PVAL(mesg) != mesgfg)
       { if (initmesg < 0) initmesg = mesg(NULL);
	 mesgfg = mesg((PVAL(mesg) ? "y" : "n"));
       };
    }
   else if (pm < 0) {
      if (initmesg != mesgfg) PRmesg((initmesg ? "n" : "y"));
    }

   return TRUE;
}





/*******************************************************   Postprocessing */
/*							*/
/*	PRlanguage -- changle languages 		*/
/*							*/
/********************************************************/


static int
PRlanguage(pm)
   INT pm;
{
   if (pm == P_curlanguage)
    { LAnewlang();
    }
   else ABORT;

   return TRUE;
}






/*******************************************************   Postprocessing */
/*							*/
/*	PRchange -- handle alteration to file		*/
/*							*/
/********************************************************/


static int
PRchange(pm)
   INT pm;
{
   if (pm == P_nochange)
    { if (PVAL(curreadonly) && PVAL(rowarn) && !PVAL(nochange))
       { DSerror("-dChanges being made to readonly file");
       };
    }
   else ABORT;

   return 0;
}






/*******************************************************   Postprocessing */
/*							*/
/*	PRbuffer -- handle reset of buffers		*/
/*							*/
/********************************************************/


static int
PRbuffer(pm)
   INT pm;
{
   if (pm == P_share) {
      BFinit(NULLSTR,FALSE);
    }
   else ABORT;

   return 0;
}






/* end of parms.c */




