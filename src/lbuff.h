/********************************************************/
/*							*/
/*		LBUFF.H 				*/
/*							*/
/*   This file contains the definitions for the internal*/
/*   line buffer.  This is the contents of the current	*/
/*   line.  It is modified by using the global routine	*/
/*   LBmove to move data into and out of the buffer.	*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/





/********************************************************/
/*							*/
/*	Entrys						*/
/*							*/
/********************************************************/


extern	int	LBmove();	/* move data thru buffer*/





/********************************************************/
/*							*/
/*	Storage 					*/
/*							*/
/********************************************************/


extern	TEXT	LBlinebuf[];	/* actual line buffer	*/





/********************************************************/
/*							*/
/*	Status for formatting routines			*/
/*							*/
/********************************************************/


typedef enum {			/* status flags 	*/
   LSTS_DONE,			/*    all finished	*/
   LSTS_PASS,			/*    no data this time */
   LSTS_CONTINUE,		/*    more data yet	*/
   LSTS_LAST,			/*    last of data	*/
   LSTS_NEXT			/*    give next line	*/
}
   LBSTATUS;





/********************************************************/
/*							*/
/*	Flags						*/
/*							*/
/********************************************************/


typedef int	LBFLAGS;

#define LBF_SMAX      0x1	/* max count for source */
#define LBF_SPARM     0x2	/* source parameter	*/
#define LBF_SINCP     0x4	/* increment source parm*/
#define LBF_SFRST     0x8	/* first flag for src	*/
#define LBF_DMAX      0x10	/* max count for target */
#define LBF_DPARM     0x20	/* parm for target	*/
#define LBF_DINCP     0x40	/* increment target parm*/
#define LBF_DFRST     0x80	/* first flag for dst	*/
#define LBF_SIGN      0x100	/* ignore source status */
#define LBF_DIGN      0x200	/* ignore target status */

#define LBF_INTAB     0x1000	/* tabs on input mapping*/
#define LBF_OUTAB     0x2000	/* tabs on output map	*/
#define LBF_STREAM    0x4000	/* stream mapping	*/
#define LBF_CRLF      0x8000	/* handle CR-LF pairs	*/

#define LBF_FMT0      0x10000	/* format mapping	*/
#define LBF_FMT1      0x20000	/* format mapping	*/
#define LBF_FMT2      0x40000	/* format mapping	*/
#define LBF_FMT3      0x80000	/* format mapping	*/
#define LBF_FMT4      0x100000	/* format mapping	*/





/* end of lbuff.h */
