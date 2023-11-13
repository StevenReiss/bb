/************************************************************************/
/*									*/
/*		APeditor.c						*/
/*									*/
/*	Miscellaneous routines for apollo version of bb 		*/
/*									*/
/************************************************************************/


#include "global.h"






/************************************************************************/
/*									*/
/*	strfill -- move bytes from one string to another		*/
/*									*/
/************************************************************************/


void strfill(s1,s2,n1)
   register STRING s1;
   register INT n1;
   register STRING s2;
{
   register INT n2;

   n2 = strlen(s2);
   if (n2 > n1) n2 = n1;
   n1 -= n2;

   if (((long)s1) < ((long) s2)) while (n2-- > 0) *s1++ = *s2++;
   else {
      register INT i;
      for (i = n2-1; i >= 0; --i) s1[i] = s2[i];
      s1 = &s1[n2];
    };

   while (n1-- > 0) *s1++ = ' ';
}





/************************************************************************/
/*									*/
/*	match								*/
/*									*/
/************************************************************************/


STRING
match(str,pat)
   STRING str;
   STRING pat;
{
   register INT i;
   register STRING p,q;

   while (*str) {
      if (*str == pat[0]) {
	 p = str;
	 q = pat;
	 while (*q != 0 && *q == *p++) ++q;
	 if (*q == 0) return str;
       };
      ++str;
    };

   return NULL;
}






/************************************************************************/
/*									*/
/*	matchn								*/
/*									*/
/************************************************************************/


STRING
matchn(str,len,pat,n)
   STRING str;
   INT len;
   STRING pat;
   INT n;
{
   register INT i;

   for (i = 0; i <= len-n; ++i) {
      if (*str == pat[0]) {
	 if (strncmp(str,pat,n) == 0) return str;
       };
      ++str;
    };

   return NULL;
}






/* end of apeditor.c */
