#ifndef SLIDING_PUZZLE_ASTAR_H
#define SLIDING_PUZZLE_ASTAR_H

#include <stddef.h>
#include <stdio.h>

#include "puzzle.h"

typedef struct State State;

typedef struct {
    const State *goal;
    size_t size;
    size_t moves;
    size_t expanded;
    size_t generated;
    size_t peak_frontier;
    State **states;
    size_t state_count;
} SearchResult;

typedef struct {
    int verbose;
    FILE *trace;
} AStarOptions;

int a_star_solve(const Puzzle *initial, SearchResult *result, char *error, size_t error_size);
int a_star_solve_with_options(const Puzzle *initial, const AStarOptions *options,
                              SearchResult *result, char *error, size_t error_size);
void search_result_print_path(const SearchResult *result);
void search_result_destroy(SearchResult *result);

#endif
