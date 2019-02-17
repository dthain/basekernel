
include ../Makefile.config

MAIN_PROGRAMS=saver.exe ball.exe shell.exe snake.exe manager.exe clock.exe mandelbrot.exe copy.exe
TEST_SOURCES=$(wildcard tests/*.c)
TEST_PROGRAMS=$(TEST_SOURCES:c=exe)

all: $(MAIN_PROGRAMS) $(TEST_PROGRAMS)

$(TEST_PROGRAMS): $(TEST_SOURCES)
	cd tests && make

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} -I ../include $< -o $@

%.exe: %.o ../library/user-start.o ../library/baselib.a
	${LD} ${KERNEL_LDFLAGS} -Ttext 0x80000000 ../library/user-start.o $< ../library/baselib.a -o $@

clean:
	rm -rf *.exe *.o
	cd tests && make clean
