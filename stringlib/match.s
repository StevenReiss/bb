	/* match(str,pat) -- return pointer to first string pat in str	     */

	.text
	.globl _match
_match:
	.word	 0x3E		/* save r1, r2, r3, r4, and r5 for scratch   */
	movl	$0xFFFF,r5
	movl	8(ap),r2	/* r2 points to start of pat		     */
	movl	4(ap),r4	/* r4 points to start of str		     */
	locc	$0,r5,(r4)	/* r1 points to end of src		     */
	subl3	r4,r1,r3	/* r4 gets length of src 		     */
	locc	$0,r5,(r2)	/* r1 points to end of pat		     */
	subl3	r2,r1,r5	/* r5 gets length of pat		     */
	matchc	r5,(r2),r3,(r4)	/* r3 points to location of pat in str	     */
	beql	L0		/* r0 = 0 if found			     */
	clrl	r0		/* not found, return null		     */
	ret
L0:	subl3	r5,r3,r0	/* otherwise point to first instance of pat  */
	ret
