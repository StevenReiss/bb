divert(-1)
/********************************************************/
/*							*/
/*		CMDLIST.M				*/
/*							*/
/*	This file contains the prefix macros needed to	*/
/*   interpret the user command specifications.  From	*/
/*   these it sets up the tables needed by cmds.hm	*/
/*							*/
/********************************************************/



define(`EXTRTNS')
define(`CMDCNT',0)
define(N4,`N4V(substr($1,0,1),substr($1,1,1),substr($1,2,1),substr($1,3,1))')


changequote([,])
define(N4V,[ifelse($2,,'$1',$3,,('$1'<<8|'$2'),
    $4,,(('$1'<<16)|('$2'<<8)|'$3'),
	(('$1'<<24)|('$2'<<16)|('$3'<<8)|'$4'))])

define([CMD],[
divert(-1)
define(`FLGS',ifelse($4,,NULL,$4))
define(`NAM4',ifelse(eval(len($1) <= 4),1,$1,substr($1,0,4)))
divert(1)
cdefine CM_$1 (CM_FIRST+CMDCNT) 	divert(2)
N4(NAM4),				divert(3)
CM_$1,					divert(-1)
ifelse(index(EXTRTNS,CMD$3|),-1,`
   define(`EXTRTNS',EXTRTNS|CMD$3|)
   divert(4)
extern	INT	CMD$3();		divert(-1)')
divert(5)
{ P_$2 , CMD$3 , $4 },			divert(-1)
define(`CMDCNT',incr(CMDCNT))
])
changequote




include(commands)


/* end of cmdlist.m */
divert
