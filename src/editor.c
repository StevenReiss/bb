/********************************************************/
/*							*/
/*		EDITOR.C				*/
/*							*/
/*	This module is the mainline for the revised	*/
/*	Brown editor.  It contains those control	*/
/*	routines that are specific to initiation or	*/
/*	termination of the editor.			*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/



#include "global.h"
#include "editor.h"
#include "parms.h"
#include "cmds.h"
#include "tempmod.h"
#include "filemod.h"
#include <signal.h>
#include <stdlib.h>
#ifdef MALLOCTEST
#include <mcheck.h>
#endif


/********************************************************/
/*							*/
/*	Internal Routines				*/
/*							*/
/********************************************************/


static	void	initsearch();


/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


	INT	EDstatus = 0;	/* termination status	*/






/********************************************************/
/*							*/
/*	main -- main program of editor			*/
/*							*/
/********************************************************/


int main(argc,argv)
   INT argc;
   STRING argv[];
{
   register STRING pm;
   register INT i;
   STRING getenv();
   register INT suspfg;
   register int (*sig)();
   struct sigaction act;

#ifdef MALLOCTEST
   mtrace();
#endif
			/********************************/
			/* set proper priority/user	*/
   if (PVAL(priority) != 20) {
      if (geteuid() != 0) PSET(priority,20);
       else
	{ nice(PVAL(priority)-20);
	  setuid(getuid());
	}
    }

   if (geteuid() == 0) setuid(getuid());

#ifdef SIGTSTP
   sigaction(SIGTSTP,NULL,&act);
   suspfg = (act.sa_handler == SIG_DFL);
#else
   suspfg = FALSE;
#endif

#ifdef M_CHECK_ACTION
   mallopt(M_CHECK_ACTION,1);
#endif

			/********************************/
			/* get initial parameters	*/
   MNinit();
   Pinit();
   if ((pm = getenv("bPARM")) != NULL) Parm(pm,TRUE);
   Pgetctrl();
   for (i = 1; i < argc; ++i) Parm(argv[i],FALSE);
   Parmdo();		/* set up initial parameters	*/
   PSETI(suspend,suspfg);/*    and open input file	 */

			/********************************/
			/* initial search		*/
   if (*PSVAL(find) != 0) initsearch();

			/********************************/
   DSwrite();		/* initialize display		*/
   DSshow();

			/********************************/
			/* command loop 		*/
   CMloop();

			/********************************/
			/* CMloop never returns 	*/
   ABORT;
}




/********************************************************/
/*							*/
/*	EDabort -- abort the editor cleanly		*/
/*							*/
/********************************************************/


void EDabort(msg,okfg)
   STRING msg;
   INT okfg;
{
   static INT retryfg = 0;

   if (EDstatus == 0)
    { EDstatus = 1;
      TMsave();
      PSET(bufoutfile,"");
      if (okfg == TRUE) 	/* if user requested, then remove temp files */
       { BFfini();
	 FLclose();
	 TMclose();
       };
    };

   if (retryfg++ > 5) exit(EDstatus);

   DSfini();

   printf("%s.\n\r\n",msg);

   if (okfg == FALSE) abort();

   exit(EDstatus);
}






/********************************************************/
/*							*/
/*	EDstop -- halt the editor normally		*/
/*							*/
/********************************************************/


void EDstop(msg)
   STRING msg;
{
   if (!Pfini()) return;	/* bad output status -- don't halt */
   BFfini();

   DSfini();

   printf("%s.\n\r\n",msg);

   exit(EDstatus);
}





/********************************************************/
/*							*/
/*	EDexec -- exit editor and execute command	*/
/*							*/
/********************************************************/


void EDexec(msg,cmd)
   STRING msg;
   STRING cmd;
{
   if (!Pfini()) return;
   BFfini();

   DSfini();

   printf("%s.\n\n$ %s\n",msg,cmd);

   if (PVAL(priority) < 20) nice(20-PVAL(priority));

   execl("/bin/csh","csh","-c",cmd,0);

   EDabort("Can't execute command",TRUE);
}





/********************************************************/
/*							*/
/*	EDfixfilename -- handle initial ~ in file names */
/*							*/
/********************************************************/


STRING
EDfixfilename(fn)
   STRING fn;
{
   static CHAR buf[FNSIZE];

   if (fn[0] != '~') sprintf(buf,"%s",fn);
   else if (fn[1] == '/') sprintf(buf,"%s%s",PSVAL(home),&fn[1]);
   else sprintf(buf,"/mnt/%s",&fn[1]);

   return buf;
}






/********************************************************/
/*							*/
/*	initsearch -- do initial string search		*/
/*							*/
/********************************************************/


static void
initsearch()
{
   PARMVAL vl;

   PSET(curline,0);
   PSET(curchar,0);

   Pcmdarg(P_Asearch,TRUE,TRUE,PVAL(find),&vl);
   CMDsearch(CM_PSRCH,vl,FALSE);

   Parmdo();
}






/* end of editor.c */
