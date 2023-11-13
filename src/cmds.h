/********************************************************/
/*							*/
/*		CMDS.H	 (derived from cmds.hm) 	*/
/*							*/
/*	This file contains the definitions of all the	*/
/*   editor commands and command related information.	*/
/*   It is generated from cmds.hm and the file commands */
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University					*/








/********************************************************/
/*							*/
/*	Command flags					*/
/*							*/
/********************************************************/


typedef INT	CMFLAGS;

#define CMFusemark	0x1	/* use marking info	*/
#define CMFerrmark	0x2	/* error if marking -ign */
#define CMFokmark	0x4	/* ok during marking	*/
#define CMFwarnmark	0x8	/* warn if marking	*/
#define CMFgomark	0x10	/* start marking	*/
#define CMFnosave	0x20	/* never save parm dflt */
#define CMFabort	0x40	/* illegal		*/
#define CMFcmode	0x80	/* cursor mode command	*/
#define CMFtyping	0x100	/* type-in type command */
#define CMFargs 	0x200	/* can have arguments	*/
#define CMFdoend	0x400	/* end a do compilation */
#define CMFignore	0x800	/* ignore arguments	*/
#define CMFflipsave	0x1000	/* flip save times	*/
#define CMFalways	CMFflipsave|CMFnosave

				/* command categorys	*/
#define CMFwindow	CMFokmark|CMFgomark
#define CMFsearch	CMFokmark|CMFalways
#define CMFgoto 	CMFokmark
#define CMFcursor	CMFgomark|CMFokmark|CMFcmode
#define CMFmove 	CMFgomark|CMFokmark
#define CMFtab		CMFgomark|CMFokmark
#define CMFdel		CMFusemark
#define CMFins		CMFusemark
#define CMFpick 	CMFusemark
#define CMFput		CMFerrmark
#define CMFswap 	CMFusemark
#define CMFaddch	CMFtyping|CMFerrmark
#define CMFfile 	CMFerrmark|CMFnosave
#define CMFreset	CMFokmark|CMFignore
#define CMFdo		CMFokmark|CMFnosave|CMFargs
#define CMFexit 	CMFwarnmark|CMFdoend
#define CMFhelp 	CMFokmark|CMFnosave
#define CMFundo 	CMFokmark|CMFnosave

#define CMFlang 	CMFokmark





/********************************************************/
/*							*/
/*	Command value definitions			*/
/*							*/
/********************************************************/


				/* RHS options		*/
#define RHS_CONT	0	/*    continue typing	*/
#define RHS_FIXED	1	/*    fixed margin	*/

				/* end marker options	*/
#define EOF_NONE	0	/*    none		*/
#define EOF_PAGE	1	/*    at page bottom	*/
#define EOF_LINE	2	/*    at last line	*/






/********************************************************/
/*							*/
/*	Command name definitions			*/
/*							*/
/********************************************************/


#define CM_FIRST	0x80	/* first command value	*/


#define CM_PPAGE (CM_FIRST+0) 	
#define CM_MPAGE (CM_FIRST+1) 	
#define CM_PLINE (CM_FIRST+2) 	
#define CM_MLINE (CM_FIRST+3) 	
#define CM_RSLIDE (CM_FIRST+4) 	
#define CM_LSLIDE (CM_FIRST+5) 	
#define CM_GOTO (CM_FIRST+6) 	
#define CM_GOBEG (CM_FIRST+7) 	
#define CM_GOEND (CM_FIRST+8) 	
#define CM_SVPOS (CM_FIRST+9) 	
#define CM_PSRCH (CM_FIRST+10) 	
#define CM_MSRCH (CM_FIRST+11) 	
#define CM_PRSRCH (CM_FIRST+12) 	
#define CM_MRSRCH (CM_FIRST+13) 	
#define CM_PFSRCH (CM_FIRST+14) 	
#define CM_MFSRCH (CM_FIRST+15) 	
#define CM_PBSRCH (CM_FIRST+16) 	
#define CM_MBSRCH (CM_FIRST+17) 	
#define CM_RIGHT (CM_FIRST+18) 	
#define CM_LEFT (CM_FIRST+19) 	
#define CM_UP (CM_FIRST+20) 	
#define CM_DOWN (CM_FIRST+21) 	
#define CM_BS (CM_FIRST+22) 	
#define CM_RET (CM_FIRST+23) 	
#define CM_BRET (CM_FIRST+24) 	
#define CM_IND (CM_FIRST+25) 	
#define CM_NL (CM_FIRST+26) 	
#define CM_HOME (CM_FIRST+27) 	
#define CM_BHOME (CM_FIRST+28) 	
#define CM_VTAB (CM_FIRST+29) 	
#define CM_0TAB (CM_FIRST+30) 	
#define CM_0BTAB (CM_FIRST+31) 	
#define CM_1TAB (CM_FIRST+32) 	
#define CM_1BTAB (CM_FIRST+33) 	
#define CM_2TAB (CM_FIRST+34) 	
#define CM_2BTAB (CM_FIRST+35) 	
#define CM_3TAB (CM_FIRST+36) 	
#define CM_3BTAB (CM_FIRST+37) 	
#define CM_4TAB (CM_FIRST+38) 	
#define CM_4BTAB (CM_FIRST+39) 	
#define CM_5TAB (CM_FIRST+40) 	
#define CM_5BTAB (CM_FIRST+41) 	
#define CM_ABSPOS (CM_FIRST+42) 	
#define CM_MRKPOS (CM_FIRST+43) 	
#define CM_MRKAT (CM_FIRST+44) 	
#define CM_CDEL (CM_FIRST+45) 	
#define CM_LDEL (CM_FIRST+46) 	
#define CM_1DEL (CM_FIRST+47) 	
#define CM_2DEL (CM_FIRST+48) 	
#define CM_3DEL (CM_FIRST+49) 	
#define CM_4DEL (CM_FIRST+50) 	
#define CM_5DEL (CM_FIRST+51) 	
#define CM_CINS (CM_FIRST+52) 	
#define CM_LINS (CM_FIRST+53) 	
#define CM_CPICK (CM_FIRST+54) 	
#define CM_LPICK (CM_FIRST+55) 	
#define CM_1PICK (CM_FIRST+56) 	
#define CM_2PICK (CM_FIRST+57) 	
#define CM_3PICK (CM_FIRST+58) 	
#define CM_4PICK (CM_FIRST+59) 	
#define CM_5PICK (CM_FIRST+60) 	
#define CM_CPUT (CM_FIRST+61) 	
#define CM_LPUT (CM_FIRST+62) 	
#define CM_1PUT (CM_FIRST+63) 	
#define CM_2PUT (CM_FIRST+64) 	
#define CM_3PUT (CM_FIRST+65) 	
#define CM_4PUT (CM_FIRST+66) 	
#define CM_5PUT (CM_FIRST+67) 	
#define CM_CSWAP (CM_FIRST+68) 	
#define CM_LSWAP (CM_FIRST+69) 	
#define CM_1SWAP (CM_FIRST+70) 	
#define CM_2SWAP (CM_FIRST+71) 	
#define CM_3SWAP (CM_FIRST+72) 	
#define CM_4SWAP (CM_FIRST+73) 	
#define CM_5SWAP (CM_FIRST+74) 	
#define CM_COPEN (CM_FIRST+75) 	
#define CM_LOPEN (CM_FIRST+76) 	
#define CM_JOIN (CM_FIRST+77) 	
#define CM_INDENT (CM_FIRST+78) 	
#define CM_0LANG (CM_FIRST+79) 	
#define CM_1LANG (CM_FIRST+80) 	
#define CM_2LANG (CM_FIRST+81) 	
#define CM_3LANG (CM_FIRST+82) 	
#define CM_4LANG (CM_FIRST+83) 	
#define CM_5LANG (CM_FIRST+84) 	
#define CM_6LANG (CM_FIRST+85) 	
#define CM_7LANG (CM_FIRST+86) 	
#define CM_8LANG (CM_FIRST+87) 	
#define CM_9LANG (CM_FIRST+88) 	
#define CM_EXIT (CM_FIRST+89) 	
#define CM_ABORT (CM_FIRST+90) 	
#define CM_CRASH (CM_FIRST+91) 	
#define CM_RSTRT (CM_FIRST+92) 	
#define CM_SAVE (CM_FIRST+93) 	
#define CM_PARM (CM_FIRST+94) 	
#define CM_FSET (CM_FIRST+95) 	
#define CM_FMAKE (CM_FIRST+96) 	
#define CM_FNEXT (CM_FIRST+97) 	
#define CM_FALT (CM_FIRST+98) 	
#define CM_SHOW (CM_FIRST+99) 	
#define CM_RESET (CM_FIRST+100) 	
#define CM_LRESET (CM_FIRST+101) 	
#define CM_IGNORE (CM_FIRST+102) 	
#define CM_SYSTEM (CM_FIRST+103) 	
#define CM_STOP (CM_FIRST+104) 	
#define CM_COMMAND (CM_FIRST+105) 	
#define CM_UNDO (CM_FIRST+106) 	
#define CM_ENTER (CM_FIRST+107) 	
#define CM_MARK (CM_FIRST+108) 	
#define CM_KNOCK (CM_FIRST+109) 	
#define CM_BSAVE (CM_FIRST+110) 	
#define CM_BRSTR (CM_FIRST+111) 	
#define CM_DOSET (CM_FIRST+112) 	
#define CM_DO (CM_FIRST+113) 	
#define CM_HELP (CM_FIRST+114) 	
#define CM_RUNCMD (CM_FIRST+115) 	
#define CM_INTR (CM_FIRST+116) 	

#define CM_LAST 	CM_FIRST+117


			/* OTHER SPECIAL CHARACTERS	*/

#define BFC_EOF 	0xf0	/* end of buffer	*/
#define BFC_EOL 	0xf1	/* eol in buffer	*/

#define CHR_BS		0xf2	/* internal backspace	*/
#define CHR_BSDEL	0xf3	/* internal bs-delete	*/

#define ICH_ENTER	0xf4	/* input editing -- ENTER */
#define ICH_DELIM	0xf5	/*    Argument delimiter  */
#define ICH_WARN	0xf6	/*    ignore but warn	  */
#define ICH_END 	0xf7	/*    end of argument	  */
#define ICH_BS		0xf8	/*    backspace 	  */
#define ICH_LAST	0xf9	/*    use last argument   */
#define ICH_IGNORE	0	/*    ignore		  */

#define DCH_START	0xfa	/* start of DO buffer	*/
#define DCH_END 	BFC_EOF /* end of DO buffer	*/






/********************************************************/
/*							*/
/*	Command name scan tables			*/
/*							*/
/********************************************************/


#ifdef DISPMOD

LONG	CMDTBL[] = {


(('P'<<24)|('P'<<16)|('A'<<8)|'G'),				
(('M'<<24)|('P'<<16)|('A'<<8)|'G'),				
(('P'<<24)|('L'<<16)|('I'<<8)|'N'),				
(('M'<<24)|('L'<<16)|('I'<<8)|'N'),				
(('R'<<24)|('S'<<16)|('L'<<8)|'I'),				
(('L'<<24)|('S'<<16)|('L'<<8)|'I'),				
(('G'<<24)|('O'<<16)|('T'<<8)|'O'),				
(('G'<<24)|('O'<<16)|('B'<<8)|'E'),				
(('G'<<24)|('O'<<16)|('E'<<8)|'N'),				
(('S'<<24)|('V'<<16)|('P'<<8)|'O'),				
(('P'<<24)|('S'<<16)|('R'<<8)|'C'),				
(('M'<<24)|('S'<<16)|('R'<<8)|'C'),				
(('P'<<24)|('R'<<16)|('S'<<8)|'R'),				
(('M'<<24)|('R'<<16)|('S'<<8)|'R'),				
(('P'<<24)|('F'<<16)|('S'<<8)|'R'),				
(('M'<<24)|('F'<<16)|('S'<<8)|'R'),				
(('P'<<24)|('B'<<16)|('S'<<8)|'R'),				
(('M'<<24)|('B'<<16)|('S'<<8)|'R'),				
(('R'<<24)|('I'<<16)|('G'<<8)|'H'),				
(('L'<<24)|('E'<<16)|('F'<<8)|'T'),				
('U'<<8|'P'),				
(('D'<<24)|('O'<<16)|('W'<<8)|'N'),				
('B'<<8|'S'),				
(('R'<<16)|('E'<<8)|'T'),				
(('B'<<24)|('R'<<16)|('E'<<8)|'T'),				
(('I'<<16)|('N'<<8)|'D'),				
('N'<<8|'L'),				
(('H'<<24)|('O'<<16)|('M'<<8)|'E'),				
(('B'<<24)|('H'<<16)|('O'<<8)|'M'),				
(('V'<<24)|('T'<<16)|('A'<<8)|'B'),				
(('0'<<24)|('T'<<16)|('A'<<8)|'B'),				
(('0'<<24)|('B'<<16)|('T'<<8)|'A'),				
(('1'<<24)|('T'<<16)|('A'<<8)|'B'),				
(('1'<<24)|('B'<<16)|('T'<<8)|'A'),				
(('2'<<24)|('T'<<16)|('A'<<8)|'B'),				
(('2'<<24)|('B'<<16)|('T'<<8)|'A'),				
(('3'<<24)|('T'<<16)|('A'<<8)|'B'),				
(('3'<<24)|('B'<<16)|('T'<<8)|'A'),				
(('4'<<24)|('T'<<16)|('A'<<8)|'B'),				
(('4'<<24)|('B'<<16)|('T'<<8)|'A'),				
(('5'<<24)|('T'<<16)|('A'<<8)|'B'),				
(('5'<<24)|('B'<<16)|('T'<<8)|'A'),				
(('A'<<24)|('B'<<16)|('S'<<8)|'P'),				
(('M'<<24)|('R'<<16)|('K'<<8)|'P'),				
(('M'<<24)|('R'<<16)|('K'<<8)|'A'),				
(('C'<<24)|('D'<<16)|('E'<<8)|'L'),				
(('L'<<24)|('D'<<16)|('E'<<8)|'L'),				
(('1'<<24)|('D'<<16)|('E'<<8)|'L'),				
(('2'<<24)|('D'<<16)|('E'<<8)|'L'),				
(('3'<<24)|('D'<<16)|('E'<<8)|'L'),				
(('4'<<24)|('D'<<16)|('E'<<8)|'L'),				
(('5'<<24)|('D'<<16)|('E'<<8)|'L'),				
(('C'<<24)|('I'<<16)|('N'<<8)|'S'),				
(('L'<<24)|('I'<<16)|('N'<<8)|'S'),				
(('C'<<24)|('P'<<16)|('I'<<8)|'C'),				
(('L'<<24)|('P'<<16)|('I'<<8)|'C'),				
(('1'<<24)|('P'<<16)|('I'<<8)|'C'),				
(('2'<<24)|('P'<<16)|('I'<<8)|'C'),				
(('3'<<24)|('P'<<16)|('I'<<8)|'C'),				
(('4'<<24)|('P'<<16)|('I'<<8)|'C'),				
(('5'<<24)|('P'<<16)|('I'<<8)|'C'),				
(('C'<<24)|('P'<<16)|('U'<<8)|'T'),				
(('L'<<24)|('P'<<16)|('U'<<8)|'T'),				
(('1'<<24)|('P'<<16)|('U'<<8)|'T'),				
(('2'<<24)|('P'<<16)|('U'<<8)|'T'),				
(('3'<<24)|('P'<<16)|('U'<<8)|'T'),				
(('4'<<24)|('P'<<16)|('U'<<8)|'T'),				
(('5'<<24)|('P'<<16)|('U'<<8)|'T'),				
(('C'<<24)|('S'<<16)|('W'<<8)|'A'),				
(('L'<<24)|('S'<<16)|('W'<<8)|'A'),				
(('1'<<24)|('S'<<16)|('W'<<8)|'A'),				
(('2'<<24)|('S'<<16)|('W'<<8)|'A'),				
(('3'<<24)|('S'<<16)|('W'<<8)|'A'),				
(('4'<<24)|('S'<<16)|('W'<<8)|'A'),				
(('5'<<24)|('S'<<16)|('W'<<8)|'A'),				
(('C'<<24)|('O'<<16)|('P'<<8)|'E'),				
(('L'<<24)|('O'<<16)|('P'<<8)|'E'),				
(('J'<<24)|('O'<<16)|('I'<<8)|'N'),				
(('I'<<24)|('N'<<16)|('D'<<8)|'E'),				
(('0'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('1'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('2'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('3'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('4'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('5'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('6'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('7'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('8'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('9'<<24)|('L'<<16)|('A'<<8)|'N'),				
(('E'<<24)|('X'<<16)|('I'<<8)|'T'),				
(('A'<<24)|('B'<<16)|('O'<<8)|'R'),				
(('C'<<24)|('R'<<16)|('A'<<8)|'S'),				
(('R'<<24)|('S'<<16)|('T'<<8)|'R'),				
(('S'<<24)|('A'<<16)|('V'<<8)|'E'),				
(('P'<<24)|('A'<<16)|('R'<<8)|'M'),				
(('F'<<24)|('S'<<16)|('E'<<8)|'T'),				
(('F'<<24)|('M'<<16)|('A'<<8)|'K'),				
(('F'<<24)|('N'<<16)|('E'<<8)|'X'),				
(('F'<<24)|('A'<<16)|('L'<<8)|'T'),				
(('S'<<24)|('H'<<16)|('O'<<8)|'W'),				
(('R'<<24)|('E'<<16)|('S'<<8)|'E'),				
(('L'<<24)|('R'<<16)|('E'<<8)|'S'),				
(('I'<<24)|('G'<<16)|('N'<<8)|'O'),				
(('S'<<24)|('Y'<<16)|('S'<<8)|'T'),				
(('S'<<24)|('T'<<16)|('O'<<8)|'P'),				
(('C'<<24)|('O'<<16)|('M'<<8)|'M'),				
(('U'<<24)|('N'<<16)|('D'<<8)|'O'),				
(('E'<<24)|('N'<<16)|('T'<<8)|'E'),				
(('M'<<24)|('A'<<16)|('R'<<8)|'K'),				
(('K'<<24)|('N'<<16)|('O'<<8)|'C'),				
(('B'<<24)|('S'<<16)|('A'<<8)|'V'),				
(('B'<<24)|('R'<<16)|('S'<<8)|'T'),				
(('D'<<24)|('O'<<16)|('S'<<8)|'E'),				
('D'<<8|'O'),				
(('H'<<24)|('E'<<16)|('L'<<8)|'P'),				
(('R'<<24)|('U'<<16)|('N'<<8)|'C'),				
(('I'<<24)|('N'<<16)|('T'<<8)|'R'),				

};

INT	CMDTBLEN = (sizeof CMDTBL)/(sizeof(LONG));


TEXT	CMDVTBL[] = {


CM_PPAGE,					
CM_MPAGE,					
CM_PLINE,					
CM_MLINE,					
CM_RSLIDE,					
CM_LSLIDE,					
CM_GOTO,					
CM_GOBEG,					
CM_GOEND,					
CM_SVPOS,					
CM_PSRCH,					
CM_MSRCH,					
CM_PRSRCH,					
CM_MRSRCH,					
CM_PFSRCH,					
CM_MFSRCH,					
CM_PBSRCH,					
CM_MBSRCH,					
CM_RIGHT,					
CM_LEFT,					
CM_UP,					
CM_DOWN,					
CM_BS,					
CM_RET,					
CM_BRET,					
CM_IND,					
CM_NL,					
CM_HOME,					
CM_BHOME,					
CM_VTAB,					
CM_0TAB,					
CM_0BTAB,					
CM_1TAB,					
CM_1BTAB,					
CM_2TAB,					
CM_2BTAB,					
CM_3TAB,					
CM_3BTAB,					
CM_4TAB,					
CM_4BTAB,					
CM_5TAB,					
CM_5BTAB,					
CM_ABSPOS,					
CM_MRKPOS,					
CM_MRKAT,					
CM_CDEL,					
CM_LDEL,					
CM_1DEL,					
CM_2DEL,					
CM_3DEL,					
CM_4DEL,					
CM_5DEL,					
CM_CINS,					
CM_LINS,					
CM_CPICK,					
CM_LPICK,					
CM_1PICK,					
CM_2PICK,					
CM_3PICK,					
CM_4PICK,					
CM_5PICK,					
CM_CPUT,					
CM_LPUT,					
CM_1PUT,					
CM_2PUT,					
CM_3PUT,					
CM_4PUT,					
CM_5PUT,					
CM_CSWAP,					
CM_LSWAP,					
CM_1SWAP,					
CM_2SWAP,					
CM_3SWAP,					
CM_4SWAP,					
CM_5SWAP,					
CM_COPEN,					
CM_LOPEN,					
CM_JOIN,					
CM_INDENT,					
CM_0LANG,					
CM_1LANG,					
CM_2LANG,					
CM_3LANG,					
CM_4LANG,					
CM_5LANG,					
CM_6LANG,					
CM_7LANG,					
CM_8LANG,					
CM_9LANG,					
CM_EXIT,					
CM_ABORT,					
CM_CRASH,					
CM_RSTRT,					
CM_SAVE,					
CM_PARM,					
CM_FSET,					
CM_FMAKE,					
CM_FNEXT,					
CM_FALT,					
CM_SHOW,					
CM_RESET,					
CM_LRESET,					
CM_IGNORE,					
CM_SYSTEM,					
CM_STOP,					
CM_COMMAND,					
CM_UNDO,					
CM_ENTER,					
CM_MARK,					
CM_KNOCK,					
CM_BSAVE,					
CM_BRSTR,					
CM_DOSET,					
CM_DO,					
CM_HELP,					
CM_RUNCMD,					
CM_INTR,					

};

#endif





/********************************************************/
/*							*/
/*	Command action tables				*/
/*							*/
/********************************************************/


#ifdef CMDMOD


				/* command routines	*/

extern	INT	CMDwindow();		
extern	INT	CMDmargin();		
extern	INT	CMDgoto();		
extern	INT	CMDsearch();		
extern	INT	CMDcursor();		
extern	INT	CMDtabs();		
extern	INT	CMDdelete();		
extern	INT	CMDinsert();		
extern	INT	CMDpick();		
extern	INT	CMDswap();		
extern	INT	CMDreset();		
extern	INT	CMDjoin();		
extern	INT	CMDindent();		
extern	INT	CMDlang();		
extern	INT	CMDexit();		
extern	INT	CMDfile();		
extern	INT	CMDshow();		
extern	INT	CMDsystem();		
extern	INT	CMDcommand();		
extern	INT	CMDundo();		
extern	INT	CMDbufsave();		
extern	INT	CMDdo();		
extern	INT	CMDhelp();		
extern	INT	CMDruncmd();		

#endif



typedef struct _CMTBL { 	/* table structure	*/
   INT	CMparm; 		/*    parameter value	*/
   INT	(*CMrtn)();		/*    processing rtn	*/
   CMFLAGS CMflags;		/*    flags		*/
} BCMTBL, *CMTBL;

extern	BCMTBL	CMDTB[];


#ifdef CMDMOD

#define P_0 -1			/* default parameter	*/

       BCMTBL CMDTB[] = {


{ P_Apage , CMDwindow , CMFwindow },			
{ P_Apage , CMDwindow , CMFwindow },			
{ P_Aline , CMDwindow , CMFwindow },			
{ P_Aline , CMDwindow , CMFwindow },			
{ P_Aslide , CMDmargin , CMFwindow },			
{ P_Aslide , CMDmargin , CMFwindow },			
{ P_Agoto , CMDgoto , CMFgoto },			
{ P_Agotobeg , CMDgoto , CMFgoto },			
{ P_Agotoend , CMDgoto , CMFgoto },			
{ P_Asavpos , CMDgoto , CMFokmark },			
{ P_Asearch , CMDsearch , CMFsearch },			
{ P_Asearch , CMDsearch , CMFsearch },			
{ P_Arsearch , CMDsearch , CMFsearch },			
{ P_Arsearch , CMDsearch , CMFsearch },			
{ P_Afsearch , CMDsearch , CMFsearch },			
{ P_Afsearch , CMDsearch , CMFsearch },			
{ P_Absearch , CMDsearch , CMFokmark },			
{ P_Absearch , CMDsearch , CMFokmark },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFmove },			
{ P_0 , CMDcursor , CMFmove },			
{ P_0 , CMDcursor , CMFmove },			
{ P_0 , CMDcursor , CMFmove },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFmove },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDtabs , CMFtab },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_0 , CMDcursor , CMFcursor },			
{ P_Acedit , CMDdelete , CMFdel },			
{ P_Aledit , CMDdelete , CMFdel },			
{ P_A1edit , CMDdelete , CMFdel },			
{ P_A2edit , CMDdelete , CMFdel },			
{ P_A3edit , CMDdelete , CMFdel },			
{ P_A4edit , CMDdelete , CMFdel },			
{ P_A5edit , CMDdelete , CMFdel },			
{ P_Acedit , CMDinsert , CMFins },			
{ P_Aledit , CMDinsert , CMFins },			
{ P_Acedit , CMDpick , CMFpick },			
{ P_Aledit , CMDpick , CMFpick },			
{ P_A1edit , CMDpick , CMFpick },			
{ P_A2edit , CMDpick , CMFpick },			
{ P_A3edit , CMDpick , CMFpick },			
{ P_A4edit , CMDpick , CMFpick },			
{ P_A5edit , CMDpick , CMFpick },			
{ P_Acedit , CMDinsert , CMFput },			
{ P_Aledit , CMDinsert , CMFput },			
{ P_A1edit , CMDinsert , CMFput },			
{ P_A2edit , CMDinsert , CMFput },			
{ P_A3edit , CMDinsert , CMFput },			
{ P_A4edit , CMDinsert , CMFput },			
{ P_A5edit , CMDinsert , CMFput },			
{ P_Acedit , CMDswap , CMFswap },			
{ P_Aledit , CMDswap , CMFswap },			
{ P_A1edit , CMDswap , CMFswap },			
{ P_A2edit , CMDswap , CMFswap },			
{ P_A3edit , CMDswap , CMFswap },			
{ P_A4edit , CMDswap , CMFswap },			
{ P_A5edit , CMDswap , CMFswap },			
{ P_0 , CMDreset , CMFaddch },			
{ P_0 , CMDinsert , CMFerrmark },			
{ P_Ajoin , CMDjoin , CMFdel },			
{ P_Aindent , CMDindent , CMFusemark },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_0 , CMDlang , CMFlang },			
{ P_Aexit , CMDexit , CMFexit|CMFalways },			
{ P_0 , CMDexit , CMFexit },			
{ P_0 , CMDexit , CMFexit },			
{ P_0 , CMDfile , CMFexit },			
{ P_0 , CMDfile , CMFexit },			
{ P_Afile , CMDfile , CMFokmark|CMFnosave },			
{ P_Afile , CMDfile , CMFfile },			
{ P_Afile , CMDfile , CMFfile },			
{ P_0 , CMDfile , CMFfile },			
{ P_Afile , CMDfile , CMFfile },			
{ P_Ashow , CMDshow , CMFokmark },			
{ P_0 , CMDreset , CMFreset },			
{ P_0 , CMDreset , CMFreset },			
{ P_0 , CMDreset , CMFreset },			
{ P_Asystem , CMDsystem , CMFokmark },			
{ P_Asystem , CMDsystem , CMFokmark },			
{ P_Acommand , CMDcommand , CMFokmark|CMFalways },			
{ P_Aundo , CMDundo , CMFundo },			
{ P_0 , CMDreset , CMFabort },			
{ P_0 , CMDreset , CMFabort },			
{ P_0 , CMDreset , CMFabort },			
{ P_Abfsave , CMDbufsave , CMFokmark },			
{ P_Abfsave , CMDbufsave , CMFokmark },			
{ P_Ado , CMDdo , CMFdo },			
{ P_Ado , CMDdo , CMFdo },			
{ P_Afile , CMDhelp , CMFhelp },			
{ P_Asystem , CMDruncmd , CMFput },			
{ P_0 , CMDexit , CMFexit },			

};

#endif


#define CMDS(cmd) CMDTB[cmd-CM_FIRST]








/* end of cmds.h */
