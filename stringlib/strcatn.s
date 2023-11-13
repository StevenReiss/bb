	/* strcatn(s1, s2, n) -- concat string s2 onto end of s1, max len n  */
	/*			returns s1				     */

	.globl _strcatn
_strcatn:
	.word	 0x3E		/* save r1, r2, r3, r4, and r5 for scratch   */
	movl	12(ap),r4
	movl	8(ap),r2	/* r2 points to start of s2		     */
	locc	$0,$0xFFFF,*4(ap) /* find null at end of s1		     */
	movl	r1,r3		/* r3 points to end of s1		     */
	locc	$0,r4,(r2)	/* r1 points to null or to end of s2	     */
	subl2	r2,r1		/* r1 gets length of s2			     */
	movc3	r1,(r2),(r3)	/* move s2 into s1, update r1 and r3	     */
	movb	$0,(r3)		/* terminate with null			     */
	movl	4(ap),r0	/* return pointer to s1			     */
	ret
