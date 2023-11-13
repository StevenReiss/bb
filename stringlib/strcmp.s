	/* strcmp(s1, s2) -- compare strings s1 and s2			     */
	/*			returns 0 if eq, n if s1 > s2, -n otherwise  */

	.globl _strcmp
_strcmp:
	.word	 0xE		/* save r1, r2, and r3 for cmpc and scratch  */
	movl	4(ap),r2	/* r2 points to start of s1		     */
	movl	8(ap),r3
	locc	$0,$0xFFFF,(r2)	/* r1 points to end of s1		     */
	subl2	r2,r1		/* last block -- move all plus the null	     */
	incl	r1		/* r1 gets length of s1	including the null   */
	cmpc3	r1,(r2),(r3)	/* compare s1 and s2, result in r0	     */
	beql	L1
	blss	L0
	movl	$1,r0
	ret
L0:	mnegl	$1,r0
L1:	ret
