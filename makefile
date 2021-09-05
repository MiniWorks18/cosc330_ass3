COMPILER = mpicc
CFLAGS = -Wall -pedantic

EXES = blur testing printMat

all: ${EXES}

blur: blur.c
	${COMPILER} ${CFLAGS} blur.c -o blur

testing: testing.c
	${COMPILER} ${CFLAGS} testing.c -o testing

printMat: printMat.c
	${COMPILER} ${CFLAGS} printMat.c -o printMat

clean:
	rm -f *.o ${EXES}