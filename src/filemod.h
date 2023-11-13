/********************************************************/
/*							*/
/*		FILEMOD.H				*/
/*							*/
/*	This file contains the definitions for file	*/
/*   processing.  These include the status flags that	*/
/*   are returned from the file module. 		*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/




/********************************************************/
/*							*/
/*	Constants					*/
/*							*/
/********************************************************/

#define MAX_BACKUP_SIZE 512*1024*1024  /* max size for saving backup */




/********************************************************/
/*							*/
/*	Status Flags					*/
/*							*/
/********************************************************/

typedef enum {
		FL_OK,		/* open succeeded	*/
		FL_FAIL,	/* open failed		*/
		FL_NEW, 	/* open failed--no file */
		FL_BAD, 	/* bad file type	*/
		FL_READ,	/* read only file	*/
		FL_LOCK 	/* file is locked	*/
} FLSTATUS;






/********************************************************/
/*							*/
/*	Routine definitions				*/
/*							*/
/********************************************************/


extern	FLSTATUS FLinit();	/* open file		*/
extern	FLSTATUS FLoutput();	/* open output file	*/
extern	void	 FLclose();	/* close all files	*/
extern	INT	 FLget();	/* get line to temp buf */
extern	INT	 FLput();	/* store line in buffer */






/* end of files.h */
