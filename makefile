COMPILER = mpicc
CFLAGS = -Wall -pedantic

EXES = blur testing

all: ${EXES}

blur: blur.c
	${COMPILER} ${CFLAGS} blur.c -o blur

testing: testing.c
	${COMPILER} ${CFLAGS} testing.c -o testing

clean:
	rm -f *.o ${EXES}