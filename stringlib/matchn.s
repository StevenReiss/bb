	/* matchn(str,len,pat,n) -- return pointer to beginning of pat in str*/
	/*			the first n characters of pat are used unless*/
	/*			pat is shorter, in which case the first non- */
	/*			null characters of pat are matched in src    */

	.globl _matchn
_matchn:
	.word	 0x1E		/* save r1, r2, r3, and r4 for scratch	     */
	movl	16(ap),r5	/* r5 gets the length of pat searched for    */
	matchc	r5,*12(ap),8(ap),*4(ap) /* r3 points after pat in str	     */
	beql	L0		/* r0 = 0 if found			     */
	clrl	r0		/* not found, return null		     */
	ret
L0:	subl3	r5,r3,r0	/* otherwise point to first instance of pat  */
	ret
