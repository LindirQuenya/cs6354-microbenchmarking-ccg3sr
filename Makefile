CC := gcc
CFLAGS := -O3 -masm=intel -march=native -Wall -Wextra -Iinclude -fno-ipa-cp -fno-tree-vectorize -g
LDFLAGS := -lpthread -lm
BIN := build
SRCS := $(wildcard src/*.c)

.PHONY: all clean fmt
all: $(BIN) $(BIN)/benchmark

$(BIN)/benchmark: $(BIN) $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN)

fmt:
	clang-format -i include/*.h src/*.c
