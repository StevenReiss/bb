/********************************************************/
/*							*/
/*		BUFMOD.H				*/
/*							*/
/*	This file contains the definitions for		*/
/*   maintaining the buffers.  Buffers are used to	*/
/*   hold macros (DO sequences) as well as deleted	*/
/*   and picked text.  25 real buffers and 3 special	*/
/*   buffers are provided.				*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/




/********************************************************/
/*							*/
/*	Parameters					*/
/*							*/
/********************************************************/


#define NMCURBF 3	/* number in core at once	*/

#define BFBLKSZ 	1024	/* size of buffer block 	*/

#define BFTEXTSZ	(BFBLKSZ-sizeof(ADDR))	/* text area of block	*/





/********************************************************/
/*							*/
/*	Data structures 				*/
/*							*/
/********************************************************/


typedef struct {		/* file buffer block	*/
   ADDR BDnext; 		/*    addr of next blk	*/
   TEXT BDtext[BFTEXTSZ];	/*    data of block	*/
} BUFBLOCK;



typedef struct _BUFINFO {	/* in core buffer info	*/
   INT	id;			/*    cur data id	*/
   ADDR  curtxt;		/*    cur text blk #	*/
   INT	txtidx; 		/*    index in text blk */
   union {
      TEXT *txtptr;		/*    ptr to text blk	*/
      long long fill;
    };
   ADDR frsttxt;		/*    address of data	*/
   INT pid;			/*    last set of buffer*/
}
   BBUFINFO,*BUFINFO;


typedef struct _BUFDATA {	/* active buffer data blocks */
   INT	bufid;			/*    buffer id 	*/
   BOOL writefg;		/*    data changed	*/
   INT	usecnt; 		/*    LRU counter	*/
   BUFBLOCK data;		/*    actual data	*/
}
   BBUFDATA,*BUFDATA;






/********************************************************/
/*							*/
/*	Buffer names					*/
/*							*/
/********************************************************/


#define BF_A	0
#define BF_B	1
#define BF_C	2
#define BF_D	3
#define BF_E	4
#define BF_F	5
#define BF_G	6
#define BF_H	7
#define BF_I	8
#define BF_J	9
#define BF_K	10
#define BF_L	11
#define BF_M	12
#define BF_N	13
#define BF_O	14
#define BF_P	15
#define BF_Q	16
#define BF_1	17
#define BF_2	18
#define BF_3	19
#define BF_4	20
#define BF_5	21
#define BF_DO	22
#define BF_CHAR 23
#define BF_LINE 24

#define BF_SINK 	25
#define BF_SPACE	26
#define BF_NEWLINE	27
#define BF_STAR 	28

#define NMBUFS		25
#define BFACTNUM	(NMBUFS+3)

#define BF_FREE 	BFACTNUM	/* freelist descriptor */
#define BF_USECT	(BFACTNUM+1)	/* use counter	       */

#define BFHDRNUM	(BFACTNUM+2)




/********************************************************/
/*							*/
/*	Entrys to buffer module 			*/
/*							*/
/********************************************************/


extern	void	BFset();	/* set buffer for read	*/
extern	INT	BFget();	/* get next char from buf */
extern	void	BFclear();	/* clear buf for write	*/
extern	void	BFputch();	/* save char in buffer	*/
extern	void	BFsync();
extern	void	BFsave();



/********************************************************/
/*							*/
/*	History methods 				*/
/*							*/
/********************************************************/

extern	void	HSbuf(INT,ADDR);
extern	void	HSclear();
extern	void	HSmark();
extern	void	HSparmstr();
extern	void	HSparm();
extern	void	HSline();
extern	void	HSlmark();
extern	int	HSundo();




/* end of bufmod.h */
