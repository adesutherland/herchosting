# printHandler Makefile

all : printHandler

printHandler : printHandler.o
	g++ -o printHandler printHandler.o -L /usr/lib/mysql -lmysqlclient

printHandler.o : printHandler.cpp
	g++ -c printHandler.cpp  -I /usr/include/mysql

install:
	cp printHandler /usr/local/bin
	chown hercules:hercules /usr/local/bin/printHandler
	chmod a+s /usr/local/bin/printHandler
