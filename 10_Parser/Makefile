# CC=g++ -O2 -fno-stack-limit -x c++ -std=c++17
STANDARD = c++17
FLAGS = -O2 -std=$(STANDARD)
CC = g++ 
APP = app

all: build_app run_app

run_app:
	./$(APP)

build_app: main
	$(CC) $(FLAGS) -o $(APP) main.o 

main.o: main.cpp memcheck_crt.h lexer.hpp parser.hpp interpreter.hpp
	$(CC) -c main.cpp

clean:
	rm -rf *.o $(APP)