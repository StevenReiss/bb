/************************************************************************/
/*									*/
/*		global.h						*/
/*									*/
/*	general definitions						*/
/*									*/
/************************************************************************/
/*	Copyright 1984 Brown University 				*/


#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef ENCORE
#include <fcntl.h>
#else
#ifdef mips
#include <fcntl.h>
#else
#ifdef vax
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#endif
#endif
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


					/* constants			*/
#define TRUE 1
#define FALSE 0
#define MAYBE 2
#ifndef NULL
#define NULL 0
#endif
#define NULLSTR ((STRING)0)



					/* Types			*/

typedef int	BOOL;			/*    boolean			*/
typedef char	CHAR;			/*    characters		*/
typedef char *	STRING; 		/*    string			*/
typedef int	INT;			/*    integer			*/
typedef long int LONG;			/*    longest integer		*/
typedef short int SHORT;		/*    16 bit integer		*/
typedef int *	ANY;			/*    arbitrary type		*/
typedef float	FLOAT;			/*    floating point		*/
typedef double	DOUBLE; 		/*    double precision		*/
typedef unsigned char BYTE;		/*    byte value		*/
typedef int	(*FCT)();		/*    integer valued function	*/

typedef long	PINT;			/* integer to hold a pointer */



					/* systems stuff		*/
#ifndef LOCK_EX
#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8
#endif


#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#ifndef APPLE
#define stat64 stat
#define fstat64 fstat
#endif
#define lseek64 lseek
#endif

extern char * our_strcpy(char *,char *);
