/********************************************************/
/*							*/
/*	librtns.c -- our versions of library routines	*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include	<stdio.h>
#include	<signal.h>
#include "global.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


int local_system(s,n)
char *s;
int n;
{
	int status, pid, w;
	register char * sh;
	void (*istat)(), (*qstat)();
	char * getenv();

	sh = getenv("SHELL");
	if (sh == NULL) sh = "/bin/csh";

	if ((pid = vfork()) == 0) {
		if (n != 0) nice(n);
		execl(sh, "sh", "-c", s, 0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}




#define tst(a,b)	(*mode == 'r'? (b) : (a))
#define RDR	0
#define WTR	1
static	int	popen_pid[20];

FILE *
local_popen(cmd,mode)
char	*cmd;
char	*mode;
{
	int p[2];
	register int myside, hisside, pid;

	if(pipe(p) < 0)
		return NULL;
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		close(myside);
		dup2(hisside, tst(0, 1));
		close(hisside);
		execl("/bin/csh", "csh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return NULL;
	popen_pid[myside] = pid;
	close(hisside);
	return(fdopen(myside, mode));
}

int
pclose(ptr)
FILE *ptr;
{
	register int f, r;
	void (*hstat)(), (*istat)(), (*qstat)();
	int status;

	f = fileno(ptr);
	fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if(r == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	signal(SIGHUP, hstat);
	return(status);
}





int mesg(arg)
char *arg;
{
#ifndef apollo
	int r=0;
	char *tty;
	char *ttyname();
	struct stat sbuf;

	tty = ttyname(2);
	if (tty == 0)
		return(-1);
	if(stat(tty, &sbuf) < 0) return(-1);
	if(arg == 0 || *arg == 0)
	      { if(!(sbuf.st_mode & 02)) r = 1; }
	else  switch(arg[0]) {
		case 'y':
			chmod(tty,sbuf.st_mode|022); break;

		case 'n':
			chmod(tty,sbuf.st_mode&~022); r=1; break;

		default:
			return -1;
		}
	return r;
#else
	return(-1);
#endif
}


