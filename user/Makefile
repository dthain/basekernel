
include ../Makefile.config

USER_PROGRAMS=ball.exe clock.exe copy.exe livestat.exe manager.exe fractal.exe procstat.exe saver.exe shell.exe snake.exe sysstat.exe

all: $(USER_PROGRAMS)

%.o: %.c
	${CC} ${USER_CCFLAGS} -I ../include $< -o $@

%.exe: %.o ../library/user-start.o ../library/baselib.a
	${LD} ${USER_LDFLAGS} $< ../library/baselib.a -o $@

clean:
	rm -rf *.exe *.o
