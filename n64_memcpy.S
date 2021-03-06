/*
 * Copyright (c) 2012
 *	MIPS Technologies, Inc., California.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the MIPS Technologies, Inc., nor the names of its
 *	contributors may be used to endorse or promote products derived from
 *	this software without specific prior written permission.
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
#define __mips 3
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

#ifndef L
# define L(label) $L ## label
#endif
#ifndef PTR_ADDIU
#define PTR_ADDIU addiu
#endif

#define PTR_SUBU dsubu
#define PTR_ADDU daddu

#ifndef PTR_SRA
#define PTR_SRA sra
#endif

#define PREFETCH_FOR_LOAD(offset, reg)
#define PREFETCH_FOR_STORE(offset, reg)

#define REG0 t0
#define REG1 t1
#define REG2 t2
#define REG3 t3
#define REG4 t4
#define REG5 t5
#define REG6 t6
#define REG7 t7
#define C_ST sw
#define C_LD lw
#define C_LDHI lwl
#define C_STHI swl
#define C_LDLO lwr
#define C_STLO swr
#define NSIZE 4
#define NSIZEMASK 0x1f
#define NSIZEDMASK 0x3f
#define UNIT(unit) ((unit)*NSIZE)
#define UNITM1(unit) (((unit)*NSIZE)-1)

	.align 8
	.global	__n64_memcpy_ASM
	.set	nomips16
	.set	nomicromips
	.set	noreorder
	.set	nomacro
	.ent	__n64_memcpy_ASM
	.type	__n64_memcpy_ASM,	@function

__n64_memcpy_ASM:
	slti	t2,a2,(2 * NSIZE)
	bne	t2,zero,L(lastb)
	move	v0,a0
	xor	t8,a1,a0
	andi	t8,t8,(NSIZE-1)
	bne	t8,zero,L(unaligned)
	PTR_SUBU a3, zero, a0
	andi	a3,a3,(NSIZE-1)
	beq	a3,zero,L(aligned)
	PTR_SUBU a2,a2,a3
	C_LDHI	t8,0(a1)
	PTR_ADDU a1,a1,a3
	C_STHI	t8,0(a0)
	PTR_ADDU a0,a0,a3
L(aligned):
	andi	t8,a2,NSIZEDMASK
	beq	a2,t8,L(chkw)
	PTR_SUBU a3,a2,t8
	PTR_ADDU a3,a0,a3
L(loop16w):
	C_LD	t0,UNIT(0)(a1)
	C_LD	t1,UNIT(1)(a1)
L(skip_pref):
	C_LD	REG2,UNIT(2)(a1)
	C_LD	REG3,UNIT(3)(a1)
	C_LD	REG4,UNIT(4)(a1)
	C_LD	REG5,UNIT(5)(a1)
	C_LD	REG6,UNIT(6)(a1)
	C_LD	REG7,UNIT(7)(a1)
	C_ST	t0,UNIT(0)(a0)
	C_ST	t1,UNIT(1)(a0)
	C_ST	REG2,UNIT(2)(a0)
	C_ST	REG3,UNIT(3)(a0)
	C_ST	REG4,UNIT(4)(a0)
	C_ST	REG5,UNIT(5)(a0)
	C_ST	REG6,UNIT(6)(a0)
	C_ST	REG7,UNIT(7)(a0)
	C_LD	t0,UNIT(8)(a1)
	C_LD	t1,UNIT(9)(a1)
	C_LD	REG2,UNIT(10)(a1)
	C_LD	REG3,UNIT(11)(a1)
	C_LD	REG4,UNIT(12)(a1)
	C_LD	REG5,UNIT(13)(a1)
	C_LD	REG6,UNIT(14)(a1)
	C_LD	REG7,UNIT(15)(a1)
	C_ST	t0,UNIT(8)(a0)
	C_ST	t1,UNIT(9)(a0)
	C_ST	REG2,UNIT(10)(a0)
	C_ST	REG3,UNIT(11)(a0)
	C_ST	REG4,UNIT(12)(a0)
	C_ST	REG5,UNIT(13)(a0)
	C_ST	REG6,UNIT(14)(a0)
	C_ST	REG7,UNIT(15)(a0)
	PTR_ADDIU a0,a0,UNIT(16)
	bne	a0,a3,L(loop16w)
	PTR_ADDIU a1,a1,UNIT(16)
	move	a2,t8
L(chkw):
	andi	t8,a2,NSIZEMASK
	beq	a2,t8,L(chk1w)
	nop
	C_LD	t0,UNIT(0)(a1)
	C_LD	t1,UNIT(1)(a1)
	C_LD	REG2,UNIT(2)(a1)
	C_LD	REG3,UNIT(3)(a1)
	C_LD	REG4,UNIT(4)(a1)
	C_LD	REG5,UNIT(5)(a1)
	C_LD	REG6,UNIT(6)(a1)
	C_LD	REG7,UNIT(7)(a1)
	PTR_ADDIU a1,a1,UNIT(8)
	C_ST	t0,UNIT(0)(a0)
	C_ST	t1,UNIT(1)(a0)
	C_ST	REG2,UNIT(2)(a0)
	C_ST	REG3,UNIT(3)(a0)
	C_ST	REG4,UNIT(4)(a0)
	C_ST	REG5,UNIT(5)(a0)
	C_ST	REG6,UNIT(6)(a0)
	C_ST	REG7,UNIT(7)(a0)
	PTR_ADDIU a0,a0,UNIT(8)
L(chk1w):
	andi	a2,t8,(NSIZE-1)
	beq	a2,t8,L(lastb)
	PTR_SUBU a3,t8,a2
	PTR_ADDU a3,a0,a3
L(wordCopy_loop):
	C_LD	REG3,UNIT(0)(a1)
	PTR_ADDIU a0,a0,UNIT(1)
	PTR_ADDIU a1,a1,UNIT(1)
	bne	a0,a3,L(wordCopy_loop)
	C_ST	REG3,UNIT(-1)(a0)
L(lastb):
	blez	a2,L(leave)
	PTR_ADDU a3,a0,a2
L(lastbloop):
	lb	v1,0(a1)
	PTR_ADDIU a0,a0,1
	PTR_ADDIU a1,a1,1
	bne	a0,a3,L(lastbloop)
	sb	v1,-1(a0)
L(leave):
	j	ra
	nop
L(unaligned):
	andi	a3,a3,(NSIZE-1)
	beqz	a3,L(ua_chk16w)
	PTR_SUBU a2,a2,a3
	C_LDHI	v1,UNIT(0)(a1)
	C_LDLO	v1,UNITM1(1)(a1)
	PTR_ADDU a1,a1,a3
	C_STHI	v1,UNIT(0)(a0)
	PTR_ADDU a0,a0,a3
L(ua_chk16w):
	andi	t8,a2,NSIZEDMASK
	beq	a2,t8,L(ua_chkw)
	PTR_SUBU a3,a2,t8
	PTR_ADDU a3,a0,a3
L(ua_loop16w):
	C_LDHI	t0,UNIT(0)(a1)
	C_LDHI	t1,UNIT(1)(a1)
	C_LDHI	REG2,UNIT(2)(a1)
	C_LDHI	REG3,UNIT(3)(a1)
L(ua_skip_pref):
	C_LDHI	REG4,UNIT(4)(a1)
	C_LDHI	REG5,UNIT(5)(a1)
	C_LDHI	REG6,UNIT(6)(a1)
	C_LDHI	REG7,UNIT(7)(a1)
	C_LDLO	t0,UNITM1(1)(a1)
	C_LDLO	t1,UNITM1(2)(a1)
	C_LDLO	REG2,UNITM1(3)(a1)
	C_LDLO	REG3,UNITM1(4)(a1)
	C_LDLO	REG4,UNITM1(5)(a1)
	C_LDLO	REG5,UNITM1(6)(a1)
	C_LDLO	REG6,UNITM1(7)(a1)
	C_LDLO	REG7,UNITM1(8)(a1)
	C_ST	t0,UNIT(0)(a0)
	C_ST	t1,UNIT(1)(a0)
	C_ST	REG2,UNIT(2)(a0)
	C_ST	REG3,UNIT(3)(a0)
	C_ST	REG4,UNIT(4)(a0)
	C_ST	REG5,UNIT(5)(a0)
	C_ST	REG6,UNIT(6)(a0)
	C_ST	REG7,UNIT(7)(a0)
	C_LDHI	t0,UNIT(8)(a1)
	C_LDHI	t1,UNIT(9)(a1)
	C_LDHI	REG2,UNIT(10)(a1)
	C_LDHI	REG3,UNIT(11)(a1)
	C_LDHI	REG4,UNIT(12)(a1)
	C_LDHI	REG5,UNIT(13)(a1)
	C_LDHI	REG6,UNIT(14)(a1)
	C_LDHI	REG7,UNIT(15)(a1)
	C_LDLO	t0,UNITM1(9)(a1)
	C_LDLO	t1,UNITM1(10)(a1)
	C_LDLO	REG2,UNITM1(11)(a1)
	C_LDLO	REG3,UNITM1(12)(a1)
	C_LDLO	REG4,UNITM1(13)(a1)
	C_LDLO	REG5,UNITM1(14)(a1)
	C_LDLO	REG6,UNITM1(15)(a1)
	C_LDLO	REG7,UNITM1(16)(a1)
	C_ST	t0,UNIT(8)(a0)
	C_ST	t1,UNIT(9)(a0)
	C_ST	REG2,UNIT(10)(a0)
	C_ST	REG3,UNIT(11)(a0)
	C_ST	REG4,UNIT(12)(a0)
	C_ST	REG5,UNIT(13)(a0)
	C_ST	REG6,UNIT(14)(a0)
	C_ST	REG7,UNIT(15)(a0)
	PTR_ADDIU a0,a0,UNIT(16)
	bne	a0,a3,L(ua_loop16w)
	PTR_ADDIU a1,a1,UNIT(16)
	move	a2,t8
L(ua_chkw):
	andi	t8,a2,NSIZEMASK
	beq	a2,t8,L(ua_chk1w)
	nop
	C_LDHI	t0,UNIT(0)(a1)
	C_LDHI	t1,UNIT(1)(a1)
	C_LDHI	REG2,UNIT(2)(a1)
	C_LDHI	REG3,UNIT(3)(a1)
	C_LDHI	REG4,UNIT(4)(a1)
	C_LDHI	REG5,UNIT(5)(a1)
	C_LDHI	REG6,UNIT(6)(a1)
	C_LDHI	REG7,UNIT(7)(a1)
	C_LDLO	t0,UNITM1(1)(a1)
	C_LDLO	t1,UNITM1(2)(a1)
	C_LDLO	REG2,UNITM1(3)(a1)
	C_LDLO	REG3,UNITM1(4)(a1)
	C_LDLO	REG4,UNITM1(5)(a1)
	C_LDLO	REG5,UNITM1(6)(a1)
	C_LDLO	REG6,UNITM1(7)(a1)
	C_LDLO	REG7,UNITM1(8)(a1)
	PTR_ADDIU a1,a1,UNIT(8)
	C_ST	t0,UNIT(0)(a0)
	C_ST	t1,UNIT(1)(a0)
	C_ST	REG2,UNIT(2)(a0)
	C_ST	REG3,UNIT(3)(a0)
	C_ST	REG4,UNIT(4)(a0)
	C_ST	REG5,UNIT(5)(a0)
	C_ST	REG6,UNIT(6)(a0)
	C_ST	REG7,UNIT(7)(a0)
	PTR_ADDIU a0,a0,UNIT(8)
L(ua_chk1w):
	andi	a2,t8,(NSIZE-1)
	beq	a2,t8,L(ua_smallCopy)
	PTR_SUBU a3,t8,a2
	PTR_ADDU a3,a0,a3
L(ua_wordCopy_loop):
	C_LDHI	v1,UNIT(0)(a1)
	C_LDLO	v1,UNITM1(1)(a1)
	PTR_ADDIU a0,a0,UNIT(1)
	PTR_ADDIU a1,a1,UNIT(1)
	bne	a0,a3,L(ua_wordCopy_loop)
	C_ST	v1,UNIT(-1)(a0)
L(ua_smallCopy):
	beqz	a2,L(leave)
	PTR_ADDU a3,a0,a2
L(ua_smallCopy_loop):
	lb	v1,0(a1)
	PTR_ADDIU a0,a0,1
	PTR_ADDIU a1,a1,1
	bne	a0,a3,L(ua_smallCopy_loop)
	sb	v1,-1(a0)
	j	ra
	nop
	
	.set	macro
	.set	reorder
	.end	__n64_memcpy_ASM
	.size   __n64_memcpy_ASM, .-__n64_memcpy_ASM
