CC := clang
CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -Werror
FORMAT := clang-format
ANALYZER := clang

TARGET := a-star
SOURCES := src/main.c src/cli.c src/puzzle.c src/astar.c
TEST_BINS := tests/bin/test_puzzle tests/bin/test_astar
TEST_SOURCES := src/puzzle.c src/astar.c
FORMAT_SOURCES := $(sort $(SOURCES) tests/test_puzzle.c tests/test_astar.c src/cli.h src/puzzle.h src/astar.h)
LINT_SOURCES := $(sort $(SOURCES) tests/test_puzzle.c tests/test_astar.c)

.PHONY: all clean test format format-check lint

all: $(TARGET)

$(TARGET): $(SOURCES) src/cli.h src/puzzle.h src/astar.h
	$(CC) $(CFLAGS) -Isrc $(SOURCES) -o $@

tests/bin/test_%: tests/test_%.c $(TEST_SOURCES) src/puzzle.h src/astar.h
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -Isrc $< $(TEST_SOURCES) -o $@

test: $(TEST_BINS)
	./tests/bin/test_puzzle
	./tests/bin/test_astar

format:
	$(FORMAT) -i $(FORMAT_SOURCES)

format-check:
	$(FORMAT) --dry-run --Werror $(FORMAT_SOURCES)

lint:
	$(ANALYZER) --analyze $(CFLAGS) -Isrc $(LINT_SOURCES)

clean:
	rm -f $(TARGET) $(TEST_BINS)