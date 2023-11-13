/********************************************************/
/*							*/
/*		TEMP.H					*/
/*							*/
/*	This file contains the definitions for tempor-	*/
/*   ary file processing.  These include parameters as	*/
/*   well as the necessary structures.			*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/






/********************************************************/
/*							*/
/*	Parameters					*/
/*							*/
/********************************************************/


#define TNMBLKS     0x7FFFFFF0	/* max # temp file blks */
#define TLBLKS		1	/* # initial line blks	*/
#define TBUFSIZE	1024	/* buffer size		*/
#define TBUFSZLG	10	/* log (base 2) buf sz	*/
#define TIBUFS		20	/* number of input bufs */

#define FBPERLB (LBLKSIZE/TBUFSIZE)	  /* sz(line bk)*/





/********************************************************/
/*							*/
/*	Types						*/
/*							*/
/********************************************************/


typedef unsigned int BLOCK;	/* block pointer    */






/********************************************************/
/*							*/
/*	Data structures 				*/
/*							*/
/********************************************************/


			/*  Header -- this structure is */
			/*     placed at the start of	*/
			/*     the temp file.  It has	*/
			/*     enough definitions, etc. */
			/*     to allow recovery in the */
			/*     event of a crash 	*/


struct _HEADER {
   time_t THDtime;		/* time file last written    */
   short  THDuser;		/* user id of file	     */
   int	  THDlines;		/* number of lines in file   */
   char   THDfile[FNSIZE];	/* current file name	     */
   char   THDchanged;		/* file changed flag	     */
   BLOCK  THDlblk;		/* start of line blocks      */
};


extern	struct	_HEADER HEADER; /* actual header	     */



/********************************************************/
/*							*/
/*	Calls						*/
/*							*/
/********************************************************/

extern	void	TMinit();
extern	void	TMclose();
extern	void	TMget();
extern	ADDR	TMput();
extern	void	TMsync();
extern	void	TMsave();





/* end of tempmod.h */
