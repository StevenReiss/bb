	/* rindex(str,c) -- return pointer to last instance of char c in str */

	.text
	.globl _rindex
_rindex:
	.word	 0x6		/* save r1 and r2 for scratch		     */
	movl	4(ap),r2	/* r2 points to start of str		     */
	locc	$0,$0xFFFF,(r2)	/* r1 points to end of str		     */
	subl3	r2,r1,r0	/* r0 gets length of str to check	     */
	movl	r2,r1		/* r1 will move through str, stopping at c's */
	clrl	r2		/* start out with null pointer, none found   */
L0:	locc	8(ap),r0,(r1)	/* find the next occurrence of c in str	     */
	beql	L1		/* if not found, return last place found     */
	movl	r1,r2		/* otherwise save location of c, try again   */
	incl	r1		/* avoid getting the same character again    */
	decl	r0
	brb	L0
L1:	movl	r2,r0		/* otherwise, r1 points to c, return in r0   */
	ret
