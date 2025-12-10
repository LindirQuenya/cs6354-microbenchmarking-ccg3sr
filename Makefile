CC := gcc
CFLAGS := -O3 -masm=intel -march=native -Wall -Wextra -Iinclude -fno-ipa-cp -fno-tree-vectorize -g
LDFLAGS := -lpthread -lm
BINDIR := build
SRCS := $(wildcard src/*.c)
HEADERS := $(wildcard include/*.h)

.PHONY: all clean fmt
all: $(BINDIR)/benchmark

$(BINDIR)/benchmark: $(SRCS) $(HEADERS) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR)

fmt:
	clang-format -i include/*.h src/*.c

