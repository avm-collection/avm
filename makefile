BIN     = ./bin
OUT     = $(BIN)/app
INSTALL = /usr/bin/avm

SRC  = $(wildcard src/*.c) $(wildcard src/**/*.c)
DEPS = $(wildcard src/*.h) $(wildcard src/**/*.h)
OBJ  = $(addsuffix .o,$(subst src/,$(BIN)/,$(basename $(SRC))))

BIN_DIRS = $(subst src/,$(BIN)/,$(sort $(dir $(wildcard src/*/))))

CC     = gcc
CSTD   = c11
CFLAGS = -O2 -std=$(CSTD) -Wall -Wextra -Werror -pedantic -Wno-deprecated-declarations

ifneq ($(OS),Windows_NT)
	LIBS = -lreadline -ldl
endif

shared: $(BIN_DIRS) $(BIN) $(OBJ) $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)

$(BIN)/%/:
	mkdir -p $@

$(BIN)/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

$(BIN)/:
	mkdir -p $(BIN)/

install:
	cp $(OUT) $(INSTALL)

clean:
	rm -r $(BIN)/*

all:
	@echo shared, static, install, clean
