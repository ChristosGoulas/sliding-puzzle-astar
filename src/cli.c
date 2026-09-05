#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cli.h"
#include "puzzle.h"

void cli_print_usage(const char *program) {
    printf("Usage: %s --size {2|3} (--board \"tiles\" | --file path | --generate moves) [options]\n"
           "\nUse 0 for the blank tile. Example:\n"
           "  %s --size 3 --generate 20 --seed 42 --show-path --verbose\n",
           program, program);
}

static char *read_file(const char *path, char *error, size_t error_size) {
    FILE *file = fopen(path, "rb");
    long length;
    char *contents;

    if (file == NULL) {
        snprintf(error, error_size, "Cannot open '%s'.", path);
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        snprintf(error, error_size, "Cannot read '%s'.", path);
        return NULL;
    }
    contents = malloc((size_t)length + 1);
    if (contents == NULL || fread(contents, 1, (size_t)length, file) != (size_t)length) {
        free(contents);
        fclose(file);
        snprintf(error, error_size, "Cannot read '%s'.", path);
        return NULL;
    }
    contents[length] = '\0';
    fclose(file);
    return contents;
}

static int parse_tiles(const char *input, int values[], size_t expected, char *error,
                       size_t error_size) {
    const char *cursor = input;
    size_t count = 0;

    while (*cursor != '\0') {
        char *end;
        long value;

        while (isspace((unsigned char)*cursor) || *cursor == ',') {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }
        if (count == expected) {
            snprintf(error, error_size, "Expected exactly %zu tiles.", expected);
            return 0;
        }
        errno = 0;
        value = strtol(cursor, &end, 10);
        if (cursor == end || errno == ERANGE || value < INT_MIN || value > INT_MAX) {
            snprintf(error, error_size, "Invalid tile near '%s'.", cursor);
            return 0;
        }
        values[count++] = (int)value;
        cursor = end;
    }
    if (count != expected) {
        snprintf(error, error_size, "Expected %zu tiles, received %zu.", expected, count);
        return 0;
    }
    return 1;
}

static int parse_size(const char *input, size_t *value) {
    char *end;
    unsigned long long parsed;

    errno = 0;
    parsed = strtoull(input, &end, 10);
    if (*input == '\0' || *end != '\0' || errno == ERANGE ||
        parsed > (unsigned long long)SIZE_MAX) {
        return 0;
    }
    *value = (size_t)parsed;
    return 1;
}

int cli_parse(int argc, char *argv[], CliOptions *options, char *error, size_t error_size) {
    const char *file_path = NULL;
    int index;

    memset(options, 0, sizeof(*options));
    for (index = 1; index < argc; index++) {
        if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0) {
            return 0;
        }
        if (strcmp(argv[index], "--size") == 0 && index + 1 < argc) {
            if (!parse_size(argv[++index], &options->size)) {
                snprintf(error, error_size, "Invalid board size.");
                return -1;
            }
        } else if (strcmp(argv[index], "--board") == 0 && index + 1 < argc) {
            options->board_input = argv[++index];
        } else if (strcmp(argv[index], "--file") == 0 && index + 1 < argc) {
            file_path = argv[++index];
        } else if (strcmp(argv[index], "--show-path") == 0) {
            options->show_path = 1;
        } else if (strcmp(argv[index], "--verbose") == 0) {
            options->verbose = 1;
        } else if (strcmp(argv[index], "--generate") == 0 && index + 1 < argc) {
            if (!parse_size(argv[++index], &options->generate_moves)) {
                snprintf(error, error_size, "Invalid generation move count.");
                return -1;
            }
            options->generate = 1;
        } else if (strcmp(argv[index], "--seed") == 0 && index + 1 < argc) {
            size_t parsed_seed;

            if (!parse_size(argv[++index], &parsed_seed)) {
                snprintf(error, error_size, "Invalid random seed.");
                return -1;
            }
            options->seed = (uint64_t)parsed_seed;
            options->seed_provided = 1;
        } else {
            snprintf(error, error_size, "Unknown or incomplete option: %s", argv[index]);
            return -1;
        }
    }
    if (options->size < PUZZLE_MIN_SIZE || options->size > PUZZLE_MAX_SIZE) {
        snprintf(error, error_size, "Supported board sizes are 2 and 3.");
        return -1;
    }
    if ((options->board_input != NULL) + (file_path != NULL) + options->generate != 1) {
        snprintf(error, error_size, "Specify exactly one of --board, --file, or --generate.");
        return -1;
    }
    if (options->seed_provided && !options->generate) {
        snprintf(error, error_size, "--seed requires --generate.");
        return -1;
    }
    if (file_path != NULL) {
        options->owned_input = read_file(file_path, error, error_size);
        if (options->owned_input == NULL) {
            return -1;
        }
        options->board_input = options->owned_input;
    }
    return 1;
}

int cli_load_puzzle(CliOptions *options, Puzzle *puzzle, char *error, size_t error_size) {
    int values[PUZZLE_MAX_CELLS];

    if (options->generate) {
        if (!options->seed_provided) {
            options->seed = (uint64_t)time(NULL);
        }

        return puzzle_generate(puzzle, options->size, options->generate_moves, options->seed, error,
                               error_size);
    }
    return parse_tiles(options->board_input, values, options->size * options->size, error,
                       error_size) &&
           puzzle_init(puzzle, options->size, values, error, error_size);
}

void cli_destroy(CliOptions *options) {
    free(options->owned_input);
    memset(options, 0, sizeof(*options));
}
