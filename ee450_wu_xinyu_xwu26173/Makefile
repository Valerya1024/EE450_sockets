all : clean serverS serverD serverU serverM client

client : client.o connection.o util.o
	gcc client.o util.o connection.o -o client

serverS : serverS.o util.o connection.o
	gcc serverS.o util.o connection.o -o serverS

serverD : serverD.o util.o connection.o
	gcc serverD.o util.o connection.o -o serverD

serverU : serverU.o util.o connection.o
	gcc serverU.o util.o connection.o -o serverU

serverM : serverM.o util.o connection.o
	gcc serverM.o util.o connection.o -o serverM

util.o : util.c util.h
	gcc -c -g -Wall util.c -o util.o

connection.o : connection.c connection.h
	gcc -c -g -Wall connection.c -o connection.o

client.o : client.c client.h
	gcc -c -g -Wall client.c -o client.o

serverS.o : serverS.c serverS.h
	gcc -c -g -Wall serverS.c -o serverS.o

serverD.o : serverD.c serverD.h
	gcc -c -g -Wall serverD.c -o serverD.o

serverU.o : serverU.c serverU.h
	gcc -c -g -Wall serverU.c -o serverU.o

serverM.o : serverM.c
	gcc -c -g -Wall serverM.c -o serverM.o

clean :
	rm -f *.o serverS serverD serverU serverM client

backup :
	cp *.c ee450_wu_xinyu_xwu26173/
	cp *.h ee450_wu_xinyu_xwu26173/
	cp member_extra.txt ee450_wu_xinyu_xwu26173/
	cp readme.txt ee450_wu_xinyu_xwu26173/
	cp Makefile ee450_wu_xinyu_xwu26173/
	tar -czvf ee450_wu_xinyu_xwu26173.tar.gz ee450_wu_xinyu_xwu26173/

extra : 
	echo "#define EXTRA" > extra.h
	make all

norm :
	echo "             " > extra.h
	make all