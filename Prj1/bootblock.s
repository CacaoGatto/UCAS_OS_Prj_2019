.text
.global main

main:
	# 1) task1 call BIOS print string "It's bootblock!"
	
	lw	$t2, printstr	#从内存区域装载函数地址(.word)
	la	$a0, msg		#使用la伪指令装载字符串首地址(.ascii)
	jal	$t2				#跳转并链接回本程序继续执行
	
	# 2) task2 call BIOS read kernel in SD card and jump to kernel start

	lw	$t1, read_sd_card	#装载sd卡读取函数地址，接下来按顺序在参数寄存器准备传入值
	lw	$a0, kernel     	#装载拷贝的目标位置
	li	$a1, 512			#由于第一个扇区交给loadboot，故kernel有512B的偏移量
	li	$a2, 512			#需要拷贝的代码大小为512B
#	lw  $ra, kernel
#    jr  $t1
	jal	$t1
	lw	$t0, kernel_main	#装载kernel函数地址，把运行交给kernel完成
	jal	$t0

# while(1) --> stop here
stop: 
	j stop

.data

msg: .asciiz "It's bootblock!\n"

# 1. PMON read SD card function address
# read_sd_card();
read_sd_card: .word 0x80011000

# 2. PMON print string function address
# printstr(char *string)
printstr: .word 0x80011100

# 3. PMON print char function address
# printch(char ch)
printch: .word 0x80011140

# 4. kernel address (move kernel to here ~)
kernel : .word 0xa0800200

# 5. kernel main address (jmp here to start kernel main!)
kernel_main : .word 0xa0800200
