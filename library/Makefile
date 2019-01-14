include ../Makefile.config

LIBRARY_OBJECTS=errno.o objno.o syscall.o syscalls.o string.o user-io.o malloc.o

all: user-start.o baselib.a

%.o: %.c
	${CC} ${KERNEL_CCFLAGS} -I ../include $< -o $@

baselib.a: ${LIBRARY_OBJECTS}
	${AR} rv $@ ${LIBRARY_OBJECTS}

clean:
	rm -rf *.a *.o
