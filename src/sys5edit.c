/************************************************************************/
/*									*/
/*	sys5edit.c -- 4.2 rtns needed to work in sys5 environment	*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <string.h>
#include "global.h"

extern	int	errno;

rename(from,to)
   char * from;
   char * to;
{
   if (link(from,to) != 0) {
      if (errno == EEXIST) {
	 unlink(to);
	 if (link(from,to) != 0) return -1;
       }
      else return -1;
    };

   return unlink(from);
}






int
flock(fid,mode)
   int fid;
   int mode;
{
   struct flock flk;
   int i,type;

   if (mode & LOCK_SH) flk.l_type = F_RDLCK;
   else if (mode & LOCK_EX) flk.l_type = F_WRLCK;
   else if (mode & LOCK_UN) flk.l_type = F_UNLCK;
   else return -1;

   if (mode & LOCK_NB) type = F_SETLK;
   else type = F_SETLKW;

   flk.l_whence = 0;
   flk.l_start = 0;
   flk.l_len = 0;
   flk.l_pid = 0;

   i = fcntl(fid,type,&flk);

   return i;
}




int
vfork()
{
   return fork();
}
