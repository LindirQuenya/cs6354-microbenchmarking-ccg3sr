CC := gcc
CFLAGS := -O3 -march=native -Wall -Wextra -Iinclude -fno-ipa-cp -fno-tree-vectorize
LDFLAGS := -lpthread -lm
BIN := build
SRCS := $(wildcard src/*.c)

all: $(BIN) $(BIN)/benchmark

$(BIN)/benchmark: $(BIN) $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN)
