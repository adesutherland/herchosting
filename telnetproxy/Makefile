# telnetProxy Makefile

all : telnetProxy typoauth

telnetProxy : telnetProxy.o
	g++ -o telnetProxy telnetProxy.o -lpthread -L /usr/lib64/mysql -lmysqlclient

telnetProxy.o : telnetProxy.cpp
	g++ -c telnetProxy.cpp -I /usr/include/mysql

typoauth : typoauth.o
	gcc -o typoauth typoauth.o -L /usr/lib64/mysql -lmysqlclient

typoauth.o : typoauth.c
	gcc -c typoauth.c -I /usr/include/mysql

install:
	cp telnetProxy /usr/local/bin
	cp typoauth /usr/local/bin
	chown exauth:exauth /usr/local/bin/typoauth
	chmod a+s /usr/local/bin/typoauth

clean:
	rm *.o *~ telnetProxy typoauth
