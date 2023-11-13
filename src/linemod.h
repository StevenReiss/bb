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


extern	void	LNinit();	/* initialize a file	*/
extern	int	LNclose();	/* close a file 	*/
extern	LINE	LNcheck();	/* check new position	*/
extern	void	LNforce();
extern	INT	LNget();	/* get given line	*/
extern	int	LNput();	/* replace given line	*/
extern	int	LNputin();	/* insert given line	*/
extern	INT	LNgetdel();	/* get and delete line	*/
extern	void	LNundo();
extern	void	LNmark();	/* save marker		*/
extern	LINE	LNgetmark();	/* retrieve marker	*/
extern	void	LNunmark();	/* remove marker	*/





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


extern	ADDR ** LNlbuff;	/* line number buffer block pointers	*/



/********************************************************/
/*							*/
/*	History methods 				*/
/*							*/
/********************************************************/

extern	void	HSline(HISTLINE,LINE,ADDR);



/* end of linemod.h */
