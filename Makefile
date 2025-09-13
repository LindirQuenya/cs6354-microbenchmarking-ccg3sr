CFLAGS := -O3 -march=native -Wall -Wextra -Iinclude
LDFLAGS := -lpthread
BIN := build
SRCS := $(wildcard src/*.c)
BINS := $(patsubst src/%.c,$(BIN)/%,$(SRCS))

all: $(BIN) $(BINS)

$(BIN)/%: src/%.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN)
