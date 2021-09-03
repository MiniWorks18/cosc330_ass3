COMPILER = mpicc
CFLAGS = -Wall -pedantic

EXES = blur

all: ${EXES}

blur: blur.c
	${COMPILER} ${CFLAGS} blur.c -o blur

clean:
	rm -f *.o ${EXES}