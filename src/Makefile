OBJECTS=kernelcore.o main.o console.o memory.o keyboard.o mouse.o clock.o interrupt.o kmalloc.o pic.o ata.o cdromfs.o string.o bitmap.o graphics.o font.o syscall.o syscall_handler.o process.o mutex.o list.o pagetable.o rtc.o

KERNEL_CCFLAGS=-Wall -c -ffreestanding -m32 -march=i386
KERNEL_LDFLAGS=-m elf_i386
LD?=ld
ISOGEN?=genisoimage

all: basekernel.iso

basekernel.iso: basekernel.img
	${ISOGEN} -J -R -o basekernel.iso -b basekernel.img basekernel.img

basekernel.img: bootblock kernel
	cat bootblock kernel /dev/zero | head -c 1474560 > basekernel.img

bootblock: bootblock.o
	${LD} ${KERNEL_LDFLAGS} -Ttext 0 -s --oformat binary $< -o $@

kernel: ${OBJECTS}
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x10000 -s --oformat binary ${OBJECTS} -o $@

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} $< -o $@

%.o: %.S
	${CC} ${KERNEL_CCFLAGS} $< -o $@

clean:
	rm -rf basekernel.img bootblock kernel *.o
