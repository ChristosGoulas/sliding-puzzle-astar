#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "puzzle.h"

static void set_error(char *error, size_t error_size, const char *format, ...) {
    va_list arguments;

    if (error == NULL || error_size == 0) {
        return;
    }
    va_start(arguments, format);
    vsnprintf(error, error_size, format, arguments);
    va_end(arguments);
}

int puzzle_init(Puzzle *puzzle, size_t size, const int values[], char *error, size_t error_size) {
    size_t index;
    unsigned int seen = 0;

    if (size < PUZZLE_MIN_SIZE || size > PUZZLE_MAX_SIZE) {
        set_error(error, error_size, "Supported board sizes are 2 and 3.");
        return 0;
    }
    puzzle->size = size;
    puzzle->cells = size * size;
    for (index = 0; index < puzzle->cells; index++) {
        int value = values[index];
        unsigned int bit;

        if (value < 0 || value >= (int)puzzle->cells) {
            set_error(error, error_size, "Tile %d is invalid; expected values from 0 to %zu.",
                      value, puzzle->cells - 1);
            return 0;
        }
        bit = 1U << value;
        if ((seen & bit) != 0) {
            set_error(error, error_size, "Tile %d occurs more than once.", value);
            return 0;
        }
        seen |= bit;
        puzzle->tiles[index] = (unsigned char)value;
    }
    return 1;
}

static uint64_t next_random(uint64_t *state) {
    uint64_t value = *state;

    if (value == 0) {
        value = UINT64_C(0x9e3779b97f4a7c15);
    }
    value ^= value << 13;
    value ^= value >> 7;
    value ^= value << 17;
    *state = value;
    return value;
}

int puzzle_generate(Puzzle *puzzle, size_t size, size_t moves, uint64_t seed, char *error,
                    size_t error_size) {
    int goal[PUZZLE_MAX_CELLS];
    size_t blank;
    size_t previous_blank = PUZZLE_MAX_CELLS;
    size_t move;
    size_t index;

    if (size < PUZZLE_MIN_SIZE || size > PUZZLE_MAX_SIZE) {
        set_error(error, error_size, "Supported board sizes are 2 and 3.");
        return 0;
    }
    for (index = 0; index < size * size; index++) {
        goal[index] = index + 1 == size * size ? 0 : (int)index + 1;
    }
    if (!puzzle_init(puzzle, size, goal, error, error_size)) {
        return 0;
    }
    blank = puzzle->cells - 1;
    for (move = 0; move < moves; move++) {
        size_t candidates[4];
        size_t candidate_count = 0;
        const int offsets[] = {-1, 1, -(int)size, (int)size};
        size_t direction;
        size_t target;
        unsigned char tile;

        for (direction = 0; direction < 4; direction++) {
            int candidate = (int)blank + offsets[direction];

            if ((direction == 0 && blank % size == 0) ||
                (direction == 1 && blank % size == size - 1) || (direction == 2 && blank < size) ||
                (direction == 3 && blank + size >= puzzle->cells) ||
                (size_t)candidate == previous_blank) {
                continue;
            }
            candidates[candidate_count++] = (size_t)candidate;
        }
        if (candidate_count == 0) {
            for (direction = 0; direction < 4; direction++) {
                int candidate = (int)blank + offsets[direction];

                if (!((direction == 0 && blank % size == 0) ||
                      (direction == 1 && blank % size == size - 1) ||
                      (direction == 2 && blank < size) ||
                      (direction == 3 && blank + size >= puzzle->cells))) {
                    candidates[candidate_count++] = (size_t)candidate;
                }
            }
        }
        target = candidates[next_random(&seed) % candidate_count];
        tile = puzzle->tiles[target];
        puzzle->tiles[target] = 0;
        puzzle->tiles[blank] = tile;
        previous_blank = blank;
        blank = target;
    }
    return 1;
}

int puzzle_is_solvable(const Puzzle *puzzle) {
    size_t inversions = 0;
    size_t blank_row_from_bottom = 0;
    size_t index;

    for (index = 0; index < puzzle->cells; index++) {
        size_t next;

        if (puzzle->tiles[index] == 0) {
            blank_row_from_bottom = puzzle->size - index / puzzle->size;
            continue;
        }
        for (next = index + 1; next < puzzle->cells; next++) {
            if (puzzle->tiles[next] != 0 && puzzle->tiles[index] > puzzle->tiles[next]) {
                inversions++;
            }
        }
    }
    return puzzle->size % 2 != 0 ? inversions % 2 == 0
                                 : (inversions + blank_row_from_bottom) % 2 != 0;
}

int puzzle_manhattan_distance(const Puzzle *puzzle) {
    size_t index;
    int distance = 0;

    for (index = 0; index < puzzle->cells; index++) {
        unsigned char tile = puzzle->tiles[index];
        size_t goal_index;

        if (tile == 0) {
            continue;
        }
        goal_index = tile - 1;
        distance += abs((int)(index / puzzle->size) - (int)(goal_index / puzzle->size));
        distance += abs((int)(index % puzzle->size) - (int)(goal_index % puzzle->size));
    }
    return distance;
}

void puzzle_print(const Puzzle *puzzle) {
    size_t index;

    for (index = 0; index < puzzle->cells; index++) {
        printf("%u%s", puzzle->tiles[index], (index + 1) % puzzle->size == 0 ? "\n" : " ");
    }
}
