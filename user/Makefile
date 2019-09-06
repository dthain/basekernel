
include ../Makefile.config

USER_PROGRAMS=test.exe long.exe printer.exe forktest.exe errcodes.exe exectest.exe filedescribetest.exe saver.exe ball.exe write.exe shell.exe pipetest.exe snake.exe sysstat.exe procstat.exe writebig.exe manager.exe clock.exe mandelbrot.exe open.exe copy.exe livestat.exe statworkload.exe

all: $(USER_PROGRAMS)

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} -I ../include $< -o $@

%.exe: %.o ../library/user-start.o ../library/baselib.a
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 ../library/user-start.o $< ../library/baselib.a -o $@

clean:
	rm -rf *.exe *.o
