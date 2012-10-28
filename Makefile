all: compile execute

compile: compile.cpp
	g++ $(CFLAGS) -ljsoncpp compile.cpp -o compile
execute: execute.c
	gcc $(CFLAGS) execute.c -o execute

clean:
	rm -f compile execute
