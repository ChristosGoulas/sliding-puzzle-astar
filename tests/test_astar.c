#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "astar.h"

static int solve_with_expected_moves(const int tiles[], size_t size, size_t expected_moves) {
    Puzzle puzzle;
    SearchResult result;
    char error[128];
    int passed = 0;

    if (!puzzle_init(&puzzle, size, tiles, error, sizeof(error))) {
        fprintf(stderr, "Unable to create test puzzle: %s\n", error);
        return 0;
    }
    if (!a_star_solve(&puzzle, &result, error, sizeof(error))) {
        fprintf(stderr, "Solver failed: %s\n", error);
        return 0;
    }
    if (result.moves != expected_moves) {
        fprintf(stderr, "Expected %zu moves, got %zu.\n", expected_moves, result.moves);
    } else {
        passed = 1;
    }
    search_result_destroy(&result);
    return passed;
}

static int verify_verbose_trace(void) {
    const int one_move[] = {1, 2, 3, 4, 5, 6, 7, 0, 8};
    AStarOptions options;
    Puzzle puzzle;
    SearchResult result;
    char error[128];
    char trace[2048];
    FILE *stream = tmpfile();
    size_t bytes_read;
    int read_error;
    int passed;

    if (stream == NULL) {
        fprintf(stderr, "Unable to create a temporary trace file.\n");
        return 0;
    }
    if (!puzzle_init(&puzzle, 3, one_move, error, sizeof(error))) {
        fclose(stream);
        return 0;
    }
    options.verbose = 1;
    options.trace = stream;
    if (!a_star_solve_with_options(&puzzle, &options, &result, error, sizeof(error))) {
        fprintf(stderr, "Verbose solver failed: %s\n", error);
        fclose(stream);
        return 0;
    }
    if (fseek(stream, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Unable to rewind verbose trace output: %s\n", strerror(errno));
        search_result_destroy(&result);
        fclose(stream);
        return 0;
    }
    bytes_read = fread(trace, 1, sizeof(trace) - 1, stream);
    read_error = errno;
    if (bytes_read != sizeof(trace) - 1 && ferror(stream)) {
        fprintf(stderr, "Unable to read verbose trace output: %s\n", strerror(read_error));
        search_result_destroy(&result);
        fclose(stream);
        return 0;
    }
    trace[bytes_read] = '\0';
    passed =
        strstr(trace, "[trace] expand #1") != NULL && strstr(trace, "[trace] goal found") != NULL;
    if (!passed) {
        fprintf(stderr, "Verbose trace did not contain required events.\n");
    }
    search_result_destroy(&result);
    fclose(stream);
    return passed;
}

int main(void) {
    const int solved_2x2[] = {1, 2, 3, 0};
    const int one_move_2x2[] = {1, 2, 0, 3};
    const int one_move_3x3[] = {1, 2, 3, 4, 5, 6, 7, 0, 8};
    const int known_20_move_3x3[] = {3, 7, 5, 1, 4, 8, 0, 6, 2};

    if (!solve_with_expected_moves(solved_2x2, 2, 0) ||
        !solve_with_expected_moves(one_move_2x2, 2, 1) ||
        !solve_with_expected_moves(one_move_3x3, 3, 1) ||
        !solve_with_expected_moves(known_20_move_3x3, 3, 20) || !verify_verbose_trace()) {
        return 1;
    }

    printf("test_astar: passed\n");
    return 0;
}
