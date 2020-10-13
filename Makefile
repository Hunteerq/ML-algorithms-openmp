CPP = /usr/local/opt/llvm/bin/clang
CFLAGS = -I/usr/local/opt/llvm/include

LD = g++
LDFLAGS = -L/usr/local/opt/llvm/lib

PROGRAM = min_max

all:	${PROGRAM}.o ${PROGRAM}

${PROGRAM}.o: ${PROGRAM}.cpp
	${CC} ${CFLAGS} -c $< -o ${PROGRAM}.o

${PROGRAM}: ${PROGRAM}.o
	${LD} $< -o ${PROGRAM}
clean:
	rm -f ${PROGRAM}.o ${PROGRAM}



