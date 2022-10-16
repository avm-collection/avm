SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

OUT = ./bin/app

CC       = gcc
CC_VER   = c99
CC_FLAGS = -O3 -std=$(CC_VER) -Wall -Wextra -Werror \
           -pedantic -Wno-deprecated-declarations
CC_LIBS  =

compile: ./bin $(OBJ) $(SRC)
	$(CC) $(CC_FLAGS) -o $(OUT) $(OBJ) $(CC_LIBS)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CC_FLAGS) -o $@

./bin:
	mkdir -p bin

clean:
	rm -r ./bin/*

all:
	@echo compile, clean
