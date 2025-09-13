CFLAGS := -O3 -march=native -Wall -Wextra -Iinclude
LDFLAGS := -lpthread
BIN := build
SRCS := $(wildcard src/*.c)

all: $(BIN) $(BIN)/benchmark

$(BIN)/benchmark: $(BIN) $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN)
