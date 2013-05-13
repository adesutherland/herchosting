# Herchosting Common Library Makefile

OPS = -g

OBJS = authenticate.o dbconnect.o userhost.o printoutput.o

all : libherchosting.a

libherchosting.a : ${OBJS} 
	ar rcs libherchosting.a ${OBJS}

.cpp.o :
	g++ ${OPS} -c $< -I /usr/include/mysql
        
${OBJS} : herchosting.h

clean:
	rm *~ *.o *.a
