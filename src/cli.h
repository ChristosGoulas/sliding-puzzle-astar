#ifndef SLIDING_PUZZLE_CLI_H
#define SLIDING_PUZZLE_CLI_H

#include <stddef.h>
#include <stdint.h>

#include "puzzle.h"

typedef struct {
    size_t size;
    const char *board_input;
    int show_path;
    int verbose;
    int generate;
    size_t generate_moves;
    uint64_t seed;
    int seed_provided;
    char *owned_input;
} CliOptions;

int cli_parse(int argc, char *argv[], CliOptions *options, char *error, size_t error_size);
int cli_load_puzzle(CliOptions *options, Puzzle *puzzle, char *error, size_t error_size);
void cli_print_usage(const char *program);
void cli_destroy(CliOptions *options);

#endif
