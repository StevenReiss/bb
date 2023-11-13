/*	lmove (fr,to,len) -- move len words from fr to to		*/

	.globl _lmove
_lmove:
	.word	0x3E		/* save r1,r2,r3,r4 and r5 for scratch	*/
	movl	12(ap),r0	/* get length				*/
	ashl	$2,r0,r0	/* convert to words			*/
	movc3	r0,*4(ap),*8(ap)  /* do the move			*/
	ret
