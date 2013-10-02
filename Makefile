clean:
	rm -f *.o *.out main

main: main.o manager.o client.o
	gcc -o main -lm -g main.o manager.o client.o

main.o: main.c
	gcc -g -c -Wall main.c

manager.o: manager.c manager.h
	gcc -g -c -Wall manager.c

client.o: client.c client.h
	gcc -g -c -Wall client.c
