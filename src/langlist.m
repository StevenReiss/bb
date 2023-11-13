divert(-1)
/********************************************************/
/*							*/
/*		LANGLIST.M				*/
/*							*/
/*	This header defines the macros used to define	*/
/*	new language instances fairly simply.  The	*/
/*	user inserts an entry into the languages file	*/
/*	and includes the module with the language	*/
/*	specific routines in the load process		*/
/*							*/
/*	The languages file entry consists of:		*/
/*							*/
/*		LANG(ctrl-block name,			*/
/*		     parameter name for set,		*/
/*		     string of extensions,		*/
/*		     (item=rtn,...),			*/
/*		     ...,(item=rtn,...))		*/
/*							*/
/*	where rtn is a user routine name and item is	*/
/*	one of: 					*/
/*		FMT0 ... FMT5				*/
/*		TAB1 ... TAB5				*/
/*		CMD0 ... CMD9				*/
/*		IDLE, INDENT, INITIAL, FINAL		*/
/*		CHRTN, CHRS				*/
/*	and for chrs, rtn should actually be a char	*/
/*	string giving the special characters for	*/
/*	which CHRTN should be called			*/
/*							*/
/********************************************************/



define(`LANGCTR',0)

define(LANG,`divert(-1)
	LANG1

	define(`SNAME',ifelse($2,,LANG$1,$2))

	DOFIELD($4)
	DOFIELD($5)
	DOFIELD($6)
	DOFIELD($7)
	DOFIELD($8)
	DOFIELD($9)

divert(1)
static	BLANGINFO	SNAME = {
	{ XXFMT0, XXFMT1, XXFMT2, XXFMT3, XXFMT4 },

	{ XXTAB1, XXTAB2, XXTAB3, XXTAB4, XXTAB5 },

	{ XXCMD0, XXCMD1, XXCMD2, XXCMD3, XXCMD4,
	  XXCMD5, XXCMD6, XXCMD7, XXCMD8, XXCMD9 },

	XXIDLE,XXINDENT,XXINITIAL,XXFINAL,
	XXCHRTN,(CHAR) sizeof(XXCHRS),XXCHRS
};
divert(2)
&SNAME, 		divert(3)
"$1",                   divert(4)
$3,			divert(-1)
	LANG2
	ifelse(LANGCTR,0,`define(`DFLTLANG',SNAME)')
	define(`LANGCTR',incr(LANGCTR))
divert')


define(LANG1,`
	define(`XXFMT0',NULL)
	define(`XXFMT1',NULL)
	define(`XXFMT2',NULL)
	define(`XXFMT3',NULL)
	define(`XXFMT4',NULL)
	define(`XXTAB1',NULL)
	define(`XXTAB2',NULL)
	define(`XXTAB3',NULL)
	define(`XXTAB4',NULL)
	define(`XXTAB5',NULL)
	define(`XXCMD0',NULL)
	define(`XXCMD1',NULL)
	define(`XXCMD2',NULL)
	define(`XXCMD3',NULL)
	define(`XXCMD4',NULL)
	define(`XXCMD5',NULL)
	define(`XXCMD6',NULL)
	define(`XXCMD7',NULL)
	define(`XXCMD8',NULL)
	define(`XXCMD9',NULL)
	define(`XXIDLE',NULL)
	define(`XXINDENT',NULL)
	define(`XXINITIAL',NULL)
	define(`XXFINAL',NULL)
	define(`XXCHRTN',NULL)
	define(`XXCHRS',"")
')

define(LANG2,`
	DOEXTERN(XXFMT0)
	DOEXTERN(XXFMT1)
	DOEXTERN(XXFMT2)
	DOEXTERN(XXFMT3)
	DOEXTERN(XXFMT4)
	DOEXTERN(XXTAB1)
	DOEXTERN(XXTAB2)
	DOEXTERN(XXTAB3)
	DOEXTERN(XXTAB4)
	DOEXTERN(XXTAB5)
	DOEXTERN(XXCMD0)
	DOEXTERN(XXCMD1)
	DOEXTERN(XXCMD2)
	DOEXTERN(XXCMD3)
	DOEXTERN(XXCMD4)
	DOEXTERN(XXCMD5)
	DOEXTERN(XXCMD6)
	DOEXTERN(XXCMD7)
	DOEXTERN(XXCMD8)
	DOEXTERN(XXCMD9)
	DOEXTERN(XXIDLE)
	DOEXTERN(XXINDENT)
	DOEXTERN(XXINITIAL)
	DOEXTERN(XXFINAL)
	DOEXTERN(XXCHRTN)
')

define(DOFIELD,`
	ifelse($1,,,index($1,`,'),-1,`define(`XX'substr($1,0,index($1,`=')),
		substr($1,eval(index($1,`=')+1)))',`DOFLDS'$1)')

define(DOFLDS,`
	ifelse($1,,,DOFIELD($1))
	ifelse($2,,,DOFIELD($2))
	ifelse($3,,,DOFIELD($3))
	ifelse($4,,,DOFIELD($4))
	ifelse($5,,,DOFIELD($5))
	ifelse($6,,,DOFIELD($6))
	ifelse($7,,,DOFIELD($7))
	ifelse($8,,,DOFIELD($8))
	ifelse($9,,,DOFIELD($9))')

define(DOEXTERN,`
ifelse($1,`NULL',,`divert(5)
extern	int	$1();		divert(-1)')')





/* end of langlist.m */
divert

