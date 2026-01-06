
include ../Makefile.config

USER_PROGRAMS=ball.exe clock.exe copy.exe livestat.exe manager.exe fractal.exe procstat.exe saver.exe shell.exe snake.exe sysstat.exe

all: $(USER_PROGRAMS)

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} -I ../include $< -o $@

%.exe: %.o ../library/user-start.o ../library/baselib.a
	${LD} -z max-page-size=4096 -T basekernel.user.ldscript $< ../library/baselib.a -o $@ -Map $@.map

clean:
	rm -rf *.exe *.o
