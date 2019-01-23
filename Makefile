include Makefile.config

LIBRARY_SOURCES=$(wildcard library/*.c)
LIBRARY_HEADERS=$(wildcard library/*.h)
USER_SOURCES=$(wildcard user/*.c)
USER_PROGRAMS=$(USER_SOURCES:c=exe)
KERNEL_SOURCES=$(wildcard kernel/*.[chS])

all: basekernel.iso

run: basekernel.iso disk.img
	qemu-system-i386 -cdrom basekernel.iso -hda disk.img

debug: basekernel.iso disk.img
	qemu-system-i386 -cdrom basekernel.iso -hda disk.img -s -S &

disk.img:
	qemu-img create disk.img 10M

library/baselib.a: $(LIBRARY_SOURCES) $(LIBRARY_HEADERS)
	cd library && make

$(USER_PROGRAMS): $(USER_SOURCES) library/baselib.a $(LIBRARY_HEADERS)
	cd user && make

kernel/basekernel.img: $(KERNEL_SOURCES) $(LIBRARY_HEADERS)
	cd kernel && make

image: kernel/basekernel.img $(USER_PROGRAMS)
	rm -rf image
	mkdir image image/boot image/bin image/data
	cp kernel/basekernel.img image/boot
	cp $(USER_PROGRAMS) image/bin
	head -2000 /usr/share/dict/words > image/data/words

basekernel.iso: image
	${ISOGEN} -input-charset utf-8 -iso-level 2 -J -R -o $@ -b boot/basekernel.img image

clean:
	rm -rf basekernel.iso image
	cd kernel && make clean
	cd library && make clean
	cd user && make clean
