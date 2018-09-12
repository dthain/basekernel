include ../Makefile.config

USER_PROGRAMS=test.exe long.exe printer.exe frktst.exe exctst.exe file_describe_test.exe saver.exe ball.exe write.exe pwd.exe shell.exe piptst.exe

all: $(USER_PROGRAMS)

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} -I ../include $< -o $@

%.exe: %.o ../library/user-start.o ../library/baselib.a
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 ../library/user-start.o $< ../library/baselib.a -o $@

clean:
	rm -rf *.exe *.o
