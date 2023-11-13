/********************************************************/
/*							*/
/*		DOMOD.C 				*/
/*							*/
/*	This module contains routines for handling	*/
/*	the do commands 				*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "bufmod.h"
#include "parms.h"
#include "cmds.h"




/********************************************************/
/*							*/
/*	External routines				*/
/*							*/
/********************************************************/





/********************************************************/
/*							*/
/*	Internal routines				*/
/*							*/
/********************************************************/


static	void	enddo();



/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


#define DOSTACK 10		/* recursive call stack size	*/

static	CHAR	dostack[DOSTACK];
static	INT	docount[DOSTACK];

static	CHAR	dostksz = 0;	/* current stack size	*/
static	CHAR	savbuf	= -1;	/* current save buffer	*/
static	CHAR	getbuf	= -1;	/* current active buffer*/
static	INT	getcnt	= 0;	/* current active cnt	*/

static	CHAR	stopfg = FALSE; /* stop immediately	*/





/********************************************************/
/*							*/
/*	DOset -- set up to accumulate a DO command	*/
/*							*/
/********************************************************/


void DOset(buf)
   INT buf;
{
   if (PVAL(dosave))		/* end of accumulation	*/
    { if (savbuf != buf && buf != BF_DO)
	 DSerror("Do compiled; wrong terminator");
       else if (buf != BF_DO) DSerror("-bDo compiled");
       else DSerror("-b-dDo compiled");
      BFrepch(CM_IGNORE,savbuf);
      BFputch(DCH_END,savbuf);
      BFset(savbuf);
      PSET(dosave,FALSE);
      savbuf = -1;
      return;
    };

   PSET(dosave,TRUE);
   savbuf = buf;
   BFclear(savbuf);
   BFputch(DCH_START,savbuf);
   DSerror("-bStart compiling DO");

   return;
}






/********************************************************/
/*							*/
/*	DOstart -- start DO execution			*/
/*							*/
/********************************************************/


void DOstart(buf,cnt)
   INT buf;
   INT cnt;
{
   if (PVAL(dosave) && buf == savbuf)	/* end rather than start ? */
    { DOset(buf);
      return;
    };

   BFset(buf);
   if (BFget(buf) != DCH_START)
    { MDins(buf,cnt);
      return;
    };

   if (PVAL(douse))
    { if (dostksz == DOSTACK-1) 	/* nested call	*/
       { PSET(silent,FALSE);
	 DSerror("DO stack overflow:  recursive DOs");
	 dostksz = 0;
	 PSET(douse,FALSE);
	 return;
       };
      dostack[dostksz] = getbuf;
      docount[dostksz] = getcnt;
      ++dostksz;
    }
   else
    { dostksz = 0;
      stopfg = FALSE;
      PSET(douse,TRUE);
      DSerror("-b-dDo executing");
      PSET(silent,TRUE);
    };

   getbuf = buf;
   getcnt = cnt;
   getbuf = buf;
   BFset(buf);
   if (BFget(buf) != DCH_START) ABORT;
}






/********************************************************/
/*							*/
/*	DOget -- get next character for current DO	*/
/*							*/
/********************************************************/


int DOget()
{
   register INT ch;

   if (!PVAL(douse)) return 0;

   if (stopfg)
    { stopfg = FALSE;
      enddo();
      return CM_IGNORE;
    };

   ch = 0;
   while (getcnt > 0 && (ch = BFget(getbuf)) == DCH_END)
      if (--getcnt > 0)
       { BFset(getbuf);
	 if (BFget(getbuf) != DCH_START) ABORT;
       }
      else if (dostksz > 0)
       { --dostksz;
	 getbuf = dostack[dostksz];
	 getcnt = docount[dostksz];
       }
      else
       { enddo();
	 return 0;
       };

   return ch;
}





/********************************************************/
/*							*/
/*	DOstop -- stop do immediately			*/
/*							*/
/********************************************************/


void DOstop()
{
   if (stopfg) PSET(douse,FALSE);

   PSET(silent,FALSE);

   stopfg = TRUE;
}






/********************************************************/
/*							*/
/*	DOsave -- save character as part of DO		*/
/*							*/
/********************************************************/


void DOsave(ch)
   INT ch;
{
   if (!PVAL(dosave) || ch == 0) return;

   BFputch(ch,savbuf);
}





/********************************************************/
/*							*/
/*	enddo -- finish do, restore normal mode 	*/
/*							*/
/********************************************************/


static void
enddo()
{
   PSET(douse,FALSE);
   PSET(silent,FALSE);

   HSmark();

   DSerror("-b-d-k");

   DSwrite();
   DSshow();
}






/* end of domod.c */
