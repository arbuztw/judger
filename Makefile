all: compile execute compare

compile: compile.cpp judger.h
	g++ $(CFLAGS) -ljsoncpp compile.cpp -o compile
execute: execute.c judger.h syscall.h
	gcc $(CFLAGS) execute.c -o execute
compare: compare.c
	gcc $(CFLAGS) compare.c -o compare
	

clean:
	rm -f compile execute
