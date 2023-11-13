	/* index(str,c) -- return pointer to first instance of char c in str */

	.text
	.globl _index
_index:
	.word	 0x6		/* save r1 and r2 for scratch		     */
	movl	4(ap),r2	/* r2 points to start of str		     */
	locc	$0,$0xFFFF,(r2)	/* r1 points to end of str		     */
	incl	r1		/* null char may be found by index if desired*/
	subl2	r2,r1		/* r1 gets length of str to check	     */
	locc	8(ap),r1,(r2)	/* find the first occurrence of c in str     */
	beql	L0		/* if not found, return null in r0	     */
	movl	r1,r0		/* otherwise, r1 points to c, return in r0   */
L0:	ret
