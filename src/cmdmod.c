/********************************************************/
/*							*/
/*		CMDMOD.C				*/
/*							*/
/*	This module contains the main command loop	*/
/*   code.  This code interprets user commands, 	*/
/*   handles interim bookkeeping, and then branches	*/
/*   to handle the various commands.  It is driven	*/
/*   primarily from the command table in the file	*/
/*   commands.						*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#define CMDMOD

#include "global.h"
#include "editor.h"
#include "parms.h"
#include "cmds.h"
#include "langmod.h"
#include "bufmod.h"
#include "lbuff.h"
#include "tempmod.h"
#include <sys/stat.h>





/********************************************************/
/*							*/
/*	Internal Routines				*/
/*							*/
/********************************************************/

extern	void	docomm();

static	void	doenter();
static	void	dotyped();
static	void	dorhs();
static	void	domark();
static	void	domarkcmd();
static	void	domarkend();
static	void	showmark();
static	void	checkmail();
static	void	checkmotd();



/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


#define ARGSIZE 256		/* maximum argument size */


static	TEXT	CMargbuf[ARGSIZE];

static	struct	stat fstatbuf;	/* buffer for checking mail/motd */


				/* command processing modes	*/
typedef enum { CMODEcmd, CMODEcursor, CMODEtype }
	CMODETYPE;

static	CMODETYPE mode; 	/* current mode 	*/

static	TEXT	cmdedt[] = {	/* command editing table */
	CM_ENTER, ICH_ENTER,
	'\r', ICH_WARN,
	CM_LEFT, ICH_BS,
	CM_UP, ICH_LAST,
	0
};





/********************************************************/
/*							*/
/*	CMloop -- main command processing loop		*/
/*							*/
/********************************************************/


void CMloop()
{
   register INT ch;
   register INT pm;
   register PARMVAL vl;

   HSclear();

   PSET(marking,FALSE);
   PSET(copmode,FALSE);
   PSET(lopmode,FALSE);

   mode = CMODEcmd;
   ch = 0;

   for ( ; ; )
    { Parmdo(); 		/* clean up last command	*/
      if (mode == CMODEcmd)
       { HSmark();		/* record history	*/
	 if (*PSVAL(mail) != 0) checkmail();
	 if (PVAL(motd)) checkmotd();
	 if ((PINT)(LAidle) != 0) (*LAidle)();
	 TMsync();
       };
      if (PVAL(marking)) showmark(TRUE);
      if (mode == CMODEcursor && !PVAL(marking) && !PVAL(movewindow)) DSpos();
      else DSshow();		/* update screen		*/

      MNposition();
      MNsync();

      ch = INcmdchar(); 	/* get next command		*/

      if (mode != CMODEcmd &&
		(ch < CM_FIRST || !(CMDS(ch).CMflags&CMFcmode)))
       { mode = CMODEcmd;
	 INunput(ch);
	 continue;
       };

				/* handle non-command input	*/
      if (ch < CM_FIRST || CMDS(ch).CMflags&CMFtyping)
       { mode = CMODEtype;
	 dotyped(ch);
	 continue;
       };

      if (CMDS(ch).CMflags&CMFcmode) mode = CMODEcursor;

      switch(ch)		 /* process command		 */
       { case CM_ENTER	 : doenter();	 /* ENTER		 */
			   break;

	 case CM_MARK	 : domark();	 /* MARK		 */
			   break;

	 default	 : pm = CMDS(ch).CMparm;
			   if (pm > 0) vl = PARMS[pm].PMvalue;
			      else vl = (PARMVAL) 0;
			   docomm(ch,vl);
			   break;
       };
    };
}





/********************************************************/
/*							*/
/*	CMdomark -- handle external rqst to start mark	*/
/*							*/
/********************************************************/


void CMdomark(int newfg)
{
   if (PVAL(marking)) {
      if (!newfg) return;
      domarkend(FALSE);
    };

   domark();
}




/********************************************************/
/*							*/
/*	docomm -- execute command with given arg	*/
/*							*/
/********************************************************/


void docomm(cmd,val)
   INT cmd;
   PARMVAL val;
{
   INT (*rtn)();
   register INT fgs,mkfg;

   mkfg = FALSE;

   if (CMDS(cmd).CMflags & CMFabort)
    { DSerror("Command invalid with arguments");
      return;
    };

   if (PVAL(dosave) && CMDS(cmd).CMflags&CMFdoend) DOset(BF_DO);

   if (PVAL(marking))
    { fgs = CMDS(cmd).CMflags;
      if (fgs & CMFerrmark)
       { DSerror("Illegal command for marking");
	 return;
       }
      else if (fgs & CMFwarnmark)
       { DSerror("Marking mode terminated");
	 domarkend(FALSE);
       }
      else if (fgs & CMFusemark)
       { domarkcmd();
	 mkfg = TRUE;
       };
    };

   if (CMDS(cmd).CMflags&CMFtyping)
    { mode = CMODEtype;
      dotyped(cmd);
      return;
    };

   rtn = CMDS(cmd).CMrtn;
   if (rtn == NULL) return;

   (*rtn)(cmd,val,mkfg);
}





/********************************************************/
/*							*/
/*	doenter -- handle ENTER -- command arguments	*/
/*							*/
/********************************************************/


static void
doenter()
{
   register INT cmd,svfg,pm,ln,svvl,bno,argfg;
   PARMVAL vl;
   register TEXT *pt;
   static TEXT lastarg[ARGSIZE] = "";

   our_strcpy((char *) CMargbuf,(char *) lastarg);
   DSmark();
   DSreply(">",CMargbuf,ARGSIZE,cmdedt);

   ln = strlen((char *) CMargbuf);
   cmd = CMargbuf[--ln];
   CMargbuf[ln] = 0;
   pm = CMDS(cmd).CMparm;

   svvl = 0;
   pt = CMargbuf;
   while (*pt == ICH_ENTER) { ++svvl; ++pt; --ln; };
   if (svvl > 3)
    { DSerror("Too many ENTER's on command, excess ignored");
      svvl = 3;
    };

   if (svvl > 0) our_strcpy((char *) CMargbuf,(char *) pt);
   bno = BF_A-1;
   pt = CMargbuf;
   if ((CMDS(cmd).CMflags&CMFignore) != 0)
    { while (*pt != 0)
       { if (*pt == ICH_DELIM)
	  { *pt = 0;
	    if (!(CMDS(cmd).CMflags&CMFargs))
	     { DSerror("Multiple arguments not allowed for command");
	       break;
	     }
	    else
	     { if (bno >= BF_A) BFset(bno);
	       BFclear(++bno);
	     };
	  }
	 else if (bno >= BF_A) BFputch(*pt,bno);
	 ++pt;
       };
      if (bno >= BF_A) BFset(bno);
    };

   if (ln == 0 && svvl == 0 && CMDS(cmd).CMflags&CMFgomark)
    { INunput(cmd);
      DSunmark();
      domark();
      return;
    };

   our_strcpy((char *) lastarg,(char *) CMargbuf);

   if (pm < 0 && ln > 0 && (CMDS(cmd).CMflags&CMFignore) != 0)
      DSerror("Command takes no arguments");

   svfg = ((svvl == 1) || (svvl == 3));
   argfg = ((svvl == 2) || (svvl == 3) || (ln > 0));
   if (CMDS(cmd).CMflags & CMFnosave) svfg = FALSE;
   if (CMDS(cmd).CMflags & CMFflipsave) svfg = !svfg;

   vl = (PARMVAL) 0;
   DSunmark();

   if (!(CMDS(cmd).CMflags&CMFignore) && pm >= 0)
      if (Pcmdarg(pm,svfg,argfg,CMargbuf,&vl))
	 docomm(cmd,vl);
}





/********************************************************/
/*							*/
/*	dotyped -- handle non-command character 	*/
/*							*/
/********************************************************/


static void
dotyped(ch)
   INT ch;
{
   register INT rhs,lhs,i;
   register BOOL copfg;
   register DATA line;

   PSET(nochange,FALSE);
   lhs = PVAL(curmargin);
   rhs = lhs+PVAL(curwidth);
   copfg = FALSE;

   for ( ; ; )
    { while (ch < CM_FIRST)
       { MDtype(ch,PVAL(copmode));
	 if (PVAL(curchar) < rhs-1) PSET(curchar,PVAL(curchar)+1);
	    else
	     { dorhs();
	       lhs = PVAL(curmargin);
	       rhs = lhs+PVAL(curwidth);
	     };
	 if (LAnchr > 0 && LAtyped != NULL)
	    for (i = 0; i < LAnchr; ++i)
	       if (ch == LAchrs[i])
		{ (*LAtyped)(ch);
		  break;
		};
	 MNsync();
	 ch = INcmdchar();
       };

      switch(ch)
       { case CM_COPEN	:
		PSETI(copmode,!PVAL(copmode));
		copfg = TRUE;
		DSshow();
		break;

	 case CM_BS	:
	 case CM_LEFT	:
		if (PVAL(curchar) == lhs) goto exit;
		PSET(curchar,PVAL(curchar)-1);
		if (ch == CM_LEFT) MDtype(CHR_BS,PVAL(copmode));
		   else MDtype(CHR_BSDEL,PVAL(copmode));
		break;

	 case CM_0TAB	:
		if (!PVAL(copmode) || !PVAL(tabtypein)) goto exit;
		i = PVAL(curchar)-lhs;
		i = ((i+8)&(~7))+lhs;
		if (i >= rhs) { TRbells(1); break; }
		while (PVAL(curchar) < i)
		 { MDtype(' ',PVAL(copmode));
		   PSET(curchar,PVAL(curchar)+1);
		 };
		break;

	 case CM_0BTAB	:
		if (!PVAL(copmode) || !PVAL(tabtypein)) goto exit;
		i = PVAL(curchar)-lhs;
		i = (i-1)&(~7)+lhs;
		if (i < lhs) goto exit;
		while (PVAL(curchar) > i)
		 { PSET(curchar,PVAL(curchar)-1);
		   MDtype(CHR_BS,PVAL(copmode));
		 };
		break;

	 default	:
		goto exit;
       };
      ch = INcmdchar();
    };

exit:
   if (PVAL(copmode)) PSET(copmode,FALSE);
   INunput(ch);

   line = PDVAL(curline);
   MDtypend();

   if (copfg && PVAL(curstream)) DSfrom(line);
}






/********************************************************/
/*							*/
/*	dorhs -- handle typing at rhs of line		*/
/*							*/
/********************************************************/


static void
dorhs()
{
   register INT i;
   register DATA ln;

   ln = PDVAL(curline);

   MDtypend();

   if (PVAL(curstream))
    { DSfrom(ln);
      Parmdo();
      HSmark();
    }
   else switch(PVAL(rhsoption))
    { case RHS_CONT	:
		PSET(curchar,PVAL(curchar)+1);
		Parmdo();
		break;

      default		:
		TRbells(1);
		break;
    };

   DSshow();
}






/********************************************************/
/*							*/
/*	domark -- start marking 			*/
/*							*/
/********************************************************/


static void
domark()
{
   if (PVAL(marking))
    { domarkend(TRUE);
      return;
    };

   PSET(marking,TRUE);
   PSET(markline,PDVAL(curline));
   PSET(markchar,PVAL(curchar));
   DSmark();
}





/********************************************************/
/*							*/
/*	domarkcmd -- process command using marking	*/
/*							*/
/********************************************************/


static void
domarkcmd()
{
   register INT i,j,fg;

   i = PDVAL(curline)-PDVAL(markline);
   j = PVAL(curchar)-PVAL(markchar);

   if (i > 0 || (i == 0 && j > 0))	/* forward case */
    { PSETI(mcntline,i);
      PSET(mcntchar,(i == 0 ? j : PVAL(curchar)));
      fg = TRUE;
    }
   else 				/* backward case */
    { PSETI(mcntline,(-i));
      PSET(mcntchar,(i == 0 ? -j : PVAL(markchar)));
      fg = FALSE;
    };

   domarkend(fg);
}





/********************************************************/
/*							*/
/*	domarkend -- finish up marking, reset if fg	*/
/*							*/
/********************************************************/


static void
domarkend(fg)
   BOOL fg;
{
   if (fg)
    { PSET(curline,PVAL(markline));
      PSET(curchar,PVAL(markchar));
    };

   PSET(marking,FALSE);
   showmark(FALSE);
   DSunmark();
}





/********************************************************/
/*							*/
/*	showmark -- display marked area in standout md	*/
/*							*/
/********************************************************/


static void
showmark(fg)
   BOOL fg;
{
   register INT x0,y0,x1,y1;
   static INT oldx0 = -1;
   static INT oldy0 = -1;
   static INT oldx1 = -1;
   static INT oldy1 = -1;

   if (!fg)
    { DSwrite();
      oldx0 = oldy0 = oldx1 = oldy1 = -1;
      return;
    };

   if (PVAL(markline) < PVAL(curline) ||
	(PVAL(markline) == PVAL(curline) && PVAL(markchar) < PVAL(curchar)))
    { x0 = PVAL(markchar);
      y0 = PVAL(markline);
      x1 = PVAL(curchar);
      y1 = PVAL(curline);
    }
   else
    { x0 = PVAL(curchar);
      y0 = PVAL(curline);
      x1 = PVAL(markchar);
      y1 = PVAL(markline);
    };

   if (oldx0 >= 0)
    { if (oldy0 < y0 || (oldy0 == y0 && oldx0 < x0))
	 TRmark(oldx0-PVAL(curmargin),
		oldy0-PVAL(curscreen),
		x0-PVAL(curmargin),
		y0-PVAL(curscreen),FALSE);
      if (oldy1 > y1 || (oldy1 == y1 && oldx1 > x1))
	 TRmark(x1-PVAL(curmargin),
		y1-PVAL(curscreen),
		oldx1-PVAL(curmargin),
		oldy1-PVAL(curscreen),FALSE);
    };

   TRmark(x0-PVAL(curmargin),
	  y0-PVAL(curscreen),
	  x1-PVAL(curmargin),
	  y1-PVAL(curscreen),TRUE);

   oldx0 = x0;
   oldx1 = x1;
   oldy0 = y0;
   oldy1 = y1;
}






/********************************************************/
/*							*/
/*	checkmail -- check for user mail arrival	*/
/*							*/
/********************************************************/


static void checkmail()
{
   static INT cntr = -1;
   static time_t date = 0;

   if (--cntr <= 0)
    { if (stat(PSVAL(mail),&fstatbuf) >= 0 &&
	     date != fstatbuf.st_mtime)
       { date = fstatbuf.st_mtime;
	 if (cntr == 0 && fstatbuf.st_size > 0) DSerror("You have new mail");
       };
      cntr = 10;
    };
}





/********************************************************/
/*							*/
/*	checkmotd -- check for new message of the day	*/
/*							*/
/********************************************************/


static void checkmotd()
{
   static INT cntr = -1;
   static time_t date = 0;

   if (--cntr <= 0)
    { if (stat("/etc/motd",&fstatbuf) >= 0 &&
	     date != fstatbuf.st_mtime)
       { date = fstatbuf.st_mtime;
	 if (cntr == 0) DSerror("-dThe message of the day has changed");
       };
      cntr = 10;
    };
}






/* end of cmdmod.c */
