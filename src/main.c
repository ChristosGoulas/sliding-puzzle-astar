#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "astar.h"
#include "cli.h"
#include "puzzle.h"

int main(int argc, char *argv[]) {
    CliOptions options;
    Puzzle puzzle;
    SearchResult result;
    char error[256];
    clock_t started;
    double elapsed;
    int parse_status;
    int show_path;
    int verbose;
    int generated;
    size_t generate_moves;
    uint64_t seed;
    AStarOptions solver_options;

    parse_status = cli_parse(argc, argv, &options, error, sizeof(error));
    if (parse_status == 0) {
        cli_print_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    if (parse_status < 0) {
        fprintf(stderr, "Error: %s\n", error);
        cli_print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (!cli_load_puzzle(&options, &puzzle, error, sizeof(error))) {
        fprintf(stderr, "Error: %s\n", error);
        cli_destroy(&options);
        return EXIT_FAILURE;
    }
    show_path = options.show_path;
    verbose = options.verbose;
    generated = options.generate;
    generate_moves = options.generate_moves;
    seed = options.seed;
    cli_destroy(&options);
    if (!puzzle_is_solvable(&puzzle)) {
        fprintf(stderr, "Error: this puzzle configuration is unsolvable.\n");
        return EXIT_FAILURE;
    }

    printf("A* sliding puzzle solver (%zux%zu)\n", puzzle.size, puzzle.size);
    if (generated) {
        printf("Generated with %zu random moves (seed=%llu).\n", generate_moves,
               (unsigned long long)seed);
    }
    printf("Initial board:\n");
    puzzle_print(&puzzle);
    started = clock();
    solver_options.verbose = verbose;
    solver_options.trace = stderr;
    if (!a_star_solve_with_options(&puzzle, &solver_options, &result, error, sizeof(error))) {
        fprintf(stderr, "Error: %s\n", error);
        return EXIT_FAILURE;
    }
    elapsed = (double)(clock() - started) / CLOCKS_PER_SEC;
    printf("Solved in %zu moves.\n", result.moves);
    printf("Expanded: %zu | Generated: %zu | Peak frontier: %zu | Time: %.3f s\n", result.expanded,
           result.generated, result.peak_frontier, elapsed);
    if (show_path) {
        search_result_print_path(&result);
    }
    search_result_destroy(&result);
    return EXIT_SUCCESS;
}
