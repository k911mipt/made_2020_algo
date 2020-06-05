# CC=g++ -O2 -fno-stack-limit -x c++ -std=c++17
STANDARD=c++17
FLAG_UNKNOWN_PRAGMAS=-Wno-unknown-pragmas
FLAG_ALL_WARNIGNS=-Wall
FLAGS=-std=$(STANDARD) -ggdb3
CC=g++
APP=app
TEST=test

all: clean build_app #run_app

.PHONY: test
test: run_test

run_app:
	./$(APP)

build_app: main.o
	$(CC) $(FLAGS) -o $(APP) main.o 

main.o: main.cpp memcheck_crt.h
	$(CC) $(FLAGS) -c main.cpp

# run_test:
# 	./$(TEST)

# build_test: test.o
# 	$(CC) -o $(TEST) test.o 

# test.o: test.cpp memcheck_crt.h lexer.hpp parser.hpp interpreter.hpp
# 	$(CC) $(FLAGS) -c test.cpp	

clean:
	rm -rf *.o $(APP) $(TEST) valgrind-out.txt

memcheck: clean build_test
	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=valgrind-out.txt \
		./$(TEST)