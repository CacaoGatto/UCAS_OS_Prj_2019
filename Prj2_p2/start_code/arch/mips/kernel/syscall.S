#include "asm.h"

LEAF(invoke_syscall)
    // TODO syscall entry
	syscall
	jr	ra
END(invoke_syscall)