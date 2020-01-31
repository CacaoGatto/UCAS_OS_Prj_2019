CC = mipsel-linux-gcc -g3 -gdwarf-2

all: clean createimage image plane asm # floppy

SRC_BOOT 	= ./arch/mips/boot/bootblock.S

SRC_ARCH	= ./arch/mips/kernel/entry.S ./arch/mips/kernel/syscall.S ./arch/mips/pmon/common.c 
SRC_DRIVER	= ./drivers/screen.c ./drivers/mac.c
SRC_INIT 	= ./init/main.c
SRC_INT		= ./kernel/irq/irq.c
SRC_LOCK	= ./kernel/locking/lock.c
SRC_MM          = ./kernel/mm/memory.c
SRC_SCHED	= ./kernel/sched/sched.c ./kernel/sched/queue.c ./kernel/sched/time.c
SRC_SYSCALL	= ./kernel/syscall/syscall.c
SRC_FS          = ./kernel/fs/fs.c
SRC_LIBS	= ./libs/string.c ./libs/printk.c

SRC_SHELL	= ./test/test_shell.c
SRC_RECV	= ./test/bonus.c
SRC_PLANE= ./arch/mips/kernel/syscall.S ./test/test_bonus/printk.c ./test/test_bonus/syscall.c ./test/test_bonus/drawing.c

SRC_IMAGE	= ./tools/createimage.c

bootblock: $(SRC_BOOT)
	${CC} -G 0 -O2 -fno-pic -mno-abicalls -fno-builtin -nostdinc -mips3 -Ttext=0xffffffffa0800000 -N -o bootblock $(SRC_BOOT) -nostdlib -e main -Wl,-m -Wl,elf32ltsmip -T ld.script

main : $(SRC_ARCH) $(SRC_DRIVER) $(SRC_INIT) $(SRC_INT) $(SRC_LOCK) $(SRC_MM) $(SRC_SCHED) $(SRC_SYSCALL) $(SRC_LIBS) $(SRC_FS) $(SRC_RECV) $(SRC_SHELL)
	${CC} -G 0 -O0 -Iinclude -Ilibs -Iarch/mips/include -Idrivers -Iinclude/os -Iinclude/sys -Itest \
	-fno-pic -mno-abicalls -fno-builtin -nostdinc -mips3 -Ttext=0xffffffffa0800200 -N -g -o main \
	$(SRC_ARCH) $(SRC_DRIVER) $(SRC_INIT) $(SRC_INT) $(SRC_LOCK) $(SRC_MM) $(SRC_SCHED) $(SRC_SYSCALL) $(SRC_LIBS) $(SRC_FS) $(SRC_RECV) $(SRC_SHELL) -nostdlib -Wl,-m -Wl,elf32ltsmip -T ld.script -L libs/ -lepmon

plane : $(SRC_PLANE)
	${CC} -G 0 -O0 -Itest/test_bonus \
	-fno-pic -mno-abicalls -fno-builtin -nostdinc -mips3 -Ttext=0x0000000040000000 -N -g -o plane \
	$(SRC_PLANE) -nostdlib -Wl,-m -Wl,elf32ltsmip -T ldt.script

createimage: $(SRC_IMAGE)
	gcc $(SRC_IMAGE) -o createimage

image: bootblock main
	./createimage --extended bootblock main

clean:
	rm -rf bootblock image createimage main plane *.o

floppy:
	sudo fdisk -l /dev/sdb
	sudo dd if=image of=/dev/sdb conv=notrunc

asm:
	mipsel-linux-objdump -d main > kernel.txt
	mipsel-linux-objdump -d plane > plane.txt