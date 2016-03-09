all: clean main

main: main.o connection.o
	g++ -g -o main main.o connection.o

main.o: udpc.cpp
	g++ -g udpc.cpp -o main.o -c

connection.o: connection.cpp
	g++ -g connection.cpp -o connection.o -c

clean:
	rm -f main
	rm -f main.o
	rm -f connection.o