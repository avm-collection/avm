SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

BIN     = ./bin
OUT     = $(BIN)/app
INSTALL = /usr/bin/avm

CC     = gcc
CSTD   = c11
CFLAGS = -static -Og -std=$(CSTD) -Wall -Wextra -Werror -pedantic -Wno-deprecated-declarations

$(OUT): $(BIN) $(OBJ) $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

$(BIN):
	mkdir -p bin

install: $(OUT)
	cp $(OUT) $(INSTALL)

clean:
	rm -r $(BIN)/*

all:
	@echo compile, install, clean
