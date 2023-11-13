divert(-1)
/********************************************************/
/*							*/
/*		PARMLIST.M				*/
/*							*/
/*	This file contains the definitions for the	*/
/*   parameter table and associated items.  It is to	*/
/*   be included into the various parameter routines	*/
/*   in order to use the table definitions or decls.	*/
/*							*/
/********************************************************/


/********************************************************/
/*							*/
/*	Macros (m4) are defined here for defining the	*/
/*	actual editor parameters.  There is one macro	*/
/*	for each particular type of parameter.	Each	*/
/*	macro has a set of common arguments, followed	*/
/*	by a set of type-specific arguments.  The	*/
/*	common arguments are:				*/
/*							*/
/*		name	-- parameter name		*/
/*		sg len	-- # significant chars in name	*/
/*		value	-- initial value		*/
/*		set dflt-- default value on set 	*/
/*		flags	-- parameter flags		*/
/*		call	-- routine to call after set	*/
/*		file set-- parameter to use to set this */
/*				one after file change	*/
/*							*/
/*	For strings, the additional parameters are:	*/
/*							*/
/*		size	-- max size of string value	*/
/*							*/
/*	For numbers, the additional parameters are:	*/
/*							*/
/*		lbd	-- lower bound (>=)		*/
/*		ubd	-- upper bound (<=)		*/
/*							*/
/********************************************************/


define(`NUMPARMS',0)
define(`NUMFILE',0)
define(`NUMCTRL',0)
define(`FILESTSZ',0)
define(`CTRLSTSZ',0)

define(DEFRTN,`ifelse(index(RTNS,*$1*),-1,`divert(4)
static	int	$1 ();
define(`RTNS',RTNS*$1*)
divert(-1)')')
define(RTNS,*NULL*)
define(FALTER,P_$1)
define(FALTER1,`ifelse(substr($1,0,1),`(',`FALTER2'$1,
		      substr($1,0,1),Y,P_$1,(P_$1<<16))')
define(FALTER2,`ifelse(substr($1,0,1),Y,(P_$1|(P_$2<<16)),(P_$2|(P_$1<<16)))')


				/* string parameters	*/
define(PARMSTR,`divert(1)
cdefine P_$1 NUMPARMS
divert(-1)
define(`NAME',substr($1,0,$2))
define(`SDFL',ifelse($4,,$3,$4))
define(`FLGS',ifelse($5,,NULL,$5))
define(`CALL',ifelse($6,,NULL,$6))
define(`USEF',ifelse($7,,-1,FALTER($7)))
define(`SIZE',ifelse($8,,16,$8))
ifelse(index(FLGS,PMFfprop),-1,,
	`define(`NUMFILE',incr(NUMFILE))
	 define(`FILESTSZ',FILESTSZ+SIZE)')
ifelse(index(FLGS,PMFctrl),-1,,
	`define(`NUMCTRL',incr(NUMCTRL))
	 define(`CTRLSTSZ',CTRLSTSZ+SIZE)')
DEFRTN(CALL)
define(`NUMPARMS',incr(NUMPARMS))
divert(2)
   { PMTYSTR,"$1",$2,FLGS,CALL,(PARMVAL) PS$1,(PARMVAL)$3,(PARMVAL)SDFL,
	 USEF,SIZE,0 },
divert(3)
static TEXT PS$1[SIZE] = $3;
divert(-1)')

					/* other parameters	*/
define(PARMXXX,`divert(1)
cdefine P_$1 NUMPARMS
divert(-1)
define(`NAME',substr($1,0,$2))
define(`SDFL',ifelse($4,,$3,$4))
define(`FLGS',ifelse($5,,0,$5))
define(`CALL',ifelse($6,,NULL,$6))
define(`USEF',ifelse($7,,-1,FALTER($7)))
define(`LBD',ifelse($8,,0,$8))
define(`UBD',ifelse($9,,32767,$9))
define(`NUMPARMS',incr(NUMPARMS))
ifelse(index(FLGS,PMFfprop),-1,,
	`define(`NUMFILE',incr(NUMFILE))')
ifelse(index(FLGS,PMFctrl),-1,,
	`define(`NUMCTRL',incr(NUMCTRL))')
DEFRTN(CALL)
divert(2)
   { PMTYX,"$1",$2,FLGS,CALL,(PARMVAL)$3,(PARMVAL)$3,(PARMVAL)SDFL,
	USEF,LBD,UBD },
divert(-1)')


define(PARMNUM,`define(`PMTYX',`PMTYNUM')
PARMXXX($1,$2,$3,$4,$5,$6,$7,$8,$9)')		 /* numeric parms */
define(PARMFLG,`define(`PMTYX',`PMTYFLAG')
PARMXXX($1,$2,$3,$4,$5,$6,$7,0,0)')		 /* flag    parms */
define(PARMIGN,`define(`PMTYX',`PMTYIGN')
PARMXXX($1,$2,$3,$4,$5,$6,$7,$8,$9)')		  /* no arg  parms */
define(PARMBUF,`define(`PMTYX',`PMTYBUF')
PARMXXX($1,$2,$3,$4,$5,$6,$7,$8,$9)')		  /* buffer  parms */
define(PARMBFCT,`define(`PMTYX',`PMTYBFCT')
PARMXXX($1,$2,$3,$4,$5,$6,$7,$8,$9)')		  /* buf/cnt parms */



/* end of parmlist.m */

include(parameters)

divert


