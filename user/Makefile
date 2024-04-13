include ../Makefile.config

USER_PROGRAMS=ball.exe clock.exe copy.exe livestat.exe manager.exe fractal.exe procstat.exe saver.exe shell.exe snake.exe sysstat.exe process1.exe process2.exe process3.exe process4.exe process5.exe schedulertest.exe named_pipe_test.exe

all: $(USER_PROGRAMS)

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} -I ../kernel -I ../include $< -o $@


%.exe: %.o ../library/user-start.o ../library/baselib.a
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 ../library/user-start.o $< ../library/baselib.a -o $@

clean:
	rm -rf *.exe *.o
 