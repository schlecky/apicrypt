apicrypt: apicrypt.o apikey.o
		gcc apicrypt.o apikey.o -o apicrypt

apicrypt.o : apicrypt.c
		 gcc -c apicrypt.c -o apicrypt.o


keygen: keygen.o apikey.o
		gcc keygen.o apikey.o -o keygen

keygen.o: keygen.c
		gcc -c keygen.c -o keygen.o


apikey.o : apikey.c apikey.h
		gcc -c apikey.c -o apikey.o

clean :
		rm *.o

test: apicrypt
		./test_prog.sh

