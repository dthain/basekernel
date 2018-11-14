include Makefile.config

LIBRARY_SOURCES=$(wildcard library/*.c)
LIBRARY_HEADERS=$(wildcard library/*.h)
USER_SOURCES=$(wildcard user/*.c)
USER_PROGRAMS=$(USER_SOURCES:c=exe)
KERNEL_SOURCES=$(wildcard kernel/*.[chS])

all: basekernel.iso

run: basekernel.iso
	qemu-system-i386 -cdrom basekernel.iso

library/baselib.a: $(LIBRARY_SOURCES) $(LIBRARY_HEADERS)
	cd library && make

$(USER_PROGRAMS): $(USER_SOURCES) library/baselib.a $(LIBRARY_HEADERS)
	cd user && make

kernel/basekernel.img: $(KERNEL_SOURCES) $(LIBRARY_HEADERS)
	cd kernel && make

image: kernel/basekernel.img $(USER_PROGRAMS)
	rm -rf image
	mkdir image image/boot image/bin
	cp kernel/basekernel.img image/boot
	cp $(USER_PROGRAMS) image/bin

basekernel.iso: image
	${ISOGEN} -input-charset utf-8 -iso-level 2 -J -R -o $@ -b boot/basekernel.img image

clean:
	rm -rf basekernel.iso image
	cd kernel && make clean
	cd library && make clean
	cd user && make clean
