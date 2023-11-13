/********************************************************/
/*							*/
/*		LANGMOD.H				*/
/*							*/
/*	This header file contains the definitions for	*/
/*	language-specific editor options		*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/



/********************************************************/
/*							*/
/*	Parameters					*/
/*							*/
/********************************************************/


#define LANUMFMT	5	/* # language format options	*/
#define LANUMTAB	5	/* # language tab options	*/
#define LANUMCMD	10	/* # language command options	*/
#define LANUMCHR	8	/* # special input character	*/




/********************************************************/
/*							*/
/*	Language definition structure			*/
/*							*/
/********************************************************/


typedef struct _LANGINFO {	/* language description info	*/
   FCT	LAIfmts[LANUMFMT];	/*    formatting routines	*/
   FCT	LAItabs[LANUMTAB];	/*    tab routines		*/
   FCT	LAIcmds[LANUMCMD];	/*    command routines		*/
   FCT	LAIidle;		/*    idle routine		*/
   FCT	LAIindent;		/*    indentation routine	*/
   FCT	LAIinit;		/*    initialization		*/
   FCT	LAIfini;		/*    finished with language	*/
   FCT	LAItyped;		/*    special character rtns	*/
   CHAR LAInchr;		/*    # of special characters	*/
   CHAR LAIchrs[LANUMCHR];	/*    special characters	*/
 }
BLANGINFO, *LANGINFO;






/********************************************************/
/*							*/
/*	Global language storage and access		*/
/*							*/
/********************************************************/


extern	LANGINFO	LAcurrent;	/* current language	*/

#define LAformats	LAcurrent->LAIfmts
#define LAtabs		LAcurrent->LAItabs
#define LAcmds		LAcurrent->LAIcmds
#define LAidle		LAcurrent->LAIidle
#define LAindent	LAcurrent->LAIindent
#define LAinit		LAcurrent->LAIinit
#define LAfini		LAcurrent->LAIfini
#define LAtyped 	LAcurrent->LAItyped
#define LAnchr		LAcurrent->LAInchr
#define LAchrs		LAcurrent->LAIchrs





/********************************************************/
/*							*/
/*	Entrys to language module			*/
/*							*/
/********************************************************/


extern	STRING	LAauto();	/* select new lang from file name	*/
extern	int	LAnewlang();	/* start a new language 		*/


extern	int	LCCinit();
extern	int	LCCfini();
extern	int	LCCwtab();
extern	int	LCCindent();

extern	void	DOstart();

extern	int	SEarch();



/* end of langmod.h */
