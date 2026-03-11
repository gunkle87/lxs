CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -O2
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXE = net2bench.exe

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(EXE)
	powershell -ExecutionPolicy Bypass -File tests/runner/run_corpus.ps1

clean:
	del /Q /F $(OBJ) $(EXE) 2>NUL || exit 0

.PHONY: all test clean
