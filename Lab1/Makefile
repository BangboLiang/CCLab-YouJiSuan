CXXFLAGS+=-O2 -ggdb -DDEBUG
CXXFLAGS+=-Wall -Wextra

all: sudoku_solve

sudoku_solve: main.cc sudoku_min_arity_cache.cc
	g++ -O2 -o $@ $^ -pthread
clean:
	rm -rf sudoku_solve
