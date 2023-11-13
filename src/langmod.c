/********************************************************/
/*							*/
/*		LANGMOD.C				*/
/*							*/
/*	This module contains the common routines for	*/
/*	switching between separate languages		*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "langmod.h"
#include "parms.h"






















/* end of languages */




extern STRING match();



/********************************************************/
/*							*/
/*	External routines				*/
/*							*/
/********************************************************/



extern	int	LDFwtab();		
extern	int	LDFetab();		
extern	int	LDFbtab();		
extern	int	LDFstab();		
extern	int	LDFhtab();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFindent();		
extern	int	LDFinit();		
extern	int	LDFfini();		
extern	int	LCCwtab();		
extern	int	LDFetab();		
extern	int	LDFbtab();		
extern	int	LDFstab();		
extern	int	LDFhtab();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LCCindent();		
extern	int	LCCinit();		
extern	int	LCCfini();		
extern	int	LCCwtab();		
extern	int	LDFetab();		
extern	int	LDFbtab();		
extern	int	LDFstab();		
extern	int	LDFhtab();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	JAVAindent();		
extern	int	JAVAinit();		
extern	int	JAVAfini();		
extern	int	LCOwtab();		
extern	int	LDFetab();		
extern	int	LDFbtab();		
extern	int	LDFstab();		
extern	int	LDFhtab();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LCOindent();		
extern	int	LCOinit();		
extern	int	LCOfini();		
extern	int	LLIwtab();		
extern	int	LDFetab();		
extern	int	LDFbtab();		
extern	int	LDFstab();		
extern	int	LDFhtab();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIcmds();		
extern	int	LLIindent();		
extern	int	LLIinit();		
extern	int	LLIfini();		
extern	int	LDFwtab();		
extern	int	LDFetab();		
extern	int	LDFbtab();		
extern	int	LDFstab();		
extern	int	LDFhtab();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFindent();		
extern	int	LRFinit();		
extern	int	LRFfini();		
extern	int	LCCwtab();		
extern	int	LDFetab();		
extern	int	LDFbtab();		
extern	int	LDFstab();		
extern	int	LDFhtab();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LDFcmds();		
extern	int	LF77indent();		
extern	int	LF77init();		
extern	int	LF77fini();		





/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/

				/* language hdr blocks	*/

static	BLANGINFO	dfltLANG = {
	{ NULL, NULL, NULL, NULL, NULL },

	{ LDFwtab, LDFetab, LDFbtab, LDFstab, LDFhtab },

	{ LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds,
	  LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds },

	NULL,LDFindent,LDFinit,LDFfini,
	NULL,(CHAR) sizeof(""),""
};

static	BLANGINFO	ccccLANG = {
	{ NULL, NULL, NULL, NULL, NULL },

	{ LCCwtab, LDFetab, LDFbtab, LDFstab, LDFhtab },

	{ LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds,
	  LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds },

	NULL,LCCindent,LCCinit,LCCfini,
	NULL,(CHAR) sizeof(""),""
};

static	BLANGINFO	javaLANG = {
	{ NULL, NULL, NULL, NULL, NULL },

	{ LCCwtab, LDFetab, LDFbtab, LDFstab, LDFhtab },

	{ LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds,
	  LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds },

	NULL,JAVAindent,JAVAinit,JAVAfini,
	NULL,(CHAR) sizeof(""),""
};

static	BLANGINFO	copsLANG = {
	{ NULL, NULL, NULL, NULL, NULL },

	{ LCOwtab, LDFetab, LDFbtab, LDFstab, LDFhtab },

	{ LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds,
	  LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds },

	NULL,LCOindent,LCOinit,LCOfini,
	NULL,(CHAR) sizeof(""),""
};

static	BLANGINFO	lispLANG = {
	{ NULL, NULL, NULL, NULL, NULL },

	{ LLIwtab, LDFetab, LDFbtab, LDFstab, LDFhtab },

	{ LLIcmds, LLIcmds, LLIcmds, LLIcmds, LLIcmds,
	  LLIcmds, LLIcmds, LLIcmds, LLIcmds, LLIcmds },

	NULL,LLIindent,LLIinit,LLIfini,
	NULL,(CHAR) sizeof(""),""
};

static	BLANGINFO	roffLANG = {
	{ NULL, NULL, NULL, NULL, NULL },

	{ LDFwtab, LDFetab, LDFbtab, LDFstab, LDFhtab },

	{ LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds,
	  LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds },

	NULL,LDFindent,LRFinit,LRFfini,
	NULL,(CHAR) sizeof(""),""
};

static	BLANGINFO	fortLANG = {
	{ NULL, NULL, NULL, NULL, NULL },

	{ LCCwtab, LDFetab, LDFbtab, LDFstab, LDFhtab },

	{ LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds,
	  LDFcmds, LDFcmds, LDFcmds, LDFcmds, LDFcmds },

	NULL,LF77indent,LF77init,LF77fini,
	NULL,(CHAR) sizeof(""),""
};



static	LANGINFO LAtable[] = {	/* table of languages	*/

&dfltLANG, 		
&ccccLANG, 		
&javaLANG, 		
&copsLANG, 		
&lispLANG, 		
&roffLANG, 		
&fortLANG, 		
};



	LANGINFO LAcurrent = &dfltLANG; /* current language	*/


static	STRING	LAnames[] = {	/* names of languages	*/

"default",                   
"c",                   
"java",                   
"cops",                   
"lisp",                   
"roff",                   
"f77",                   
};



static	STRING LAsfxs[] = {	/* extensions for language */

"",			
".c.cm.h.H.hm.y.hi.C.l.L.Y.cc.cxx.hxx.cpp.html.pl.ino.pde.",			
".java.jj.jjt.js.taiga.decaf.php.jsp.ascus.remix.",			
".cops.",			
".lisp.",			
".n.r.me.ms.t.1.2.3.4.5.6.7.8.tex.txt.",			
".f.f77.",			
};


#define NUMLANG 7 	/* number of languages	*/






/********************************************************/
/*							*/
/*	LAauto -- select language by extension		*/
/*							*/
/********************************************************/


STRING
LAauto()
{
   register STRING pt;
   CHAR buf[32];
   register INT i;

   pt = PSVAL(filetail);
   pt = strrchr(pt,'.');
   if (pt == NULL) pt = "..";
    else
     { sprintf(buf,".%s.",++pt);
       pt = buf;
     };

   for (i = 0; i < NUMLANG; ++i)
      if (match(LAsfxs[i],pt) != NULL)
	 return LAnames[i];

   return NULL;
}





/********************************************************/
/*							*/
/*	LAnewlang -- change to a new language		*/
/*							*/
/********************************************************/


int LAnewlang()
{
   register STRING pt;
   register INT ln,i;

   pt = PSVAL(curlanguage);
   ln = strlen(pt);

   for (i = 0; i < NUMLANG; ++i)	/* find language	*/
      if (ln == 0 || strncmp(pt,LAnames[i],ln) == 0) break;

   if (i >= NUMLANG)
    { DSerror("Illegal language name");
      i = 0;
    };

   if (LAfini != NULL) (*LAfini)();
   LAcurrent = LAtable[i];
   if (LAinit != NULL) (*LAinit)();

   return TRUE;
}





/* end of langmod.cm */
