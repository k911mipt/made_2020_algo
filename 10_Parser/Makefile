# CC=g++ -O2 -fno-stack-limit -x c++ -std=c++17
STANDARD = c++17
FLAGS = -O2 -std=$(STANDARD)
CC = g++ 
APP = app
TEST = test

all: build_app run_app

.PHONY: test
test: build_test run_test

run_app:
	./$(APP)

build_app: main.o
	$(CC) $(FLAGS) -o $(APP) main.o 

main.o: main.cpp memcheck_crt.h lexer.hpp parser.hpp interpreter.hpp
	$(CC) -c main.cpp

run_test:
	./$(TEST)

build_test: test.o
	$(CC) $(FLAGS) -o $(TEST) test.o 

test.o: test.cpp memcheck_crt.h lexer.hpp parser.hpp interpreter.hpp
	$(CC) -c test.cpp	

clean:
	rm -rf *.o $(APP) $(TEST)