#include <string.h>

#include <stdio.h>

#include "puzzle.h"

static int expect_int(const char *label, int actual, int expected) {
    if (actual == expected) {
        return 1;
    }
    fprintf(stderr, "%s: expected %d, got %d\n", label, expected, actual);
    return 0;
}

int main(void) {
    const int one_move[] = {1, 2, 3, 4, 5, 6, 7, 0, 8};
    const int unsolvable[] = {1, 2, 3, 4, 5, 6, 8, 7, 0};
    const int duplicate[] = {1, 2, 3, 4, 5, 6, 7, 7, 0};
    Puzzle puzzle;
    Puzzle generated_first;
    Puzzle generated_second;
    char error[128];

    if (!puzzle_init(&puzzle, 3, one_move, error, sizeof(error)) ||
        !expect_int("one-move puzzle solvability", puzzle_is_solvable(&puzzle), 1) ||
        !expect_int("one-move Manhattan distance", puzzle_manhattan_distance(&puzzle), 1)) {
        return 1;
    }
    if (!puzzle_init(&puzzle, 3, unsolvable, error, sizeof(error)) ||
        !expect_int("unsolvable puzzle detection", puzzle_is_solvable(&puzzle), 0)) {
        return 1;
    }
    if (!expect_int("duplicate tile rejection",
                    puzzle_init(&puzzle, 3, duplicate, error, sizeof(error)), 0)) {
        return 1;
    }
    if (!expect_int("unsupported generator size rejection",
                    puzzle_generate(&puzzle, 4, 20, 42, error, sizeof(error)), 0)) {
        return 1;
    }
    if (!puzzle_generate(&generated_first, 3, 20, 42, error, sizeof(error)) ||
        !puzzle_generate(&generated_second, 3, 20, 42, error, sizeof(error)) ||
        !expect_int("generated puzzle solvability", puzzle_is_solvable(&generated_first), 1) ||
        !expect_int("deterministic generated puzzle",
                    memcmp(generated_first.tiles, generated_second.tiles, generated_first.cells),
                    0)) {
        return 1;
    }

    printf("test_puzzle: passed\n");
    return 0;
}
