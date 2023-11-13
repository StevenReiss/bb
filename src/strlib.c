/*	Copyright 1984 Brown University 				*/
#include <string.h>



int allblank(pt)	    /* check if string is all blanks */
   char *pt;
{
   char ch;

   while ((ch = *pt++) == ' ');

   return (ch == 0) ? 1 : 0;
}





void tabfilter(line,len)     /* replaces tabs with spaces in */
   char *line;		/*   line (recopys & expands as */
   int len;		/*   necessary up to len	*/
{
   char buf[10240],*pta,*ptb,ch;
   int ct;

   if (strchr(line,'\t') == 0) return;

   pta = line;
   ptb = buf;
   ct = 0;

   while ((ch = *pta++) != 0 && ct < 1023)
      if (ch != '\t') { *ptb++ = ch; ++ct; }
       else
	  do { *ptb++ = ' '; ++ct; }
	     while ((ct & 07) != 0);

   *ptb++ = 0;
   if (ct > len) buf[len-1] = 0;

   strcpy(line,buf);
}





void null_strncpy(to,fr,ln)	     /* strncpy, but insure null termination */
   char *to;
   char *fr;
   int ln;
{
   strncpy(to,fr,ln);
   to[ln] = 0;
}



char * our_strcpy(char * to,char *frm)		       /* strcpy, don't complain on overlap */
{
   char * rslt = to;
   char ch;

   while ((ch = *frm++) != 0) {
      *to++ = ch;
    }
   *to++ = 0;
   return rslt;
}




/* end of strlib.c */
