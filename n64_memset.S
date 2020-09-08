#if 1

/*
 * Copyright (c) 2013
 *      MIPS Technologies, Inc., California.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the MIPS Technologies, Inc., nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE MIPS TECHNOLOGIES, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE MIPS TECHNOLOGIES, INC. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#define zero $0
#define at $1
#define v0 $2
#define v1 $3
#define a0 $4
#define a1 $5
#define a2 $6
#define a3 $7
#define t0 $8
#define t1 $9
#define t2 $10
#define t3 $11
#define t4 $12
#define t5 $13
#define t6 $14
#define t7 $15
#define s0 $16
#define s1 $17
#define s2 $18
#define s3 $19
#define s4 $20
#define s5 $21
#define s6 $22
#define s7 $23
#define t8 $24
#define t9 $25
#define k0 $26
#define k1 $27
#define gp $28
#define sp $29
#define fp $30
#define ra $31

#define __mips 3 
//#define USE_DOUBLE
//#define DOUBLE_ALIGN

/* Some asm.h files do not have the L macro definition.  */
#ifndef L
//#if _MIPS_SIM == _ABIO32
# define L(label) $L ## label
//#else
//# define L(label) .L ## label
//#endif
#endif
/* Some asm.h files do not have the PTR_ADDIU macro definition.  */
#ifndef PTR_ADDIU
#ifdef USE_DOUBLE
#define PTR_ADDIU	daddiu
#else
#define PTR_ADDIU	addiu
#endif
#endif


#define PTR_SUBU dsubu
#define PTR_ADDU daddu

/* Some asm.h files do not have the PTR_SRA macro definition.  */
#ifndef PTR_SRA
#ifdef USE_DOUBLE
#define PTR_SRA		dsra
#else
#define PTR_SRA		sra
#endif
#endif

/* We load/store 64 bits at a time when USE_DOUBLE is true.
   The C_ prefix stands for CHUNK and is used to avoid macro name
   conflicts with system header files.  */
#ifdef USE_DOUBLE
# define C_ST	sd
#  define C_STHI	sdl	/* high part is left in big-endian	*/
#else
# define C_ST	sw
#  define C_STHI	swl	/* high part is left in big-endian	*/
#endif
/* Bookkeeping values for 32 vs. 64 bit mode.  */
#ifdef USE_DOUBLE
# define NSIZE 8
# define NSIZEMASK 0x3f
# define NSIZEDMASK 0x7f
#else
# define NSIZE 4
# define NSIZEMASK 0x1f
# define NSIZEDMASK 0x3f
#endif
#define UNIT(unit) ((unit)*NSIZE)
#define UNITM1(unit) (((unit)*NSIZE)-1)

	.align 8
	.global __n64_memset_ZERO_ASM
	.global	__n64_memset_ASM
	.set	nomips16
	.set	nomicromips
    .set	noreorder
    .set	nomacro
	.ent	__n64_memset_ASM
	.type	__n64_memset_ASM,	@function
	__n64_memset_ZERO_ASM:
	__n64_memset_ASM:
	
	/* If the size is less than 2*NSIZE (8 or 16), go to L(lastb).  Regardless of
   size, copy dst pointer to v0 for the return value.  */
	slti	t2,a2,(2 * NSIZE)
	bne	t2,zero,L(lastb)
	move	v0,a0
/* If memset value is not zero, we copy it to all the bytes in a 32 or 64
   bit word.  */
	beq	a1,zero,L(set0)		/* If memset value is zero no smear  */
	PTR_SUBU a3,zero,a0
	nop
	/* smear byte into 32 or 64 bit word */
# ifdef USE_DOUBLE
        and     a1,0xff
	dsll	t2,a1,8
	or	a1,t2
	dsll	t2,a1,16
	or	a1,t2
	dsll	t2,a1,32
	or	a1,t2
# else
        and     a1,0xff
	sll	t2,a1,8
	or	a1,t2
	sll	t2,a1,16
	or	a1,t2
# endif
/* If the destination address is not aligned do a partial store to get it
   aligned.  If it is already aligned just jump to L(aligned).  */
L(set0):
	andi	t2,a3,(NSIZE-1)		/* word-unaligned address?          */
	beq	t2,zero,L(aligned)	/* t2 is the unalignment count      */
	PTR_SUBU a2,a2,t2
	C_STHI	a1,0(a0)
	PTR_ADDU a0,a0,t2
L(aligned):
/* If USE_DOUBLE is not set we may still want to align the data on a 16
   byte boundry instead of an 8 byte boundry to maximize the opportunity
   of proAptiv chips to do memory bonding (combining two sequential 4
   byte stores into one 8 byte store).  We know there are at least 4 bytes
   left to store or we would have jumped to L(lastb) earlier in the code.  */
#ifdef DOUBLE_ALIGN
	andi	t2,a3,4
	beq	t2,zero,L(double_aligned)
	PTR_SUBU a2,a2,t2
	sw	a1,0(a0)
	PTR_ADDU a0,a0,t2
L(double_aligned):
#endif
/* Now the destination is aligned to (word or double word) aligned address
   Set a2 to count how many bytes we have to copy after all the 64/128 byte
   chunks are copied and a3 to the dest pointer after all the 64/128 byte
   chunks have been copied.  We will loop, incrementing a0 until it equals
   a3.  */
	andi	t8,a2,NSIZEDMASK /* any whole 64-byte/128-byte chunks? */
	beq	a2,t8,L(chkw)	 /* if a2==t8, no 64-byte/128-byte chunks */
	PTR_SUBU a3,a2,t8	 /* subtract from a2 the reminder */
	PTR_ADDU a3,a0,a3	 /* Now a3 is the final dst after loop */
L(loop16w):
L(skip_pref):
	C_ST	a1,UNIT(0)(a0)
	C_ST	a1,UNIT(1)(a0)
	C_ST	a1,UNIT(2)(a0)
	C_ST	a1,UNIT(3)(a0)
	C_ST	a1,UNIT(4)(a0)
	C_ST	a1,UNIT(5)(a0)
	C_ST	a1,UNIT(6)(a0)
	C_ST	a1,UNIT(7)(a0)
	C_ST	a1,UNIT(8)(a0)
	C_ST	a1,UNIT(9)(a0)
	C_ST	a1,UNIT(10)(a0)
	C_ST	a1,UNIT(11)(a0)
	C_ST	a1,UNIT(12)(a0)
	C_ST	a1,UNIT(13)(a0)
	C_ST	a1,UNIT(14)(a0)
	C_ST	a1,UNIT(15)(a0)
	PTR_ADDIU a0,a0,UNIT(16)	/* adding 64/128 to dest */
	bne	a0,a3,L(loop16w)
	nop
	move	a2,t8
/* Here we have dest word-aligned but less than 64-bytes or 128 bytes to go.
   Check for a 32(64) byte chunk and copy if if there is one.  Otherwise
   jump down to L(chk1w) to handle the tail end of the copy.  */
L(chkw):
	andi	t8,a2,NSIZEMASK	/* is there a 32-byte/64-byte chunk.  */
				/* the t8 is the reminder count past 32-bytes */
	beq	a2,t8,L(chk1w)/* when a2==t8, no 32-byte chunk */
	nop
	C_ST	a1,UNIT(0)(a0)
	C_ST	a1,UNIT(1)(a0)
	C_ST	a1,UNIT(2)(a0)
	C_ST	a1,UNIT(3)(a0)
	C_ST	a1,UNIT(4)(a0)
	C_ST	a1,UNIT(5)(a0)
	C_ST	a1,UNIT(6)(a0)
	C_ST	a1,UNIT(7)(a0)
	PTR_ADDIU a0,a0,UNIT(8)
/* Here we have less than 32(64) bytes to set.  Set up for a loop to
   copy one word (or double word) at a time.  Set a2 to count how many
   bytes we have to copy after all the word (or double word) chunks are
   copied and a3 to the dest pointer after all the (d)word chunks have
   been copied.  We will loop, incrementing a0 until a0 equals a3.  */
L(chk1w):
	andi	a2,t8,(NSIZE-1)	/* a2 is the reminder past one (d)word chunks */
	beq	a2,t8,L(lastb)
	PTR_SUBU a3,t8,a2	/* a3 is count of bytes in one (d)word chunks */
	PTR_ADDU a3,a0,a3	/* a3 is the dst address after loop */
/* copying in words (4-byte or 8 byte chunks) */
L(wordCopy_loop):
	PTR_ADDIU a0,a0,UNIT(1)
	bne	a0,a3,L(wordCopy_loop)
	C_ST	a1,UNIT(-1)(a0)
/* Copy the last 8 (or 16) bytes */
L(lastb):
	blez	a2,L(leave)
	PTR_ADDU a3,a0,a2       /* a3 is the last dst address */
L(lastbloop):
	PTR_ADDIU a0,a0,1
	bne	a0,a3,L(lastbloop)
	sb	a1,-1(a0)
L(leave):
	j	ra
	nop
	.set	macro
	.set	reorder
	.end	__n64_memset_ASM
    .size   __n64_memset_ASM, .-__n64_memset_ASM	
#endif	

#if 0
/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Hartvig Ekner <hartvige@mips.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* void *memset(void *s, int c, size_t n).  */

# define SWHI	swl		/* high part is left in big-endian	*/


#define zero $0
#define at $1
#define v0 $2
#define v1 $3
#define a0 $4
#define a1 $5
#define a2 $6
#define a3 $7
#define t0 $8
#define t1 $9
#define t2 $10
#define t3 $11
#define t4 $12
#define t5 $13
#define t6 $14
#define t7 $15
#define s0 $16
#define s1 $17
#define s2 $18
#define s3 $19
#define s4 $20
#define s5 $21
#define s6 $22
#define s7 $23
#define t8 $24
#define t9 $25
#define k0 $26
#define k1 $27
#define gp $28
#define sp $29
#define fp $30
#define ra $31

__n64_memset_ASM:
	.align 8
        .global __n64_memset_ASM

	.set	noreorder

	slti	t1, a2, 8		# Less than 8?
	bne	t1, zero, L_last8
	move	v0, a0			# Setup exit value before too late

	beq	a1, zero, L_ueven	# If zero pattern, no need to extend
	andi	a1, 0xff		# Avoid problems with bogus arguments
	dsll	t0, a1, 8
	or	a1, t0
	dsll	t0, a1, 16
	or	a1, t0			# a1 is now pattern in full word
	dsll32	t0, a1, 0
	or	a1, t0                  # a1 is now pattern in full dw
L_ueven:	
	subu	t0, zero, a0		# Unaligned address?
	andi	t0, 0x3
	beq	t0, zero, L_chkw
	subu	a2, t0
	SWHI	a1, 0(a0)		# Yes, handle first unaligned part
	addu	a0, t0			# Now both a0 and a2 are updated

L_chkw:	
	andi	t0, a2, 0x7		# Enough left for one loop iteration?
	beq	t0, a2, L_chkl
	subu	a3, a2, t0
	addu	a3, a0			# a3 is last loop address +1
	move	a2, t0			# a2 is now # of bytes left after loop
L_loopw:	
	addiu	a0, 8			# Handle 2 words pr. iteration
	sw	a1, -8(a0)
	bne	a0, a3, L_loopw
	sw	a1, -4(a0)
//        sd	a1, -8(a0)

L_chkl:	
	andi	t0, a2, 0x4		# Check if there is at least a full
	beq	t0, zero, L_last8	#  word remaining after the loop
	subu	a2, t0
	sw	a1, 0(a0)		# Yes...
	addiu	a0, 4

L_last8:	
	blez	a2, L_exit		# Handle last 8 bytes (if cnt>0)
	addu	a3, a2, a0		# a3 is last address +1
L_lst8l:	
	addiu	a0, 1
	bne	a0, a3, L_lst8l
	sb	a1, -1(a0)
L_exit:	
	j	ra			# Bye, bye
	nop

	.set	reorder


#experimental results: 32.0099 megaticks to run compared to 4.0015 megaticks for memsetASM
#__n64_zero_ASM:
#	.global __n64_zero_ASM
#	.set	noreorder
#
#	beq	a2, zero, L_endZero	# if length is zero, go to end
#	addu	t1, a2, zero		# put length into t1
#	addu	t2, a0, zero		# put pointer into t2
#L_nextZero:
#	sb	zero, 0(t2)		# store zero at pointer
#	addiu	t2, 1			# increment pointer by 1 byte
#	bne	t1, zero, L_nextZero	# if length is not zero, go again
#	addiu	t1, -1			# decrement length
#L_endZero:
#	j	ra
#	nop
#	.set	reorder





__n64_memset_ZERO_ASM:
	.align 8
        .global __n64_memset_ZERO_ASM

	.set	noreorder

	slti	t1, a2, 8		# Less than 8?
	bne	t1, zero, L_last8
	move	v0, a0			# Setup exit value before too late

L_uevenZERO:	
	subu	t0, zero, a0		# Unaligned address?
	andi	t0, 0x3
	beq	t0, zero, L_chkwZERO
	subu	a2, t0
	SWHI	zero, 0(a0)		# Yes, handle first unaligned part
	addu	a0, t0			# Now both a0 and a2 are updated

L_chkwZERO:	
	andi	t0, a2, 0x7		# Enough left for one loop iteration?
	beq	t0, a2, L_chklZERO
	subu	a3, a2, t0
	addu	a3, a0			# a3 is last loop address +1
	move	a2, t0			# a2 is now # of bytes left after loop
L_loopwZERO:	
	addiu	a0, 8			# Handle 2 words pr. iteration
	sw	zero, -8(a0)
	bne	a0, a3, L_loopwZERO
	sw	zero, -4(a0)
//	addiu	a0, 8
//	bne	a0, a3, L_loopwZERO
//	sd	zero, -8(a0)


L_chklZERO:	
	andi	t0, a2, 0x4		# Check if there is at least a full
	beq	t0, zero, L_last8ZERO	#  word remaining after the loop
	subu	a2, t0
	sw	zero, 0(a0)		# Yes...
	addiu	a0, 4

L_last8ZERO:	
	blez	a2, L_exitZERO		# Handle last 8 bytes (if cnt>0)
	addu	a3, a2, a0		# a3 is last address +1
L_lst8lZERO:	
	addiu	a0, 1
	bne	a0, a3, L_lst8lZERO
	sb	zero, -1(a0)
L_exitZERO:	
	j	ra			# Bye, bye
	nop

	.set	reorder
#endif