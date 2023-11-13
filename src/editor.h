/********************************************************/
/*							*/
/*		EDITOR.H				*/
/*							*/
/*   This file contains definitions global to the	*/
/*   editor.  These are generally common parameters	*/
/*   or types that are used by several modules, as	*/
/*   well as global data structures.			*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/



/********************************************************/
/*							*/
/*	Parameters					*/
/*							*/
/********************************************************/


#define FNSIZE	256		/* file name string size*/
#define LBLKSIZE (16*1024)	/* line # block size	*/
#define LBSZLOG  (4+10) 	/* log of LBLKSIZE	*/
#define LINESIZE (1024*1024)	/* max line size	*/
#define IOPGSIZE 8192		/* I/O page size	*/
#define NMLBLKS  65536		/* # line number blocks */
#define NMLBLKS_HUGE  655360	/* # line number blocks in huge mode */
#define FILESTACK   3		/* # previous files	*/
#define PFILESIZE 512		/* file parm area size	*/
#define PCTRLSIZE 512		/* ctrl parm area size	*/
#define LOGSIZEADDR 3		/* log of sizeof(ADDR)	*/




/********************************************************/
/*							*/
/*	Types						*/
/*							*/
/********************************************************/


typedef long DATA;		/* general data values	*/

typedef unsigned long LINE;	/* line number		*/
typedef unsigned long long ADDR;     /* line number pointer  */
typedef unsigned char TEXT;	/* text values		*/
typedef unsigned long PTR;	/* holder of pointer	*/




/********************************************************/
/*							*/
/*	Common routines 				*/
/*							*/
/********************************************************/


extern	void	DSerror();	/* display error message*/
extern	INT	DSquery();	/* display error query	*/
extern	void	DSreply();	/* get user reply str	*/
extern	void	DSwrite();
extern	void	DSshow();
extern	void	DSfini();
extern	void	DSreset();
extern	void	DSlreset();
extern	void	DSsystem();
extern	void	DSfrom();
extern	void	DSline();
extern	void	DStype();
extern	void	DSabort();
extern	void	DSslowdisp();
extern	void	DSpos();
extern	void	DSmark();
extern	void	DSunmark();

extern	void	MNinit();
extern	void	MNtypein();
extern	void	MNreplace();
extern	void	MNinsert();
extern	void	MNdelete();
extern	void	MNopen();
extern	void	MNposition();
extern	void	MNsync();

extern	void	MDins();
extern	void	MDdel();
extern	void	MDtype();
extern	void	MDtypend();

extern	void	CMloop();
extern	void	CMdomark();

extern	void	BFinit();
extern	void	BFfini();
extern	void	BFfree();
extern	void	BFundo();
extern	void	BFrepch();

extern	int	CMDsearch();
extern	int	CMDcursor();

extern	void	DOset();
extern	void	DOstop();
extern	int	DOget();
extern	void	DOsave();

extern	int	INchar();
extern	void	INline();
extern	void	INdone();
extern	void	INinit();
extern	int	INsetup();
extern	int	INcmdchar();
extern	void	INunput();

extern	void	TRbells();
extern	void	TRtype();
extern	void	TRdisp();
extern	void	TRdisp_goto();
extern	void	TRmark();
extern	void	TRinsn();
extern	void	TRins();
extern	void	TRend();
extern	void	TRcmark();
extern	int	TRwinset();
extern	void	TRgoto();
extern	void	TRprint();
extern	void	TRclear();
extern	int	TRinfo();
extern	void	TRnew();

extern	void	EDabort();	/* abort cleanly	*/
extern	STRING	EDfixfilename();/* handle ~ in filename */
extern	void	EDstop();
extern	void	EDexec();





/********************************************************/
/*							*/
/*	Miscellaneous definitions			*/
/*							*/
/********************************************************/


#define ABORT	abort();	/* abort the editor	*/






/* end of editor.h */
