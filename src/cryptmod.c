/********************************************************/
/*							*/
/*		CRYPTMOD.C				*/
/*   provide encrypted file services to the editor.  this uses the	*/
/*   standard unix package for encryption, although it is copied here	*/
/*   and not used as a systems program					*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/

#include "global.h"
#include "editor.h"
#include <sys/types.h>
#include <sys/wait.h>


/*
 *	A one-rotor machine designed along the lines of Enigma
 *	but considerably trivialized.
 */

#define ECHO 010
#define ROTORSZ 256
#define MASK 0377

static	char	t1[ROTORSZ];
static	char	t2[ROTORSZ];
static	char	t3[ROTORSZ];
extern	char	*getpass();

static	int	inn1,inn2;
static	int	otn1,otn2;

#define TRANSLATE(xx,n1,n2,zz)\
   zz=xx;\
   zz = t2[(t3[(t1[(zz+n1)&MASK]+n2)&MASK]-n2)&MASK]-n1;\
   if (++n1 == ROTORSZ) {	\
      n1 = 0;			\
      if (++n2 == ROTORSZ) n2 = 0; \
    }



static void setup(pw)
char *pw;
{
	int ic, i, k, temp, pf[2];
	unsigned random;
	char buf[13];
	long seed;

	strncpy(buf, pw, 8);
	while (*pw)
		*pw++ = '\0';
	buf[8] = buf[0];
	buf[9] = buf[1];
	pipe(pf);
	if (fork()==0) {
		close(0);
		close(1);
		dup(pf[0]);
		dup(pf[1]);
		execl("/usr/lib/makekey", "-", 0);
		execl("/lib/makekey", "-", 0);
		exit(1);
	}
	write(pf[1], buf, 10);
	wait((int *)NULL);
	if (read(pf[0], buf, 13) != 13) {
		fprintf(stderr, "crypt: cannot generate key\n");
		exit(1);
	}
	seed = 123;
	for (i=0; i <ROTORSZ;++i)
		t1[i] = t2[i] = t3[i] = 0;
	for (i=0; i<13; i++)
		seed = seed*buf[i] + i;
	for(i=0;i<ROTORSZ;i++)
		t1[i] = i;
	for(i=0;i<ROTORSZ;i++) {
		seed = 5*seed + buf[i%13];
		random = seed % 65521;
		k = ROTORSZ-1 - i;
		ic = (random&MASK)%(k+1);
		random >>= 8;
		temp = t1[k];
		t1[k] = t1[ic];
		t1[ic] = temp;
		if(t3[k]!=0) continue;
		ic = (random&MASK) % k;
		while(t3[ic]!=0) ic = (ic+1) % k;
		t3[k] = ic;
		t3[ic] = k;
	}
	for(i=0;i<ROTORSZ;i++)
		t2[t1[i]&MASK] = i;
}




int CRinchr(ch) 	    /* translate input string	    */
   TEXT ch;
{
   int nch;

   TRANSLATE(ch,inn1,inn2,nch);

   return nch;
}




int CRotchr(ch) 	    /* translate output character   */
   TEXT ch;
{
   int nch;

   TRANSLATE(ch,otn1,otn2,nch);

   return nch;
}





void CRinset(file)		     /* set up encryption for file	     */
   char *file;
{
   char msg[80],nmsg[80];

   sprintf(msg,"Enter key for file '%s': ",file);
   DSreply(msg,nmsg,80,-1);
   nmsg[strlen(nmsg)-1] = 0;
   setup(nmsg);

   inn1 = inn2 = otn1 =otn2 =0;
}





/* end of cryptmod.c */
