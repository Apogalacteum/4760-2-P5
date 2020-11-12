oss: main.o
	cc -o oss main.o
	
user_proc: chproc.o
	cc -o user_proc chproc.o

main.o: main.c
	cc -c main.c
	
chproc.o: chproc.c
	cc -c chproc.c
  
clean :
	rm oss user_proc main.o chproc.o