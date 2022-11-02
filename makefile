SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

OUT = ./bin/app

CC     = gcc
STD    = c99
CFLAGS = -O3 -std=$(STD) -Wall -Wextra -Werror \
         -pedantic -Wno-deprecated-declarations
LIBS   =

compile: ./bin $(OBJ) $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

./bin:
	mkdir -p bin

clean:
	rm -r ./bin/*

all:
	@echo compile, clean
