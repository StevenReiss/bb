	/* strlen(str) -- return length of str 				     */

	.text
	.globl _strlen
_strlen:
	.word	 0x6		/* save r1 and r2 for scratch		     */
	movl	4(ap),r2	/* r2 points to start of str		     */
	locc	$0,$0xFFFF,(r2)	/* r1 points to end of str		     */
	subl3	r2,r1,r0	/* return length of str			     */
	ret
