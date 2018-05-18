KERNEL_OBJECTS=kernelcore.o main.o console.o memory.o keyboard.o mouse.o clock.o interrupt.o kmalloc.o pic.o ata.o cdromfs.o string.o bitmap.o graphics.o font.o syscall_handler.o process.o mutex.o list.o pagetable.o rtc.o kshell.o fs.o kevinfs/kevinfs.o hashtable.o kevinfs/kevinfs_test.o kevinfs/kevinfs_ata.o serial.o elf.o device.o kobject.o buffer.o fs_space.o

USER_PROGRAMS=test.exe long.exe printer.exe frktst.exe exctst.exe saver.exe ball.exe copy.exe write.exe pwd.exe

LIBRARY_OBJECTS=test.o syscall.o syscalls.o string.o user-io.o malloc.o

KERNEL_CCFLAGS=-Wall -c -ffreestanding -fno-pie -g
CC=i686-elf-gcc
LD=i686-elf-ld
ISOGEN=genisoimage
OBJCOPY=i686-elf-objcopy

all: basekernel.iso

basekernel.iso: basekernel.img ${USER_PROGRAMS}
	${ISOGEN} -J -R -o $@ -b $^ $<

basekernel.img: bootblock kernel
	cat bootblock kernel /dev/zero | head -c 1474560 > basekernel.img

bootblock.elf: bootblock.o
	${LD} ${KERNEL_LDFLAGS} -Ttext 0 $< -o $@

kernel: kernel.elf
	${OBJCOPY} -O binary $< $@

bootblock: bootblock.elf
	${OBJCOPY} -O binary $< $@

kernel.elf: ${KERNEL_OBJECTS}
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x10000 ${KERNEL_OBJECTS} -o $@

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} $< -o $@

%.o: %.S
	${CC} ${KERNEL_CCFLAGS} $< -o $@

baselib.a: ${LIBRARY_OBJECTS}
	ar rv $@ $<

%.exe: %.o user-start.o baselib.a
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 user-start.o $< baselib.a-o $@

clean:
	rm -rf basekernel.iso $(KERNEL_OBJECTS) $(LIBRARY_OBJECTS) $(USER_PROGRAMS)
