	/* strcpy(s1, s2) -- copy string s2 into s1			     */
	/*			returns s1				     */

	.globl _strcpy
_strcpy:
	.word	 0x3E		/* save r1, r2, r3, r4, and r5 for scratch   */
	movl	8(ap),r2	/* r2 points to start of s2		     */
	movl	4(ap),r3	/* r3 points to start of s1		     */
	locc	$0,$0xFFFF,(r2)	/* r1 points to null or 16k-1 away	     */
	subl2	r2,r1		/* r1 gets length of s2 		     */
	incl	r1		/* include the null when copying s2	     */
	movc3	r1,(r2),(r3)	/* move s2 to s1			     */
	movl	4(ap),r0	/* return pointer to s1			     */
	ret
