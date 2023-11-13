/********************************************************/
/*							*/
/*		PROCMOD.C				*/
/*							*/
/*	This module contains the primary routines for	*/
/*   processing user commands.	Each routine processes	*/
/*   one or more commands.  All are called from the	*/
/*   command module and are specified in the command	*/
/*   table.  They share a common calling sequence:	*/
/*		cmd -- actual command			*/
/*		val -- (PARMVAL) value			*/
/*		mkf -- marking flag			*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "parms.h"
#include "cmds.h"
#include "bufmod.h"
#include "langmod.h"
#include "lbuff.h"
#include "tempmod.h"
#include "linemod.h"



/********************************************************/
/*							*/
/*	2 character macros				*/
/*							*/
/********************************************************/

#define OP_CHAR 	(('o' << 8) + 'p')


/********************************************************/
/*							*/
/*	External Routines				*/
/*							*/
/********************************************************/

extern	void	docomm();




/********************************************************/
/*							*/
/*	Internal Routines				*/
/*							*/
/********************************************************/


static	int	dodelpick();
static	void	getspacing();
static	INT	openline();




/********************************************************/
/*							*/
/*	CMDwindow -- handle window movement commands	*/
/*							*/
/********************************************************/


int CMDwindow(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register DATA sc,dvl;

   sc = PDVAL(curscreen);
   dvl = (DATA) val;

   switch (cmd)
    { case CM_PPAGE	: sc += dvl*PDVAL(curlength); break;
      case CM_MPAGE	: sc -= dvl*PDVAL(curlength); break;
      case CM_PLINE	: sc += dvl;		      break;
      case CM_MLINE	: sc -= dvl;		      break;
    };

   if (sc > PVAL(filesize)-3) sc = PVAL(filesize)-3;
   if (sc < 0) sc = 0;

   PSET(curscreen,sc);

   return TRUE;
}





/********************************************************/
/*							*/
/*	CMDsearch -- search commands			*/
/*							*/
/********************************************************/


int CMDsearch(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register BOOL refg;
   register STRING str;
   register LINE scrn;
   LINE line;
   INT chr;
   register INT dir,buf,sts;
   CHAR lbuf[FNSIZE];
   register CHAR *lp;
   register INT ch;

   dir = 1;
   str = (STRING) val;

   switch(cmd)
    { case CM_MSRCH	: dir = -1;		/* search */
      case CM_PSRCH	:
		refg = PVAL(search);
		break;

      case CM_MRSRCH	: dir = -1;		/* reg expr search */
      case CM_PRSRCH	:
		refg = TRUE;
		break;

      case CM_MFSRCH	: dir = -1;		/* fast search	*/
      case CM_PFSRCH	:
		refg = TRUE;
		break;

      case CM_MBSRCH	: dir = -1;		/* buffer search */
      case CM_PBSRCH	:
		refg = FALSE;
		buf = BFCBUF(val);
		BFset(buf);
		lp = lbuf;
		while ((ch = BFget(buf)) != BFC_EOF && ch != BFC_EOL)
		   *lp++ = ch;
		*lp = 0;
		str = lbuf;
		dir *= BFCCOUNT(val);
		break;
    };

   if (*str == 0)
    { DSerror("Nothing to search for");
      return FALSE;
    }
   else sts = SEarch(str,dir,refg,&line,&chr);	/* do the search	*/

   if (sts)
    { scrn = PDVAL(curscreen);
      PSET(curline,line);
      PSET(curchar,(PTR) chr);
      if (scrn != PDVAL(curscreen))
	 PSET(curscreen,PVAL(curline)-3);
    }
   else if (PVAL(douse))
    { DOstop();
      DSerror("Do terminated by search failure");
    }
   else DSerror("Not found");

   return sts;
}






/********************************************************/
/*							*/
/*	CMDgoto -- goto commands			*/
/*							*/
/********************************************************/


#define GOSTKSIZ 16		/* size of goto stack	*/



int CMDgoto(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   static INT gotocnt = 0;
   register INT bufid;
   register DATA line;

   switch(cmd)
    { case CM_GOTO :
		if (BFCBUF(val) == BADBUF && BFCCOUNT(val) == 0)
		 { if (--gotocnt < 0) gotocnt = GOSTKSIZ-1;
		   bufid = ('g'<<8)+(gotocnt);
		   line = LNgetmark(bufid);
		   if (line >= 0) LNunmark(bufid); else line = -1;
		 }
		else if (BFCBUF(val) == BADBUF) line = -1;
		else {
		   bufid = ('g' << 8) + 'a' + BFCBUF(val);
		   line = LNgetmark(bufid);
		   if (line < 0) line = -1;
		 };
		line += BFCCOUNT(val);
		break;

      case CM_GOBEG	:
		line = BFCCOUNT(val)-1;
		break;

      case CM_GOEND	:
		line = PDVAL(filesize)-BFCCOUNT(val);
		break;

      case CM_SVPOS	:
		bufid = (INT)(long) val;
		if (bufid == BADBUF) bufid = ('g'<<8)+(gotocnt++);
		else bufid += ('g' << 8) + 'a';
		LNmark(bufid,PDVAL(curline));
		DSerror("-d-bPosition saved");
		return TRUE;
    };

   if (line < 0) line = 0;

   PSET(curline,line);
   PSET(curchar,0);
   CMDcursor(CM_IND,(PARMVAL)0,FALSE);

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDtabs -- handle language-specific tab cmds	*/
/*							*/
/********************************************************/


int CMDtabs(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   DATA line;
   INT chr;
   register INT cnt,nm;

   cnt = (INT)(long)val;
   if (cnt == 0) cnt = 1;

   switch(cmd)
    { case CM_1BTAB : cnt = -cnt;
      case CM_1TAB  : nm = 0;
		      break;

      case CM_2BTAB : cnt = -cnt;
      case CM_2TAB  : nm = 1;
		      break;

      case CM_3BTAB : cnt = -cnt;
      case CM_3TAB  : nm = 2;
		      break;

      case CM_4BTAB : cnt = -cnt;
      case CM_4TAB  : nm = 3;
		      break;

      case CM_5BTAB : cnt = -cnt;
      case CM_5TAB  : nm = 4;
		      break;
    };

   if (LAtabs[nm] == NULL) return FALSE;

   if ((*(LAtabs[nm]))(cnt,&chr,&line))
    { PSET(curline,line);
      PSET(curchar,(PTR) chr);
    }
   else TRbells(1);

   return TRUE;
}





/********************************************************/
/*							*/
/*	CMDmargin -- handle commands to change margin	*/
/*							*/
/********************************************************/


int CMDmargin(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT mg,ivl;

   mg = PVAL(curmargin);
   ivl = (INT)(long) val;

   switch (cmd)
    { case CM_LSLIDE	: mg += ivl; break;
      case CM_RSLIDE	: mg -= ivl; break;
    };

   if (mg >= LINESIZE-PVAL(curwidth)) mg = LINESIZE-PVAL(curwidth)-1;
   if (mg < 0) mg = 0;

   PSET(curmargin,(PTR) mg);

   return TRUE;
}





/********************************************************/
/*							*/
/*	CMDcursor -- handle cursor movement commands	*/
/*							*/
/********************************************************/


int CMDcursor(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT x,y,mrk,oind;
   register BOOL mvfg;

   x = PVAL(curchar)-PVAL(curmargin);
   y = PVAL(curline)-PVAL(curscreen);
   mvfg = PVAL(movewindow);
   mrk = -1;
   oind = -1;

   switch (cmd)
    { case CM_RIGHT	: x++; break;
      case CM_BS	:
      case CM_LEFT	: x--; break;
      case CM_UP	: y--; break;
      case CM_DOWN	: y++; break;

      case CM_NL	:
      case CM_RET	: y++;
			  x = -PVAL(curmargin);
			  mvfg = TRUE;
			  break;
      case CM_BRET	: y--;
			  x = -PVAL(curmargin);
			  mvfg = TRUE;
			  break;

      case CM_0TAB	: x = (x+8)&(~7); break;
      case CM_0BTAB	: x = (x-1)&(~7); break;
      case CM_HOME	: x = y = 0;	  break;
      case CM_IND	: mvfg = TRUE;	  break;

      case CM_VTAB	: mvfg = TRUE;
			  y += PVAL(Aline);
			  break;
      case CM_BHOME	: x = 0;
			  y = PVAL(curlength)-1;
			  break;
      case CM_ABSPOS	: x = INchar()-32-1;
			  y = INchar()-32-1;
			  if (x >= PVAL(curwidth)) x = PVAL(curwidth)-1;
			  if (x < 0) x = 0;
			  if (y < 0 || y >= PVAL(curlength)) mvfg = TRUE;
			  break;
      case CM_MRKPOS	: x = INchar()-32-1;
			  y = INchar()-32-1;
			  if (x >= PVAL(curwidth)) x = PVAL(curwidth)-1;
			  if (x < 0) x = 0;
			  if (y < 0 || y >= PVAL(curlength)) mvfg = TRUE;
			  mrk = 0;
			  break;
      case CM_MRKAT	: x = INchar()-32-1;
			  y = INchar()-32-1;
			  if (x >= PVAL(curwidth)) x = PVAL(curwidth)-1;
			  if (x < 0) x = 0;
			  if (y < 0 || y >= PVAL(curlength)) mvfg = TRUE;
			  mrk = 1;
			  break;
    };

   if (!mvfg && ( x < 0 || x >= PVAL(curwidth) || y < 0 || y >= PVAL(curlength)))
    { TRbells(1);
      return FALSE;
    };

   PSET(curline,PVAL(curscreen)+y);

   if (cmd == CM_RET || cmd == CM_NL)
      if (PVAL(lopmode) && PDVAL(curline) == LNgetmark(OP_CHAR))
       { PSET(curchar,0);
	 MDins(BF_NEWLINE,1);
	 LNmark(OP_CHAR,PDVAL(curline)+1);
	 oind = openline();
       };

   if (cmd == CM_RET || cmd == CM_IND || cmd == CM_BRET)
    { if (PVAL(curindent))
       { LNget(PDVAL(curline));
	 for (x = 0; LBlinebuf[x] == ' '; ++x);
	 if (LBlinebuf[x] == 0 && LAindent != NULL) x = (*LAindent)();
	 if (oind >= 0) x = oind;
	 x -= PVAL(curmargin);
       };
    };

   PSET(curchar,PVAL(curmargin)+x);

   if (mrk >= 0) CMdomark(mrk);

   if (mvfg != PVAL(movewindow)) DSshow();

   return TRUE;
}



static INT
openline()
{
   INT x,y,x1;
   INT ind = -1;
   INT rind = -1;

   for (y = PDVAL(curline)-1; y >= 0; --y) {
      LNget(y);
      for (x = 0; LBlinebuf[x] == ' '; ++x);
      if (LBlinebuf[x] == '*' && (LBlinebuf[x+1] == ' ' || LBlinebuf[x+1] == 0)) {
	 if (ind < 0) ind = x;
	 else if (x < ind && x > 0) ind = x;
	 for (x1 = x+1; LBlinebuf[x1] == ' '; ++x1) ;
	 if (LBlinebuf[x1] != 0) {
	    if (rind < 0 || x1-x-1 < rind) rind = x1-x-1;
	  }
       }
      else if (LBlinebuf[x] == '/' && LBlinebuf[x+1] == '*' && LBlinebuf[x+2] == '*') {
	 if (ind < 0) return -1;
	 else break;
       }
      else return -1;
    }
   if (y < 0) return -1;

   PSET(curchar,(PTR) ind);
   MDins(BF_STAR,1);
   if (rind < 0) rind = 1;
   PSET(curchar,(PTR)(ind+1+rind));

   return ind+1+rind;
}




/********************************************************/
/*							*/
/*	CMDinsert -- handle insert/put commands 	*/
/*							*/
/********************************************************/


int CMDinsert(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT buf,i,cnt;
   register BOOL retfg;

   if (PVAL(nochange)) PSET(nochange,FALSE);
   retfg = FALSE;

   if (mrkfg)
    { if (cmd != CM_CINS && cmd != CM_LINS) ABORT;
      if (PVAL(mcntchar) != 0) MDins(BF_SPACE,PVAL(mcntchar));
      if (PVAL(mcntline) != 0) MDins(BF_NEWLINE,PVAL(mcntline));
    }
   else if (cmd == CM_LOPEN)
    { PSET(lopmode,(PTR) !PVAL(lopmode));
      if (PVAL(lopmode))
       { if (PVAL(curindent) && PVAL(curchar) != 0)
	  { LNget(PDVAL(curline));
	    for (i = 0; i < PVAL(curchar) && LBlinebuf[i] == ' '; ++i);
	    if (i == PVAL(curchar) || LBlinebuf[i] == 0) PSET(curchar,0);
	  };
	 if (PVAL(curchar) == 0)
	  { MDins(BF_NEWLINE,1);
	    CMDcursor(CM_IND,(PARMVAL)0,FALSE);
	    openline();
	  };
	 LNmark(OP_CHAR,PDVAL(curline)+1);
       }
      else LNunmark(OP_CHAR);
    }
   else
    { switch (cmd)
       { case CM_CINS : buf = BF_SPACE; break;
	 case CM_LINS :
		if (PVAL(curindent) && PVAL(curchar) != 0)
		 { LNget(PDVAL(curline));
		   for (i = 0; i < PVAL(curchar) && LBlinebuf[i] == ' '; ++i);
		   if (i == PVAL(curchar)) retfg = TRUE;
		 };
		buf = BF_NEWLINE;
		break;
	 default      : buf = BFCBUF(val); break;
       };
      BFset(buf);
      if (BFget(buf) == DCH_START) DOstart(buf,BFCCOUNT(val));
	 else
	  { if (retfg)
	     { i = PVAL(curchar);
	       PSET(curchar,0);
	     };
	    MDins(buf,BFCCOUNT(val));
	    if (retfg) CMDcursor(CM_IND,(PARMVAL)0,FALSE);
	  };
    };

   return TRUE;
}





/********************************************************/
/*							*/
/*	CMDdelete, CMDpick -- handle pick/delete cmds	*/
/*							*/
/********************************************************/


int CMDdelete(cmd,val,mrkfg)	    /* DELETE commands	    */
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   if (PVAL(nochange)) PSET(nochange,FALSE);

   return dodelpick(cmd,val,mrkfg,TRUE);
}





int CMDpick(cmd,val,mrkfg)	    /* PICK commands	    */
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   return dodelpick(cmd,val,mrkfg,FALSE);
}





static int
dodelpick(cmd,val,mrkfg,delfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
   INT delfg;
{
   register INT nm,i;
   INT x,lcnt,ccnt;
   DATA y;
   register BOOL retfg;

   retfg = FALSE;
   getspacing(cmd,val,mrkfg,&lcnt,&ccnt);

   if (!mrkfg) {
      switch (cmd)
       { case CM_LDEL	:
	 case CM_LPICK	:
	 case CM_LSWAP	:
		if (PVAL(curindent) && PVAL(curchar) != 0)
		 { LNget(PDVAL(curline));
		   for (i = 0; i < PVAL(curchar) && LBlinebuf[i] == ' '; ++i);
		   if (i == PVAL(curchar) || LBlinebuf[i] == 0) retfg = TRUE;
		 };
		break;
       };

      if (lcnt+PDVAL(curline) > PDVAL(filesize) && delfg)
       { DSerror("-dDelete stopped at end of file");
	 lcnt = PDVAL(filesize)-PDVAL(curline);
       };
    };

   if (lcnt != 0 || ccnt != 0) {
      if (retfg) {
	 i = PVAL(curchar);
	 PSET(curchar,0);
       };
      MDdel(BFCBUF(val),lcnt,ccnt,delfg);
      if (retfg) CMDcursor(CM_IND,(PARMVAL)0,FALSE);
    };

   return TRUE;
}





static void
getspacing(cmd,val,mrkfg,lp,cp)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
   INT *lp,*cp;
{
   register INT lcnt,ccnt,nm,i;
   INT x;
   DATA y;

   if (mrkfg) {
      lcnt = PVAL(mcntline);
      ccnt = PVAL(mcntchar);
    }
   else
    { lcnt = ccnt = 0;
      nm = -1;
      switch (cmd)
       { case CM_CDEL	:
	 case CM_CPICK	:
	 case CM_CSWAP	:
		ccnt = BFCCOUNT(val);
		break;

	 case CM_LDEL	:
	 case CM_LPICK	:
	 case CM_LSWAP	:
		lcnt = BFCCOUNT(val);
		break;

	 case CM_1DEL	:
	 case CM_1PICK	:
	 case CM_1SWAP	:
		nm = 0;
		break;

	 case CM_2DEL	:
	 case CM_2PICK	:
	 case CM_2SWAP	:
		nm = 1;
		break;

	 case CM_3DEL	:
	 case CM_3PICK	:
	 case CM_3SWAP	:
		nm = 2;
		break;

	 case CM_4DEL	:
	 case CM_4PICK	:
	 case CM_4SWAP	:
		nm = 3;
		break;

	 case CM_5DEL	:
	 case CM_5PICK	:
	 case CM_5SWAP	:
		nm = 4;
		break;
       };

      if (nm >= 0 && LAtabs[nm] != NULL)
       { (*(LAtabs[nm]))(BFCCOUNT(val),&x,&y);
	 lcnt = y-PDVAL(curline);
	 if (lcnt == 0) ccnt = x-PVAL(curchar);
	    else ccnt = x;
       };
    };

   *lp = lcnt;
   *cp = ccnt;
}





/********************************************************/
/*							*/
/*	CMDswap -- swap buffer and indicated area	*/
/*							*/
/********************************************************/


int CMDswap(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT buf;
   register INT x,y;
   INT lcnt,ccnt;

   buf = BFCBUF(val);

   BFset(buf);
   if (BFget(buf) == DCH_START) {
      DSerror("SWAP illegal with DO buffer");
      return FALSE;
    };

   x = PVAL(curchar);
   y = PDVAL(curline);
   getspacing(cmd,val,mrkfg,&lcnt,&ccnt);

   if (lcnt == 0) {
      PSET(curchar,(PTR)(x+ccnt));
    }
   else {
      PSET(curline,(PTR)(y+lcnt));
      PSET(curchar,(PTR) ccnt);
    };

   MDins(buf,1);
   PSET(curchar,(PTR) x);
   PSET(curline,(PTR) y);

   if (lcnt != 0 || ccnt != 0) MDdel(buf,lcnt,ccnt,TRUE);

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDindent -- indent one or more lines		*/
/*							*/
/********************************************************/


int CMDindent(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT i,cnt,lcnt;
   register DATA y,y0;

   y0 = y = PDVAL(curline);
   cnt = PVAL(curchar);

   lcnt = (INT)(long) val;
   if (mrkfg)
    { lcnt = PVAL(mcntline);
      if (lcnt == 0) lcnt = 1;
       else
	{ LNget(y+lcnt);
	  for (i = 0; i < PVAL(mcntchar); ++i)
	     if (LBlinebuf[i] != ' ') { ++lcnt; break; };
	};
    };

   for ( ; lcnt-- > 0 && y < PDVAL(filesize); ++y)
    { if (LAindent == NULL) return FALSE;
      PSET(curline,y);
      cnt = (*LAindent)();
      LNget(y);
      for (i = 0; LBlinebuf[i] == ' '; ++i);
      if (LBlinebuf[i] == 0) i = cnt;
      i = cnt-i;

      PSET(curchar,0);
      if (i < 0) MDdel(BF_CHAR,0,-i,TRUE);
      if (i > 0) MDins(BF_SPACE,i);
    };

   PSET(curchar,(PTR) cnt);

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDjoin -- convenient join of two lines 	*/
/*							*/
/********************************************************/


int CMDjoin(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT x0,x,cnt,buf;
   register DATA y,y0;

   x0 = PVAL(curchar);
   y = y0 = PDVAL(curline);

   cnt = BFCCOUNT(val);
   buf = BFCBUF(val);
   if (mrkfg)
    { cnt = PVAL(mcntline);
      if (cnt == 0) cnt = 1;
    };

   while (--cnt >= 0 && y < PDVAL(filesize))
    { y = PDVAL(curline);
      LNget(y);
      for (x = strlen((char *) LBlinebuf)-1; x >= 0; --x)
	 if (LBlinebuf[x] != ' ') break;
      if (x < 0) PSET(curchar,0);
	 else PSET(curchar,(PTR)(x+2));
      LNget(y+1);
      for (x = 0; LBlinebuf[x] == ' ' || LBlinebuf[x] == '\t'; ++x);
      if ((int)(PVAL(curchar)+(strlen((char *) LBlinebuf)-x)) >= LINESIZE-2)
       { DSerror("Line too long for JOIN");
	 break;
       };
      MDdel(buf,1,x,TRUE);
    };

   PSET(curline,y0);
   PSET(curchar,(PTR) x0);

   DSfrom(PDVAL(curline));

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDdo -- handle do commands			*/
/*							*/
/********************************************************/


int CMDdo(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT buf,cnt;

   buf = BFCBUF(val);
   cnt = BFCCOUNT(val);

   switch(cmd)
    { case CM_DOSET	:
		DOset(buf);
		break;

      case CM_DO	:
		if ((PVAL(doboth) || !(PVAL(doset))) && cnt <= 0)
		 { DOset(buf);
		   break;
		 };
		if (cnt <= 0) cnt = 1;
		DOstart(buf,cnt);
		break;
    };

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDundo -- handle undo command			*/
/*							*/
/********************************************************/


int CMDundo(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT cnt;

   cnt = (INT)(long) val;

   while (cnt-- >= 0 && HSundo());

   DSwrite();

   return 0;
}






/********************************************************/
/*							*/
/*	CMDfile -- handle file set commands		*/
/*							*/
/********************************************************/


int CMDfile(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register STRING fl,pt,npt;
   CHAR buf[FNSIZE];

   if (PVAL(nonewfile) &&
	(cmd == CM_FSET || cmd == CM_FALT || cmd == CM_FNEXT ||
		cmd == CM_FMAKE))
    { DSerror("Can't change files.  Try exiting.");
      return FALSE;
    };

   fl = (STRING) val;
   PSET(alternate,FALSE);

   switch(cmd)
    { case CM_FSET	:		/* SET FILE	*/
		PSET(newfile,FALSE);
		break;

      case CM_FALT	:		/* ALTERNATE FILE */
		PSET(alternate,TRUE);
		PSET(newfile,FALSE);
		break;

      case CM_FMAKE	:		/* MAKE FILE	*/
		PSET(newfile,TRUE);
		break;

      case CM_FNEXT	:		/* NEXT FILE	*/
		if (*PSVAL(afterfile) == 0)
		 { DSerror("No next file");
		   return FALSE;
		 };
		npt = buf;
		for (pt = PSVAL(afterfile); *pt != ' '; ++pt)
		   *npt++ = *pt;
		*npt = 0;
		++pt;
		PSET(afterfile,pt);
		fl = buf;
		break;

      case CM_PARM	:			/* parameter command	*/
		break;

      case CM_RSTRT	:			/* restart command	*/
		if (DSquery("Restart (y or n)? ","yY") < 0) return TRUE;
		PSET(tabin,PVAL(tabin));
		PSET(nochange,TRUE);
		break;
      case CM_SAVE	:
		PSET(tabin,PVAL(tabin));
		PSET(savequery,FALSE);
		break;
    };

   Parm(fl,TRUE);

   if (cmd == CM_RSTRT) HSclear();

   return TRUE;
}





/********************************************************/
/*							*/
/*	CMDshow -- show parameters to user for editing	*/
/*							*/
/********************************************************/


int CMDshow(cmdid,val,mrkfg)
   INT cmdid;
   PARMVAL val;
   INT mrkfg;
{
   CHAR pmnm[FNSIZE],cmd[PMXSIZE*2];
   register FILE *fid;
   register STRING pt;
   int fno;

   pt = (STRING) val;
   if (pt == NULL || *pt == 0) pt = "bPARMSXXXXXX";

   sprintf(pmnm,"%s/%s",PSVAL(tdirect),pt);
   fno = mkstemp(pmnm);
   fid = fdopen(fno,"w");
   if (fid == NULL) ABORT;
   setbuf(fid,NULL);

   Pwriteall(fid);

   fclose(fid);

   pt = (STRING) val;
   if (pt == NULL || *pt == 0)
    { sprintf(cmd,"%s ba=0 nocontrol=1 nonewfile=1 %s",PSVAL(editor),pmnm);
      DSsystem(cmd,FALSE);

      fid = fopen(pmnm,"r");
      if (fid == NULL) return TRUE;
      setbuf(fid,NULL);
      while (fgets(cmd,PMXSIZE*2,fid) != NULL) Parm(cmd,FALSE);
      fclose(fid);
      unlink(pmnm);
    };

   return TRUE;
}





/********************************************************/
/*							*/
/*	CMDhelp -- help command 			*/
/*							*/
/********************************************************/


int CMDhelp(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   CHAR str[1024];
   register STRING svl,pms;

   pms = "-r -endcl nonewfile=1 nocontrol=1";
   svl = (STRING) val;
   if (*svl == 0) sprintf(str,"%s %s /usr/info/b.help",PSVAL(editor),pms);
      else sprintf(str,"%s %s /usr/info/b.help 'find=>%s'",PSVAL(editor),pms,svl);

   DSsystem(str,FALSE);

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDexit -- handle termination commands		*/
/*							*/
/********************************************************/


int CMDexit(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register STRING pt;

   pt = (STRING) val;

   switch (cmd)
    { case CM_EXIT	: if (*PSVAL(afterfile) != 0)
			   { if (DSquery("Exit or Nextfile (E or N)? ","nN") >= 0)
				return CMDfile(CM_FNEXT,val,mrkfg);
			   };
			  if (pt == NULL || *pt == 0) EDstop("Exit");
			     else EDexec("Exit",pt);
			  break;

      case CM_ABORT	: EDabort("Abort",TRUE);
			  break;

      case CM_CRASH	: EDabort("Abort",MAYBE);
			  break;
    };

   return FALSE;
}





/********************************************************/
/*							*/
/*	CMDsystem -- system call command		*/
/*							*/
/********************************************************/


int CMDsystem(cmdnum,val,mrkfg)
   INT cmdnum;
   PARMVAL val;
   INT mrkfg;
{
   register STRING cmd;
   CHAR shcmd[32];
   STRING getenv();

   TMsave();

   cmd = val;

   if (cmd == NULL || *cmd == 0) {
      if (cmdnum == CM_STOP && PVAL(suspend)) cmd = "*STOP*";
       else
	{ cmd = PSVAL(shell);
	  if (cmd == NULL || *cmd == 0) cmd = "/bin/csh";
	  sprintf(shcmd,"%s -i",cmd);
	  cmd = shcmd;
	};
    }

   DSsystem(cmd,TRUE);

   return TRUE;
}



/********************************************************/
/*							*/
/*	CMDruncmd -- execute system function and insert */
/*	output at current position			*/
/*							*/
/********************************************************/


int CMDruncmd(cmdnum,val,mrkfg)
   INT cmdnum;
   PARMVAL val;
   INT mrkfg;
{
   FILE *cmdfp, *local_popen();
   register STRING cmd;
   INT	ch;

   TMsave();

   cmd = val;

   if (cmd != NULL && *cmd != 0) {
      BFclear(BF_Q);
      cmdfp = local_popen(cmd,"r");
      while ((ch = getc(cmdfp)) != EOF) {
	 if (ch == '\n') ch = BFC_EOL;
	 BFputch(ch,BF_Q);
      }
      pclose(cmdfp);
      MDins(BF_Q,1);
   };

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDcommand -- handle named command command	*/
/*							*/
/********************************************************/


int CMDcommand(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   extern LONG CMDTBL[];
   extern INT CMDTBLEN;
   extern BYTE CMDVTBL[];
   register INT vl,i,pm;
   register STRING pt;
   PARMVAL pvl;

   pt = (STRING) val;
   vl = i = 0;

   while (isalnum(*pt))
    { if (islower(*pt)) *pt = *pt-('a'-'A');
      if (i++ < 4) vl = (vl<<8)+ *pt++;
	 else ++pt;
    };

   for (i = 0; i < CMDTBLEN; ++i)
      if (vl == CMDTBL[i]) break;

   if (i >= CMDTBLEN)
    { DSerror("Illegal command name");
      return FALSE;
    };
   i = CMDVTBL[i];

   pm = CMDS(i).CMparm;
   if (*pt != ' ')
    { if (pm > 0) pvl = PARMS[pm].PMvalue;
	 else pvl = (PARMVAL) 0;
    }
   else Pcmdarg(pm,FALSE,TRUE,++pt,&pvl);

   docomm(i,pvl);

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDlang -- handle language specific commands	*/
/*							*/
/********************************************************/


int CMDlang(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register INT nm;

   nm = cmd-CM_0LANG;

   if (nm < 0 || nm >= LANUMCMD) ABORT;

   if (LAcmds[nm] == NULL) return FALSE;

   return (*(LAcmds[nm]))(cmd,val,mrkfg);
}





/********************************************************/
/*							*/
/*	CMDbufsave -- handle save/restore of buffers	*/
/*							*/
/********************************************************/


int CMDbufsave(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   register STRING st;

   switch(cmd)
    { case CM_BSAVE	:
		st = (STRING) val;
		BFsave(st);
		break;

      case CM_BRSTR	:
		st = (STRING) val;
		BFinit(st,FALSE);
		break;
    };

   return TRUE;
}






/********************************************************/
/*							*/
/*	CMDreset -- handle reset commands		*/
/*							*/
/********************************************************/


int CMDreset(cmd,val,mrkfg)
   INT cmd;
   PARMVAL val;
   INT mrkfg;
{
   switch(cmd)
    { case CM_RESET  : DSreset();
		       break;

      case CM_LRESET : DSlreset();
		       break;

      case CM_IGNORE : break;
    };

   return TRUE;
}






/* end of procmod.c */
