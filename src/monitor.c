/****************************************************************************************/
/*											*/
/*		monitor.c								*/
/*											*/
/*	This file contains the code to attempt to connect to TEA to support		*/
/*	semantic-based editing.  Things here have no effect if no connection		*/
/*	is made.									*/
/*											*/
/****************************************************************************************/


#include "global.h"
#include "parms.h"
#include "editor.h"

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>



/****************************************************************************************/
/*											*/
/*	Forward Definitions								*/
/*											*/
/****************************************************************************************/


static	void		open_socket();
static	void		get_addr_file(char *);
static	void		read_input();



/****************************************************************************************/
/*											*/
/*	Local Storage									*/
/*											*/
/****************************************************************************************/


static	int	sock_fd = -1;
static	FILE *	sock_out;
static	FILE *	sock_in;
static	BOOL	have_file;



/****************************************************************************************/
/*											*/
/*	Initialization methods								*/
/*											*/
/****************************************************************************************/


void
MNinit()
{
   sock_fd = -1;
   sock_out = NULL;
   have_file = FALSE;

   open_socket();

   if (sock_fd >= 0) sock_out = fdopen(sock_fd,"w");
   if (sock_fd >= 0) sock_in = fdopen(sock_fd,"r");
}




/****************************************************************************************/
/*											*/
/*	Message sending methods 							*/
/*											*/
/****************************************************************************************/


void
MNopen()
{
   STRING d,f;

   if (sock_fd < 0) return;

   d = PSVAL(directory);
   if (d[0] != '/') {
      char buf[1024];
      getcwd(buf,1024);
      if (strcmp(d,".") != 0) {
	 strcat(buf,"/");
	 strcat(buf,d);
       }
      d = buf;
    }
   f = PSVAL(filename);

   fprintf(sock_out,"OPEN %s/%s\n",d,f);
   fprintf(sock_out,"BEGIN\n");
   fflush(sock_out);
   have_file = TRUE;
}



void
MNreplace(LINE ln,STRING txt)
{
   if (sock_fd < 0 || !have_file) return;
   fprintf(sock_out,"REPLACE %ld %s\n",ln,txt);
   fflush(sock_out);
}



void
MNtypein(LINE ln,STRING txt)
{
   MNreplace(ln,txt);
}



void
MNinsert(LINE ln)
{
   if (sock_fd < 0 || !have_file) return;
   fprintf(sock_out,"INSERT %ld\n",ln);
   fflush(sock_out);
}



void
MNdelete(LINE ln)
{
   if (sock_fd < 0 || !have_file) return;
   fprintf(sock_out,"DELETE %ld\n",ln);
   fflush(sock_out);
}



void
MNsync()
{
   if (sock_fd < 0 || !have_file) return;
   fprintf(sock_out,"END\n");
   fprintf(sock_out,"SYNC\n");
   fflush(sock_out);
   read_input();
   fprintf(sock_out,"BEGIN\n");
   fflush(sock_out);
}



void
MNposition()
{
   if (sock_fd < 0 || !have_file) return;

   if (PVAL(marking)) {
      INT x0,y0,x1,y1;

      if (PVAL(markline) < PVAL(curline) ||
	     (PVAL(markline) == PVAL(curline) && PVAL(markchar) < PVAL(curchar))) {
	 x0 = PVAL(markchar);
	 y0 = PVAL(markline);
	 x1 = PVAL(curchar);
	 y1 = PVAL(curline);
       }
      else {
	 x0 = PVAL(curchar);
	 y0 = PVAL(curline);
	 x1 = PVAL(markchar);
	 y1 = PVAL(markline);
       }
      fprintf(sock_out,"POS %d %d %d %d\n",y0,x0,y1,x1);
    }
   else {
      fprintf(sock_out,"POS %ld %ld\n",PVAL(curline),PVAL(curchar));
    }

   fflush(sock_out);
}




/****************************************************************************************/
/*											*/
/*	Socket methods									*/
/*											*/
/****************************************************************************************/


static void
open_socket()
{
   char buf[1024],fbuf[1024],hbuf[1024];
   int fd;
   struct sockaddr_in name;
   int host,port;
   int i,s;
   char * p;
   char * q;
   struct hostent * he;

   sock_fd = -1;

   get_addr_file(buf);
   fd = open(buf,O_RDWR,0);
   if (fd < 0)	return;

   i = read(fd,fbuf,128);
   close(fd);
   if (i <= 0) return;

   q = hbuf;
   for (p = fbuf; *p != ' ' && *p != 0; ++p) *q++ = *p;
   *q = 0;
   if (*p == 0) return;
   ++p;
   port = 0;
   while (isdigit(*p)) port = port*10 + *p++ - '0';
   if (port == 0) return;

   memset(&name,0,sizeof(name));
   name.sin_family = AF_INET;
   name.sin_port = htons(port);
   gethostname(fbuf,64);
   fbuf[0] = 0; 			/* don't allow loopback */
   if (strcmp(fbuf,hbuf) == 0) name.sin_addr.s_addr = INADDR_LOOPBACK;
   else {
      he = gethostbyname(hbuf);
      if (he == NULL) return;
      else name.sin_addr.s_addr = *((int *) he->h_addr);
    }

   s = socket(AF_INET,SOCK_STREAM,0);
   if (s < 0) return;

   while (connect(s,(struct sockaddr *) &name,sizeof(name)) < 0) {
      if (errno != EINPROGRESS) {
	 close(s);
	 return;
       }
    }

   sock_fd = s;

   signal(SIGPIPE,SIG_IGN);
}





/************************************************************************/
/*									*/
/*	get_addr_file -- get name of lock file				*/
/*									*/
/************************************************************************/


static void
get_addr_file(char * buf)
{
   char * s;

   s = getenv("TEA_SEDI_FILE");
   if (s != NULL) {
      strcpy(buf,s);
      return;
    }

   sprintf(buf,"/pro/tea/tmp/SediServer.%s.data",getenv("USER"));
}



/****************************************************************************************/
/*											*/
/*	Input processing routines							*/
/*											*/
/****************************************************************************************/


static void
read_input()
{
   char buf[10240];
   char * rslt;

   for ( ; ; ) {
      rslt = fgets(buf,10240,sock_in);
      if (rslt == NULL) {
	 if (ferror(sock_in)) {
	    close(sock_fd);
	    sock_fd = -1;
	  }
	 break;
       }
      if (strncmp(buf,"ACK",3) == 0) break;
      else if (strncmp(buf,"FMT",3) == 0) {
       }
      else if (strncmp(buf,"MSG",3) == 0) {
       }
      else if (strncmp(buf,"FIL",3) == 0) {
	 char * p = buf;
	 while (!isspace(*p) && *p != 0) ++p;
	 if (*p == 0) continue;
	 while (isspace(*p)) ++p;
	 if (*p == 0) continue;
	 PSET(newfile,FALSE);
	 PSET(alternate,FALSE);
	 Parm(p,TRUE);
       }
      else if (strncmp(buf,"EDT",3) == 0) {
       }
    }
}




/* end of monitor.c */
