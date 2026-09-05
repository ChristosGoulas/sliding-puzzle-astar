#ifndef SLIDING_PUZZLE_H
#define SLIDING_PUZZLE_H

#include <stddef.h>
#include <stdint.h>

#define PUZZLE_MIN_SIZE 2
#define PUZZLE_MAX_SIZE 3
#define PUZZLE_MAX_CELLS (PUZZLE_MAX_SIZE * PUZZLE_MAX_SIZE)

typedef struct {
    size_t size;
    size_t cells;
    unsigned char tiles[PUZZLE_MAX_CELLS];
} Puzzle;

int puzzle_init(Puzzle *puzzle, size_t size, const int values[], char *error, size_t error_size);
int puzzle_generate(Puzzle *puzzle, size_t size, size_t moves, uint64_t seed, char *error,
                    size_t error_size);
int puzzle_is_solvable(const Puzzle *puzzle);
int puzzle_manhattan_distance(const Puzzle *puzzle);
void puzzle_print(const Puzzle *puzzle);

#endif
