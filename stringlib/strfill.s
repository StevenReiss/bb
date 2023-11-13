	/* strfill(s1, s2, n) -- copy string s2 into s1 for max n bytes      */
	/*			returns s1				     */

	.globl _strfill
_strfill:
	.word	 0x3E		/* save r1, r2, r3, r4, and r5 for scratch   */
	movl	8(ap),r3	/* r3 points to start of s2		     */
	movl	12(ap),r2	/* r2 gets n				     */
	locc	$0,r2,(r3)	/* r1 points to end of s2		     */
	subl2	r3,r1		/* r1 gets number of (non-null) bytes to move*/
	movc5	r1,(r3),$32,r2,*4(ap)	 /* move s2 into s1 padding with sps */
	movl	4(ap),r0	/* return pointer to s1 		     */
	ret

