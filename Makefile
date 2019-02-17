include Makefile.config

LIBRARY_SOURCES=$(wildcard library/*.c)
LIBRARY_HEADERS=$(wildcard library/*.h)

USER_MAIN_SOURCES=$(wildcard user/*.c)
USER_TEST_SOURCES=$(wildcard user/tests/*.c)
USER_MAIN_PROGRAMS=$(USER_MAIN_SOURCES:c=exe)
USER_TEST_PROGRAMS=$(USER_TEST_SOURCES:c=exe)
USER_PROGRAMS=$(USER_MAIN_PROGRAMS) $(USER_TEST_PROGRAMS)

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

$(USER_PROGRAMS): $(USER_MAIN_SOURCES) $(USER_TEST_SOURCES) library/baselib.a $(LIBRARY_HEADERS)
	cd user && make

kernel/basekernel.img: $(KERNEL_SOURCES) $(LIBRARY_HEADERS)
	cd kernel && make

image: kernel/basekernel.img $(USER_PROGRAMS)
	rm -rf image
	mkdir image image/boot image/bin image/data image/bin/tests
	cp kernel/basekernel.img image/boot
	cp $(USER_MAIN_PROGRAMS) image/bin
	cp $(USER_TEST_PROGRAMS) image/bin/tests
	head -2000 /usr/share/dict/words > image/data/words

basekernel.iso: image
	${ISOGEN} -input-charset utf-8 -iso-level 2 -J -R -o $@ -b boot/basekernel.img image

clean:
	rm -rf basekernel.iso image
	cd kernel && make clean
	cd library && make clean
	cd user && make clean
