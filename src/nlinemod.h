/********************************************************/
/*							*/
/*		 LINEMOD.H				*/
/*							*/
/*	This file contains the definitions needed by	*/
/*	external modules in order to use the linemod	*/
/*	module. 					*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/






/********************************************************/
/*							*/
/*	Parameter definitions				*/
/*							*/
/********************************************************/


				/* # lines per line blk */
#define LNPERLB 	(LBLKSIZE/sizeof(ADDR))
				/* log2(LNPERLB)	*/
#define LNPBLOG 	(LBSZLOG-LOGSIZEADDR)





/********************************************************/
/*							*/
/*	Routine definitins				*/
/*							*/
/********************************************************/


extern		LNinit();	/* initialize a file	*/
extern		LNclose();	/* close a file 	*/
extern	LINE	LNcheck();	/* check new position	*/
extern	INT	LNget();	/* get given line	*/
extern		LNput();	/* replace given line	*/
extern		LNputin();	/* insert given line	*/
extern		LNgetdel();	/* get and delete line	*/
extern		LNmark();	/* save marker		*/
extern	LINE	LNgetmark();	/* retrieve marker	*/
extern		LNunmark();	/* remove marker	*/





/********************************************************/
/*							*/
/*	Data structures -- history information		*/
/*							*/
/********************************************************/


typedef enum {			/* history actions for	*/
   HSL_PUT, HSL_INS, HSL_DEL	/* line data -- used in */
}				/* marking/recovery	*/
   HISTLINE;






/********************************************************/
/*							*/
/*	External storage				*/
/*							*/
/********************************************************/


extern	ADDR *	LNlbuff[];	/* line number buffer block pointers	*/






/* end of linemod.h */
