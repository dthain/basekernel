KERNEL_OBJECTS=kernelcore.o main.o console.o memory.o keyboard.o mouse.o clock.o interrupt.o kmalloc.o pic.o ata.o cdromfs.o string.o bitmap.o graphics.o font.o syscall_handler.o process.o mutex.o list.o pagetable.o rtc.o kshell.o fs.o

USER_OBJECTS=user-start.o test.o syscall.o syscalls.o string.o user-io.o

KERNEL_CCFLAGS=-Wall -c -ffreestanding -fno-pie -g
CC=i686-elf-gcc
LD=i686-elf-ld
OBJCOPY?=objcopy
ISOGEN?=genisoimage

all: basekernel.iso

basekernel.iso: basekernel.img test.exe saver.exe ball.exe long.exe
	${ISOGEN} -J -R -o $@ -b $^ $<

basekernel.img: bootblock kernel
	cat bootblock kernel /dev/zero | head -c 1474560 > basekernel.img

bootblock.elf: bootblock.o
	${LD} ${KERNEL_LDFLAGS} -Ttext 0 $< -o $@

kernel: kernel.elf
	${OBJCOPY} -O binary $< $@

bootblock: bootblock.elf
	${OBJCOPY} -O binary $< $@

%.exe: %.elf
	${OBJCOPY} -O binary $< $@

kernel.elf: ${KERNEL_OBJECTS}
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x10000 ${KERNEL_OBJECTS} -o $@

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} $< -o $@

%.o: %.S
	${CC} ${KERNEL_CCFLAGS} $< -o $@

test.elf: test.o $(USER_OBJECTS)
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 user-start.o $< syscall.o syscalls.o string.o user-io.o -o $@

long.elf: long.o $(USER_OBJECTS)
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 user-start.o $< syscall.o syscalls.o string.o user-io.o -o $@

saver.elf: saver.o $(USER_OBJECTS)
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 user-start.o $< syscall.o syscalls.o string.o user-io.o -o $@

ball.elf: ball.o $(USER_OBJECTS)
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 user-start.o $< syscall.o syscalls.o user-io.o string.o -o $@

clean:
	rm -rf basekernel.img bootblock kernel *.o test.exe saver.exe ball.exe long.exe
