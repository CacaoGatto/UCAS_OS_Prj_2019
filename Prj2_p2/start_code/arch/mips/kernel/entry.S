#include "asm.h"

.equ	KERNEL, 0
.equ	USER,   156
.equ	TIME_SLICE, 150000

.macro STI
	mfc0	k0, CP0_STATUS
	ori		k0, 0x1
	mtc0	k0, CP0_STATUS
.endm

.macro CLI
	mfc0	k0, CP0_STATUS
	li		k1, 0xfffffffe
	and		k0, k0, k1
	mtc0	k0, CP0_STATUS
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    jr  ra
END(asm_start)

.macro SAVE_CONTEXT offset
    // TODO save context
	lw		k0,current_running
	addi	k0,\offset
	sw		zero,0(k0)
#	sw		at,4(k0)
	sw		v0,8(k0)
	sw		v1,12(k0)
	sw		a0,16(k0)
	sw		a1,20(k0)
	sw		a2,24(k0)
	sw		a3,28(k0)
	sw		t0,32(k0)
	sw		t1,36(k0)
	sw		t2,40(k0)
	sw		t3,44(k0)
	sw		t4,48(k0)
	sw		t5,52(k0)
	sw		t6,56(k0)
	sw		t7,60(k0)
	sw		s0,64(k0)
	sw		s1,68(k0)
	sw		s2,72(k0)
	sw		s3,76(k0)
	sw		s4,80(k0)
	sw		s5,84(k0)
	sw		s6,88(k0)
	sw		s7,92(k0)
	sw		t8,96(k0)
	sw		t9,100(k0)
	sw		gp,112(k0)
	sw		sp,116(k0)
	sw		fp,120(k0)
	sw		ra,124(k0)
	
	mfc0	k1,CP0_STATUS
	sw		k1,128(k0)
	mfhi	k1
	sw		k1,132(k0)
	mflo	k1
	sw		k1,136(k0)
	mtc0	k1,CP0_BADVADDR
	sw		k1,140(k0)
	mfc0	k1,CP0_CAUSE
	sw		k1,144(k0)
	mfc0	k1,CP0_EPC
	sw		k1,148(k0)
.endm

.macro RESTORE_CONTEXT offset
    // TODO restore context
	lw		k0,current_running
	addi	k0,\offset
	lw		zero,0(k0)
#	lw		at,4(k0)
	lw		v0,8(k0)
	lw		v1,12(k0)
	lw		a0,16(k0)
	lw		a1,20(k0)
	lw		a2,24(k0)
	lw		a3,28(k0)
	lw		t0,32(k0)
	lw		t1,36(k0)
	lw		t2,40(k0)
	lw		t3,44(k0)
	lw		t4,48(k0)
	lw		t5,52(k0)
	lw		t6,56(k0)
	lw		t7,60(k0)
	lw		s0,64(k0)
	lw		s1,68(k0)
	lw		s2,72(k0)
	lw		s3,76(k0)
	lw		s4,80(k0)
	lw		s5,84(k0)
	lw		s6,88(k0)
	lw		s7,92(k0)
	lw		t8,96(k0)
	lw		t9,100(k0)
	lw		gp,112(k0)
	lw		sp,116(k0)
	lw		fp,120(k0)
	lw		ra,124(k0)
	
	lw		k1,128(k0)
	mtc0	k1,CP0_STATUS
	lw		k1,132(k0)
	mthi	k1
	lw		k1,136(k0)
	mtlo	k1
	lw		k1,140(k0)
	mtc0	k1,CP0_BADVADDR
	lw		k1,144(k0)
	mtc0	k1,CP0_CAUSE
	lw		k1,148(k0)
	mtc0	k1,CP0_EPC
.endm

NESTED(do_scheduler, 0, ra)
	SAVE_CONTEXT(KERNEL)
	jal		scheduler
	RESTORE_CONTEXT(KERNEL)
	jr		ra
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    // TODO close interrupt
	SAVE_CONTEXT(USER)
    // jmp exception_handler[i] which decided by CP0_CAUSE
	mfc0	t0, CP0_CAUSE
	andi	t0, CAUSE_EXCCODE
	la		t1, exception_handler
	add		t1, t1, t0
	lw		t0, (t1)
	jr		t0
    // Leve2 exception Handler.
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt handler
    // Leve3 exception Handler.
	mfc0	a0, CP0_STATUS
	mfc0	a1, CP0_CAUSE
	addiu	sp, sp, -8
	jal 	interrupt_helper
	addiu	sp, sp, 8
	RESTORE_CONTEXT(USER)
	mtc0	zero, CP0_COUNT
	li		k0, TIME_SLICE
	mtc0	k0, CP0_COMPARE
	eret
END(handle_int)

NESTED(handle_syscall, 0, sp)
    // system call handler
	addiu	sp, sp, -16
	jal 	system_call_helper
	addiu	sp, sp, 16
	RESTORE_CONTEXT(USER)
	
	mfc0	k0, CP0_EPC
	addi	k0, 4
	mtc0	k0, CP0_EPC
	eret
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
	mtc0	zero, CP0_COUNT
	li		k0, TIME_SLICE
	mtc0	k0, CP0_COMPARE
	eret
END(handle_other)

LEAF(enable_int)
	mfc0	k0, CP0_STATUS
	ori		k0, 0x1
    mtc0	k0, CP0_STATUS
    jr		ra
END(enable_int)

LEAF(disable_int)
	mfc0	k0, CP0_STATUS
	li		k1, 0xfffffffe
	and		k0, k0, k1
    mtc0	k0, CP0_STATUS
    jr		ra
END(disable_int)

LEAF(init_int)
	li		k0, 0x10008001
    mtc0	k0, CP0_STATUS
    jr		ra
END(init_int)

LEAF(save_clk)
    mfc0	v0, CP0_COUNT
    jr		ra
END(save_clk)

LEAF(restore_clk)
    mtc0	a0, CP0_COUNT
    jr		ra
END(restore_clk)

LEAF(init_clk)
    mtc0	zero, CP0_COUNT
	li		k0, TIME_SLICE
	mtc0	k0, CP0_COMPARE
    jr		ra
END(init_clk)


