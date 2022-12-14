SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

BIN     = ./bin
OUT     = $(BIN)/app
INSTALL = /usr/bin/avm

CC     = gcc
CSTD   = c11
CFLAGS = -Og -std=$(CSTD) -Wall -Wextra -Werror -pedantic -Wno-deprecated-declarations

shared: $(BIN) $(OBJ) $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)

static: $(BIN) $(OBJ) $(SRC)
	$(CC) -static $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

$(BIN):
	mkdir -p bin

install:
	cp $(OUT) $(INSTALL)

clean:
	rm -r $(BIN)/*

all:
	@echo shared, static, install, clean
