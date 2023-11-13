/* lmove(from, to, len): move len words from from to to */

void lmove(from, to, len)
	register int *from, *to, len;
{
   if (to < from) {
      while (len--)
	 *to++ = *from++;
    }
   else {
      while (len--)
	 to[len] = from[len];
    };
}


void llmove(unsigned long long * from,unsigned long long * to, long len)
			
{
   if (to < from) {
      while (len--)
	 *to++ = *from++;
    }
   else {
      while (len--)
	 to[len] = from[len];
    };
}


