	/* strcat(s1, s2) -- concat string s2 onto end of s1		     */
	/*			returns s1				     */

	.globl _strcat
_strcat:
	.word	 0x3E		/* save r1, r2, r3, r4, and r5 for scratch   */
	movl	$0xFFFF,r5
	movl	8(ap),r2	/* r2 points to start of s2		     */
	locc	$0,r5,*4(ap)	/* find null at end of s1		     */
	movl	r1,r3		/* r3 points to end of s1		     */
	locc	$0,r5,(r2)	/* r1 points to null at end of s2	     */
	subl2	r2,r1		/* r1 gets length of s2			     */
	incl	r1		/* include the null in the move		     */
	movc3	r1,(r2),(r3)	/* concatenate s2 onto the end of s1	     */
	movl	4(ap),r0	/* return pointer to s1			     */
	ret
