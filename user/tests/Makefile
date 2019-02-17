include ../../Makefile.config

TESTS=long.exe printer.exe test.exe exectest.exe forktest.exe errcodes.exe filedescribetest.exe pipetest.exe sysstat.exe procstat.exe writebig.exe open.exe write.exe

all: $(TESTS)

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} -I ../../include $< -o $@

%.exe: %.o ../../library/user-start.o ../../library/baselib.a
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 ../../library/user-start.o $< ../../library/baselib.a -o $@

clean:
	rm -rf *.exe *.o
