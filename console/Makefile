# Herchosting console Makefile

OPS = -g # -g for debug

LIBS = -L ../common -lherchosting -L /usr/lib64/mysql -lmysqlclient -L /usr/local/lib -lwt 
INCLUDES =-I . -I ../common -I /usr/include/mysql -I /usr/local/include -I /usr/local/include/boost-1_39

LIBSSA = -lpthread -L /usr/local/lib -lwt
INCLUDESSA =-I . -I /usr/local/include -I /usr/local/include/boost-1_39

CGILIBS = ${LIBS} -lwtfcgi
HTTPLIBS = ${LIBS} -lwthttp

CGILIBSSA = ${LIBS-SA} -lwtfcgi
HTTPLIBSSA = ${LIBS-SA} -lwthttp

OBJS = console.o introduction.o status.o print.o jobs.o punch.o tape.o account.o terminal.o webservice.o

all : console web3270 console.fcgi web3270.fcgi

console : ${OBJS}
	g++ ${OPS} -o console ${OBJS} ${HTTPLIBS}

console.fcgi : ${OBJS}
	g++ ${OPS} -o console.fcgi ${OBJS} ${CGILIBS}

# Standalone web3270 Target
web3270 : terminal.cpp console.cpp introduction.cpp console.h terminal.h
	g++ ${OPS} -DSTANDALONE -c -o console-sa.o console.cpp ${INCLUDESSA}
	g++ ${OPS} -DSTANDALONE -c -o terminal-sa.o terminal.cpp ${INCLUDESSA}
	g++ ${OPS} -DSTANDALONE -c -o introduction-sa.o introduction.cpp ${INCLUDESSA}
	g++ ${OPS} -o web3270 console-sa.o terminal-sa.o introduction-sa.o ${HTTPLIBSSA}
	g++ ${OPS} -o web3270.fcgi console-sa.o terminal-sa.o introduction-sa.o ${CGILIBSSA}

.cpp.o :
	g++ ${OPS} -c $< ${INCLUDES}

${OBJS} : console.h terminal.h

clean :
	rm *.o *~ console web3270 console.fcgi web3270.fcgi

install:
	/sbin/service httpd stop
	cp console.fcgi /local/www/mvs.open-bpm.org
	chown apache:apache /local/www/mvs.open-bpm.org/console.fcgi
	chmod a+s /local/www/mvs.open-bpm.org/console.fcgi
	cp web3270.fcgi /local/www/mvs.open-bpm.org
	chown apache:apache /local/www/mvs.open-bpm.org/web3270.fcgi
	chmod a+s /local/www/mvs.open-bpm.org/web3270.fcgi
	cp -r images/* /local/www/mvs.open-bpm.org/images
	cp openbpm.css /local/www/mvs.open-bpm.org
	cp -r /usr/local/share/Wt/resources /local/www/mvs.open-bpm.org/resources
	/sbin/service httpd start

devinstall:
	/sbin/service httpd stop
	cp console.fcgi /local/www/mvs.open-bpm.org/devconsole.fcgi
	chown apache:apache /local/www/mvs.open-bpm.org/devconsole.fcgi
	chmod a+s /local/www/mvs.open-bpm.org/devconsole.fcgi
	cp web3270.fcgi /local/www/mvs.open-bpm.org/devweb3270.fcgi
	chown apache:apache /local/www/mvs.open-bpm.org/devweb3270.fcgi
	chmod a+s /local/www/mvs.open-bpm.org/devweb3270.fcgi
	cp -r images/* /local/www/mvs.open-bpm.org/images
	cp openbpm.css /local/www/mvs.open-bpm.org
	cp -r /usr/local/share/Wt/resources /local/www/mvs.open-bpm.org/resources
	/sbin/service httpd start
