/********************************************************/
/*							*/
/*		HISTMOD.C				*/
/*							*/
/*	This module maintains an potentially unlimited	*/
/*    and general history of the editing session.  It	*/
/*    uses dynamic storage to store previous values of	*/
/*    parameters, lines and buffers.			*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "linemod.h"
#include "bufmod.h"
#include "parms.h"



/********************************************************/
/*							*/
/*	External routines				*/
/*							*/
/********************************************************/


#ifndef __STDC__
#define PALLOC(typ) ((typ)malloc(sizeof(B/**/typ)))
#else
#define PALLOC(typ) ((typ)malloc(sizeof(B ## typ)))
#endif


/********************************************************/
/*							*/
/*	Internal routines				*/
/*							*/
/********************************************************/


static	void	histfree();
static	void	histlink();



/********************************************************/
/*							*/
/*	Data structures 				*/
/*							*/
/********************************************************/


typedef enum {		/* history node types		*/
   HTYVOID,		/*    dummy marker		*/
   HTYHMARK,		/*    history stop marker	*/
   HTYSPARM,		/*    string parameter		*/
   HTYPARM,		/*    normal parameter		*/
   HTYLINE,		/*    line modification 	*/
   HTYMARK,		/*    line marker		*/
   HTYBUF		/*    buffer modification	*/
}
   HSTTYPE;





typedef struct _HISTORY *HISTORY;

typedef struct _HISTORY /* history node description	*/
{  HISTORY Hlast;	/*    pointer to previous one	*/
   HISTORY Hnext;	/*    pointer to next one	*/
   HSTTYPE Htype;	/*    type of node		*/
} BHISTORY;



/********************************************************/
/*							*/
/*	history subtypes -- these are alternative	*/
/*	definitions of the history block for the	*/
/*	various types of nodes				*/
/*							*/
/********************************************************/


typedef struct _HBKPOS		/* position marker	*/
{  HISTORY Hlast;	/*    pointer to previous one	*/
   HISTORY Hnext;	/*    pointer to next one	*/
   HSTTYPE Htype;	/*    type of node		*/
   CHAR  Hdouse;	/*    do in use flag		*/
   DATA  Hline; 	/*    line number		*/
   INT	 Hchar; 	/*    character number		*/
} BHBKPOS,*HBKPOS;


typedef struct _HBKSPARM	/* string parameter	*/
{  HISTORY Hlast;	/*    pointer to previous one	*/
   HISTORY Hnext;	/*    pointer to next one	*/
   HSTTYPE Htype;	/*    type of node		*/
   SHORT HSPparm;	/*    parameter number		*/
   CHAR HSPdata[1];	/*    string value		*/
} BHBKSPARM,*HBKSPARM;


typedef struct _HBKPARM /* numeric parameter		*/
{  HISTORY Hlast;	/*    pointer to previous one	*/
   HISTORY Hnext;	/*    pointer to next one	*/
   HSTTYPE Htype;	/*    type of node		*/
   SHORT HPMparm;	/*    parameter number		*/
   PARMVAL HPMdata;	/*    parameter value		*/
} BHBKPARM,*HBKPARM;


typedef struct _HBKLINE /* line changes 		*/
{  HISTORY Hlast;	/*    pointer to previous one	*/
   HISTORY Hnext;	/*    pointer to next one	*/
   HSTTYPE Htype;	/*    type of node		*/
   HISTLINE HLNtype;	/*    type of change		*/
   LINE HLNline;	/*    line number of change	*/
   ADDR HLNaddr;	/*    temp addr of old contents */
} BHBKLINE,*HBKLINE;


typedef struct _HBKMARK /* line marker changes		*/
{  HISTORY Hlast;	/*    pointer to previous one	*/
   HISTORY Hnext;	/*    pointer to next one	*/
   HSTTYPE Htype;	/*    type of node		*/
   BYTE HMKfile;	/*    file id			*/
   INT HMKid;		/*    marker id 		*/
   LINE HMKline;	/*    line number		*/
} BHBKMARK,*HBKMARK;


typedef struct _HBKBUF	/* buffer changes		*/
{  HISTORY Hlast;	/*    pointer to previous one	*/
   HISTORY Hnext;	/*    pointer to next one	*/
   HSTTYPE Htype;	/*    type of node		*/
   CHAR HBFid;		/*    buffer id 		*/
   ADDR HBFtext;	/*    text location of old buf	*/
} BHBKBUF,*HBKBUF;





/********************************************************/
/*							*/
/*	Internal routines				*/
/*							*/
/********************************************************/


#define USEHIST (HSready && !HSundoing && PVAL(history) > 0)






/********************************************************/
/*							*/
/*	Storage declarations				*/
/*							*/
/********************************************************/


static	HISTORY HSlast; 	/* last history node	*/
static	HISTORY HSfirst;	/* first history node	*/
static	BOOL	HSready = FALSE;/* history ready flag	*/
static	DATA	HScount;	/* # history nodes	*/
static	BOOL	HSundoing = FALSE; /* currently in undo */




/********************************************************/
/*							*/
/*	HSclear -- start new/first history		*/
/*							*/
/********************************************************/


void HSclear()
{
   if (HSready)
      while (HSfirst != NULL) histfree();

   HSfirst = HSlast = NULL;
   HScount = 0;
   HSready = TRUE;
   HSundoing = FALSE;

   HSmark();
}





/********************************************************/
/*							*/
/*	HSmark -- create marker node			*/
/*							*/
/********************************************************/


void HSmark()
{
   register HBKPOS h;

   if (!USEHIST) return;

   h = PALLOC(HBKPOS);
   h->Htype = HTYHMARK;
   h->Hdouse = PVAL(douse);
   h->Hline = PDVAL(curline);
   h->Hchar = PVAL(curchar);

   histlink(h);
}





/********************************************************/
/*							*/
/*	HSparmstr -- save string parameter		*/
/*							*/
/********************************************************/


void HSparmstr(parm,str)
   INT parm;			/* parameter number	*/
   STRING str;			/* old value		*/
{
   register HBKSPARM h;

   if (!USEHIST) return;

   h = (HBKSPARM) (malloc(sizeof(BHBKSPARM)+strlen(str)));
   h->Htype = HTYSPARM;
   h->HSPparm = parm;
   our_strcpy(h->HSPdata,str);

   histlink(h);
}





/********************************************************/
/*							*/
/*	HSparm -- save non-string parameter		*/
/*							*/
/********************************************************/


void HSparm(parm,val)
   INT parm;			/* parameter number	*/
   PARMVAL val; 		/* parameter value	*/
{
   register HBKPARM h;

   if (!USEHIST) return;

   h = PALLOC(HBKPARM);
   h->Htype = HTYPARM;
   h->HPMparm = parm;
   h->HPMdata = val;

   histlink(h);
}





/********************************************************/
/*							*/
/*	HSline -- save line information 		*/
/*							*/
/********************************************************/


void HSline(cls,line,addr)
   HISTLINE cls;		/* type of modification */
   LINE line;			/* line number		*/
   ADDR addr;			/* address of old cnts	*/
{
   register HBKLINE h;

   if (!USEHIST) return;

   h = PALLOC(HBKLINE);
   h->Htype = HTYLINE;
   h->HLNtype = cls;
   h->HLNline = line;
   h->HLNaddr = addr;

   histlink(h);
}





/********************************************************/
/*							*/
/*	HSlmark -- save line marker			*/
/*							*/
/********************************************************/


void HSlmark(id,line,file)
   INT id;		/* marker id			*/
   LINE line;		/* line number			*/
   INT file;		/* file id			*/
{
   register HBKMARK h;

   if (!USEHIST) return;

   h = PALLOC(HBKMARK);
   h->Htype = HTYMARK;
   h->HMKfile = file;
   h->HMKid = id;
   h->HMKline = line;

   histlink(h);
}





/********************************************************/
/*							*/
/*	HSbuf -- save buffer pointer			*/
/*							*/
/********************************************************/


void HSbuf(buf,addr)
   INT buf;			/* buffer id		*/
   ADDR addr;			 /* buffer cnts pointer  */
{
   register HBKBUF h;

   if (!USEHIST)
    { BFfree(addr);
      return;
    };

   h = PALLOC(HBKBUF);
   h->Htype = HTYBUF;
   h->HBFid = buf;
   h->HBFtext = addr;

   histlink(h);
}





/********************************************************/
/*							*/
/*	HSundo -- undo history to last marker		*/
/*							*/
/********************************************************/


int HSundo()
{
   register HISTORY h;
   register HSTTYPE hty;

   if (!USEHIST) return FALSE;
   HSundoing = TRUE;

   do
    { h = HSlast;
      if (h == NULL)
       { HSundoing = FALSE;
	 return FALSE;
       };

      HSlast = h->Hlast;
      if (HSlast == NULL) HSfirst = NULL;
	 else HSlast->Hnext = NULL;

      hty = h->Htype;

      switch(h->Htype)
       { case HTYSPARM	:
		Pset(((HBKSPARM)h)->HSPparm,((HBKSPARM)h)->HSPdata);
		break;

	 case HTYPARM	:
		Pset(((HBKPARM)h)->HPMparm,((HBKPARM)h)->HPMdata);
		break;

	 case HTYLINE	:
		LNundo(((HBKLINE)h)->HLNtype,
		       ((HBKLINE)h)->HLNline,
		       ((HBKLINE)h)->HLNaddr);
		break;

	 case HTYMARK	:
		LNmark(((HBKMARK)h)->HMKid,((HBKMARK)h)->HMKline);
		break;

	 case HTYBUF	:
		BFundo(((HBKBUF)h)->HBFid,((HBKBUF)h)->HBFtext);
		break;

	 case HTYHMARK	:
		PSET(curline,((HBKPOS)h)->Hline);
		PSETI(curchar,((HBKPOS)h)->Hchar);
		if (((HBKPOS)h)->Hdouse && !PVAL(douse)) hty = HTYVOID;
		break;

	 default	:
		break;
       };

      free(h);
    }
   while (hty != HTYHMARK);

   HSundoing = FALSE;

   return TRUE;
}





/********************************************************/
/*							*/
/*	histlink -- add a new history block to list	*/
/*							*/
/********************************************************/


static void
histlink(h)
   HISTORY h;
{
   while (HScount >= PVAL(history)) histfree();

   if (HSlast == NULL) HSfirst = h;
      else HSlast->Hnext = h;

   h->Hlast = HSlast;
   HSlast = h;
   h->Hnext = NULL;
   if (h->Htype == HTYHMARK && !PVAL(douse)) ++HScount;
}





/********************************************************/
/*							*/
/*	histfree -- release history block		*/
/*							*/
/********************************************************/


static void
histfree()
{
   register HISTORY h;

   h = HSfirst;
   if (h == NULL)
    { HScount = 0;
      return;
    };

   HSfirst = h->Hnext;
   if (HSfirst == NULL) HSlast = NULL;
      else HSfirst->Hlast = NULL;

   switch(h->Htype)
    { case HTYBUF	:
		BFfree(((HBKBUF)h)->HBFtext);
		break;

      case HTYHMARK	:
		if (!((HBKPOS)h)->Hdouse || PVAL(douse)) --HScount;
		break;

      default		:
		break;
    };

   free(h);
}





/* end of histmod.c */
