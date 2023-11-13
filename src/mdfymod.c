/********************************************************/
/*							*/
/*		MDFYMOD.C				*/
/*							*/
/*	This module is called to make any changes to	*/
/*	the file.  It provides entrys to insert and	*/
/*	delete/pick text, as well as single character	*/
/*	modifications either in insert or replace	*/
/*	mode.						*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "lbuff.h"
#include "bufmod.h"
#include "parms.h"
#include "cmds.h"
#include "linemod.h"



/********************************************************/
/*							*/
/*	External routines				*/
/*							*/
/********************************************************/


static	int	MDiget();	/* return line/insert	*/
static	void	MDiput();	/* accept line/insert	*/
static	int	MDdget();	/* return line/delete	*/
static	void	MDdput();	/* accept line/delete	*/




#define LBFEDITS LBF_STREAM|LBF_FMT3	/* editing codes	*/





/********************************************************/
/*							*/
/*	Data type definitions				*/
/*							*/
/********************************************************/


typedef enum {			/* modify states	*/
	MDSinit,		/*    initial		*/
	MDSnorm,		/*    normal		*/
	MDSend			/*    final		*/
} MODSTATE;





/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


	TEXT	MDbuf[LINESIZE];	/* internal line buffer */

static	INT	md_x;			/* current x position	*/
static	INT	md_y;			/* current y position	*/
static	INT	md_cnt; 		/* current line count	*/
static	INT	md_buf; 		/* current buffer id	*/
static	INT	md_len; 		/* current length	*/
static	MODSTATE md_state;		/* current state	*/

static	BOOL	typingfg;		/* type-in mode flag	*/





/********************************************************/
/*							*/
/*	MDins -- insert buffer @ cur pos (cnt) times	*/
/*							*/
/********************************************************/


void MDins(buf,cnt)
   INT buf;			/* buffer to insert	*/
   INT cnt;			/* # of times to repeat */
{
   register INT lcnt;
   register DATA line;

   md_x = PVAL(curchar);	/* set up global vars	*/
   md_cnt = cnt;
   md_buf = buf;
   md_state = MDSinit;

   LBmove(LNget,PDVAL(curline),1l,   /* get cur line */
	  MDiput,0l,0l,
	  LBF_SMAX|LBF_SPARM|LBF_SIGN|LBF_FMT0);

   line = PDVAL(curline);
   lcnt = LBmove(MDiget,0l,0l,	     /* save insert  */
		 LNput,PDVAL(curline),0l,
		 LBF_DFRST|LBF_DINCP|LBF_DFRST|LBFEDITS);

   PSET(nochange,FALSE);

   if (lcnt == 1 && !PVAL(curstream)) DSline(line);/* fix display  */
      else DSfrom(line);
}





/********************************************************/
/*							*/
/*	MDiput -- retrieve line to insert in		*/
/*							*/
/*	Called from LBmove for insertions		*/
/*							*/
/********************************************************/


static void
MDiput()
{
   register INT ln;

   strcpy((char *) MDbuf,(char *) LBlinebuf);

   ln = strlen((char *) MDbuf); 	 /* space extend to cur pos */
   while (ln < md_x) MDbuf[ln++] = ' ';
   MDbuf[ln] = 0;
}





/********************************************************/
/*							*/
/*	MDiget -- return line with insertions		*/
/*							*/
/*	Called by LBmove to get updated line		*/
/*							*/
/********************************************************/


static int
MDiget()
{
   register INT i;
   register TEXT *pt,*opt;
   register TEXT ch;

   if (md_state == MDSinit) BFset(md_buf);
      else if (md_state == MDSend) return FALSE;
   md_state = MDSnorm;

   pt = MDbuf;
   opt = LBlinebuf;

   for (i = 0; i < md_x; ++i) *opt++ = *pt++;	/* split line */
   if (md_x != 0) {
      our_strcpy(MDbuf,pt);
    }

   for ( ; ; )
      switch(ch = BFget(md_buf))	/* read from buffer	*/
       { case BFC_EOL	:		/* -- new line		*/
		*opt = 0;
		md_x = 0;
		return TRUE;

	 case BFC_EOF	:		/* -- end of buffer	*/
		if (--md_cnt > 0)
		   BFset(md_buf);	/* repeat if cnt > 0	*/
		else
		 { if ((int)(opt-LBlinebuf+strlen((char *) MDbuf)) >= LINESIZE)
		    { pt = MDbuf;
		      while (opt-LBlinebuf <= LINESIZE) *opt++ = *pt++;
		      our_strcpy((char *) MDbuf,(char *) pt);
		      DSerror("Edited line too long; split in two");
		      return TRUE;
		    };
		   our_strcpy((char *) opt,(char *) MDbuf);   /* remainder of buffer  */
		   md_state = MDSend;
		   return TRUE; 	/* almost done		*/
		 };
		break;

	 default	:		/* normal character	*/
		*opt++ = ch;
		if (opt-LBlinebuf >= LINESIZE-1)
		 { DSerror("Edited line too long; split in two");
		   *opt = 0;
		   md_x = 0;
		   return TRUE;
		 };
		break;
       };
}






/********************************************************/
/*							*/
/*	MDdel -- delete/pick indicated block		*/
/*							*/
/********************************************************/


void MDdel(buf,lcnt,ccnt,delfg)
   INT buf;			/* buffer to put text into	*/
   INT lcnt;			/* # lines to delete/pick	*/
   INT ccnt;			/* # characters to delete/pick	*/
				/* (after #lines are deleted)	*/
   BOOL delfg;			/* delete->TRUE, pick->FALSE	*/
{
   register DATA line;

   md_x = PVAL(curchar);	/* set global variables */
   md_y = PVAL(curline);
   md_cnt = ccnt;
   md_buf = buf;
   md_state = MDSinit;

   BFclear(buf);
   MDbuf[0] = 0;
   md_len = 0;

   if (lcnt > 0) {		/* delete lines 	*/
      if (delfg)
	 LBmove(LNgetdel,(DATA)md_y,(DATA)lcnt,
		MDdput,0l,0l,
		LBF_SMAX|LBF_SPARM|LBF_SIGN|LBF_FMT0);
       else
	{ LBmove(LNget,(DATA)md_y,(DATA)lcnt,
		MDdput,0l,0l,
		LBF_SMAX|LBF_SPARM|LBF_SINCP|LBF_SIGN|LBF_FMT0);
	  md_y += lcnt;
	};
    }

   line = PDVAL(curline);
   md_state = MDSend;		/* delete characters	*/
   if (ccnt > 0 || PVAL(curchar) != 0)
    { LBmove(LNget,(DATA)md_y,1l,
	     MDdput,0l,0l,
	     LBF_SMAX|LBF_SPARM|LBF_SINCP|LBF_SIGN|LBF_FMT0);
      if (delfg)
	 LBmove(MDdget,0l,1l,	/* replace line 	*/
		LNput,md_y,0l,
		LBF_SMAX|LBF_DFRST|LBF_DINCP|LBFEDITS);
    }
   else BFputch(BFC_EOF,buf);

   BFset(buf);

   if (delfg)			/* fix screen		*/
    { PSET(nochange,FALSE);
      if (lcnt == 0 && !PVAL(curstream)) DSline(line);
	 else DSfrom(line);
    };
}





/********************************************************/
/*							*/
/*	MDdput -- get line to be deleted		*/
/*							*/
/*	Called by LBmove for deletions			*/
/*							*/
/********************************************************/


static void
MDdput()
{
   register INT ln;
   register TEXT *pt,*opt;

   ln = strlen((char *) LBlinebuf);	 /* space extend line	 */
   while (ln < md_x) LBlinebuf[ln++] = ' ';
   LBlinebuf[ln] = 0;

   pt = &MDbuf[md_len];
   opt = LBlinebuf;

   for (ln = 0; ln < md_x; ++ln)	/* save line up to deletion */
    { *pt++ = *opt++;
      ++md_len;
    };
   md_x = 0;

   if (md_state != MDSend)
    { while (*opt != 0) BFputch(*opt++,md_buf);
      BFputch(BFC_EOL,md_buf);		/* save line in buffer	*/
    }
   else
    { while (*opt != 0 && md_cnt-- > 0)
	 BFputch(*opt++,md_buf);	/* save last line in buf */
      BFputch(BFC_EOF,md_buf);
      while (*opt != 0)
       { *pt++ = *opt++;
	 ++md_len;
       };
    };

   *pt = 0;
}





/********************************************************/
/*							*/
/*	MDdget -- replace line after deletion		*/
/*							*/
/*	Called by LBmove to get corrected line		*/
/*							*/
/********************************************************/


static int
MDdget()
{
   strcpy((char *) LBlinebuf,(char *) MDbuf);

   return TRUE;
}





/********************************************************/
/*							*/
/*	MDtype -- single character modification 	*/
/*							*/
/*	This routine is called with a single character	*/
/*	to insert/replace at the current position.  It	*/
/*	can be efficiently called repeatedly, but the	*/
/*	routine MDtypend must be called to actually	*/
/*	save corrections.  Line position should not	*/
/*	change between calls.				*/
/*							*/
/********************************************************/


void MDtype(ch,insfg)
   INT ch;			/* character to use	*/
   BOOL insfg;			/* insert if TRUE	*/
{
   register INT i,x;

   if (!typingfg)		/* handle first time	*/
    { md_x = PVAL(curchar);
      LBmove(LNget,PDVAL(curline),1l,
	     MDiput,0l,0l,
	     LBF_SMAX|LBF_SPARM|LBF_SIGN|LBF_FMT0);
      md_len = strlen((char *) MDbuf);
      typingfg = TRUE;
      PSET(nochange,FALSE);
    };

   x = PVAL(curchar);

   if (ch == CHR_BS || ch == CHR_BSDEL) /* handle backspaces	*/
    { if (insfg)
       { for (i = x; i < md_len; ++i)
	    MDbuf[i] = MDbuf[i+1];
	 --md_len;
       }
      else if (PVAL(backspace) || ch == CHR_BSDEL)
	 if (x == md_len-1) --md_len;
	    else MDbuf[x] = ' ';
      else
       { DSshow();
	 return;
       };
    }
   else 			/* handle characters	*/
    { if (insfg)
       { for (i = strlen((char *) MDbuf); i > x; --i)
	     MDbuf[i] = MDbuf[i-1];
	 md_len++;
       };
      MDbuf[x] = ch;
      if (x >= md_len) md_len = x+1;
    };

   MDbuf[md_len] = 0;

   MNtypein(PVAL(curline),MDbuf);

   DStype(ch,insfg);		/* fixup display	*/
}





/********************************************************/
/*							*/
/*	MDtypend -- end single character edit mode	*/
/*							*/
/********************************************************/


void MDtypend()
{
   if (!typingfg) return;

   LBmove(MDdget,0l,1l, 	/* replace line 	*/
	  LNput,PDVAL(curline),0l,
	  LBF_SMAX|LBF_DFRST|LBF_DINCP|LBFEDITS);

   typingfg = FALSE;

   DSline(PDVAL(curline));	 /* fixup up screen	 */
}





/* end of mdfymod.c */
