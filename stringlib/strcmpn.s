	/* strcmpn(s1, s2, n) -- compare up to n characters of s1 and s2     */
	/*			returns 0 if eq, 1 if s1 > s2, -1 otherwise  */

	.globl _strcmpn
	.globl _strncmp
_strcmpn:
_strncmp:
	.word	 0x1E		/* save r1, r2, r3, and r4 for scratch	     */
	movl	4(ap),r3	/* r3 points to start of s1		     */
	movl	12(ap),r2	/* r2 gets n				     */
	locc	$0,r2,(r3)	/* find null in s1, if any		     */
	movl	r1,r4		/* r4 points to null or 1 byte past end of s1*/
	cmpc3	r2,(r3),*8(ap)
	beql	L1
	blss	L0
	cmpl	r1,r4
	bgtr	L1
	movl	$1,r0
	ret
L0:	cmpl	r1,r4
	bgtr	L1
	mnegl	$1,r0
	ret
L1:	clrl	r0
	ret
