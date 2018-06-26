all: ha11server.o ha11client.o
	gcc -o server ha11server.o -lpthread
	gcc -o client ha11client.o

ha11server.o: ha11server.c
	gcc -c ha11server.c -lpthread

ha11client.o: ha11client.c
	gcc -c ha11client.c

clean:
	rm *.o