all: compile execute

compile: compile.cpp
	g++ $(CFLAGS) -ljsoncpp -o compile
execute: execute.cpp
	g++ $(CFLAGS) -o execute

clean:
	rm -f compile execute
