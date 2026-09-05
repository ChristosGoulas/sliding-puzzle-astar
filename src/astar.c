#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astar.h"

struct State {
    unsigned char tiles[PUZZLE_MAX_CELLS];
    int g;
    int h;
    struct State *parent;
    char move;
};

typedef struct {
    State **items;
    size_t count;
    size_t capacity;
} StateList;

typedef StateList MinHeap;

static void set_error(char *error, size_t error_size, const char *format, ...) {
    va_list arguments;

    if (error == NULL || error_size == 0) {
        return;
    }
    va_start(arguments, format);
    vsnprintf(error, error_size, format, arguments);
    va_end(arguments);
}

static FILE *trace_stream(const AStarOptions *options) {
    if (options == NULL || !options->verbose) {
        return NULL;
    }
    return options->trace == NULL ? stderr : options->trace;
}

static void trace_board(FILE *trace, const unsigned char tiles[], const Puzzle *puzzle) {
    size_t row;

    for (row = 0; row < puzzle->size; row++) {
        size_t column;

        fprintf(trace, "[trace]   ");
        for (column = 0; column < puzzle->size; column++) {
            size_t index = row * puzzle->size + column;

            fprintf(trace, "%u%s", tiles[index], column + 1 == puzzle->size ? "\n" : " ");
        }
    }
}

static size_t permutation_count(size_t cells) {
    size_t total = 1;

    while (cells > 1) {
        total *= cells--;
    }
    return total;
}

static size_t rank_tiles(const unsigned char tiles[], size_t cells) {
    size_t rank = 0;
    size_t index;

    for (index = 0; index < cells; index++) {
        size_t smaller = 0;
        size_t next;

        for (next = index + 1; next < cells; next++) {
            if (tiles[next] < tiles[index]) {
                smaller++;
            }
        }
        rank = rank * (cells - index) + smaller;
    }
    return rank;
}

static int state_precedes(const State *left, const State *right) {
    int left_score = left->g + left->h;
    int right_score = right->g + right->h;

    return left_score != right_score ? left_score < right_score : left->h < right->h;
}

static int reserve(StateList *list, size_t required) {
    State **items;
    size_t capacity = list->capacity == 0 ? 64 : list->capacity;

    while (capacity < required) {
        capacity *= 2;
    }
    items = realloc(list->items, capacity * sizeof(*items));
    if (items == NULL) {
        return 0;
    }
    list->items = items;
    list->capacity = capacity;
    return 1;
}

static int heap_push(MinHeap *heap, State *state) {
    size_t index;

    if (heap->count == heap->capacity && !reserve(heap, heap->count + 1)) {
        return 0;
    }
    index = heap->count++;
    while (index > 0) {
        size_t parent = (index - 1) / 2;

        if (state_precedes(heap->items[parent], state)) {
            break;
        }
        heap->items[index] = heap->items[parent];
        index = parent;
    }
    heap->items[index] = state;
    return 1;
}

static State *heap_pop(MinHeap *heap) {
    State *minimum;
    State *tail;
    size_t index = 0;

    if (heap->count == 0) {
        return NULL;
    }
    minimum = heap->items[0];
    tail = heap->items[--heap->count];
    while (index * 2 + 1 < heap->count) {
        size_t child = index * 2 + 1;

        if (child + 1 < heap->count && state_precedes(heap->items[child + 1], heap->items[child])) {
            child++;
        }
        if (state_precedes(tail, heap->items[child])) {
            break;
        }
        heap->items[index] = heap->items[child];
        index = child;
    }
    if (heap->count > 0) {
        heap->items[index] = tail;
    }
    return minimum;
}

static State *new_state(const unsigned char tiles[], const Puzzle *puzzle, int g, State *parent,
                        char move) {
    State *state = malloc(sizeof(*state));
    Puzzle state_puzzle = {.size = puzzle->size, .cells = puzzle->cells};

    if (state == NULL) {
        return NULL;
    }
    memcpy(state->tiles, tiles, puzzle->cells);
    memcpy(state_puzzle.tiles, tiles, puzzle->cells);
    state->g = g;
    state->h = puzzle_manhattan_distance(&state_puzzle);
    state->parent = parent;
    state->move = move;
    return state;
}

int a_star_solve_with_options(const Puzzle *initial, const AStarOptions *options,
                              SearchResult *result, char *error, size_t error_size) {
    size_t total_states = permutation_count(initial->cells);
    int *best_g = malloc(total_states * sizeof(*best_g));
    StateList allocated = {0};
    MinHeap frontier = {0};
    State *start;
    FILE *trace = trace_stream(options);
    size_t index;

    memset(result, 0, sizeof(*result));
    if (best_g == NULL) {
        set_error(error, error_size, "Unable to allocate search bookkeeping.");
        return 0;
    }
    for (index = 0; index < total_states; index++) {
        best_g[index] = INT_MAX;
    }
    start = new_state(initial->tiles, initial, 0, NULL, '\0');
    if (start == NULL || !reserve(&allocated, 1) || !heap_push(&frontier, start)) {
        free(start);
        free(allocated.items);
        free(frontier.items);
        free(best_g);
        set_error(error, error_size, "Unable to allocate the search frontier.");
        return 0;
    }
    allocated.items[allocated.count++] = start;
    best_g[rank_tiles(start->tiles, initial->cells)] = 0;
    result->size = initial->size;
    result->peak_frontier = frontier.count;
    if (trace != NULL) {
        fprintf(trace, "[trace] starting A* search\n");
        trace_board(trace, start->tiles, initial);
    }

    while (frontier.count > 0) {
        State *current = heap_pop(&frontier);
        size_t rank = rank_tiles(current->tiles, initial->cells);
        size_t blank = 0;
        const char moves[] = {'L', 'R', 'U', 'D'};
        size_t direction;

        if (current->g != best_g[rank]) {
            if (trace != NULL) {
                fprintf(trace, "[trace] discard stale state: g=%d, best_g=%d\n", current->g,
                        best_g[rank]);
            }
            continue;
        }
        result->expanded++;
        if (trace != NULL) {
            fprintf(trace, "[trace] expand #%zu: g=%d h=%d f=%d frontier=%zu\n", result->expanded,
                    current->g, current->h, current->g + current->h, frontier.count);
            trace_board(trace, current->tiles, initial);
        }
        if (current->h == 0) {
            result->goal = current;
            result->moves = (size_t)current->g;
            result->states = allocated.items;
            result->state_count = allocated.count;
            if (trace != NULL) {
                fprintf(trace, "[trace] goal found: depth=%d, expanded=%zu\n", current->g,
                        result->expanded);
            }
            free(frontier.items);
            free(best_g);
            return 1;
        }
        while (current->tiles[blank] != 0) {
            blank++;
        }
        for (direction = 0; direction < 4; direction++) {
            int target = (int)blank;
            unsigned char tiles[PUZZLE_MAX_CELLS];
            State *child;
            size_t child_rank;

            if ((direction == 0 && blank % initial->size == 0) ||
                (direction == 1 && blank % initial->size == initial->size - 1) ||
                (direction == 2 && blank < initial->size) ||
                (direction == 3 && blank + initial->size >= initial->cells)) {
                if (trace != NULL) {
                    fprintf(trace, "[trace] skip move %c: illegal\n", moves[direction]);
                }
                continue;
            }
            target += direction == 0   ? -1
                      : direction == 1 ? 1
                      : direction == 2 ? -(int)initial->size
                                       : (int)initial->size;
            memcpy(tiles, current->tiles, initial->cells);
            tiles[blank] = tiles[target];
            tiles[target] = 0;
            child_rank = rank_tiles(tiles, initial->cells);
            if (current->g + 1 >= best_g[child_rank]) {
                if (trace != NULL) {
                    Puzzle candidate = {.size = initial->size, .cells = initial->cells};
                    int candidate_h;

                    memcpy(candidate.tiles, tiles, candidate.cells);
                    candidate_h = puzzle_manhattan_distance(&candidate);
                    fprintf(trace, "[trace] prune move %c: g=%d h=%d f=%d (best g=%d)\n",
                            moves[direction], current->g + 1, candidate_h,
                            current->g + 1 + candidate_h, best_g[child_rank]);
                    trace_board(trace, tiles, initial);
                }
                continue;
            }
            child = new_state(tiles, initial, current->g + 1, current, moves[direction]);
            if (child == NULL || !reserve(&allocated, allocated.count + 1) ||
                !heap_push(&frontier, child)) {
                free(child);
                search_result_destroy(
                    &(SearchResult){.states = allocated.items, .state_count = allocated.count});
                free(frontier.items);
                free(best_g);
                set_error(error, error_size, "Unable to grow the search frontier.");
                return 0;
            }
            allocated.items[allocated.count++] = child;
            best_g[child_rank] = child->g;
            result->generated++;
            if (trace != NULL) {
                fprintf(trace, "[trace] enqueue move %c: g=%d h=%d f=%d frontier=%zu\n",
                        child->move, child->g, child->h, child->g + child->h, frontier.count);
                trace_board(trace, child->tiles, initial);
            }
        }
        if (frontier.count > result->peak_frontier) {
            result->peak_frontier = frontier.count;
        }
    }

    search_result_destroy(
        &(SearchResult){.states = allocated.items, .state_count = allocated.count});
    free(frontier.items);
    free(best_g);
    set_error(error, error_size, "No solution was found.");
    return 0;
}

int a_star_solve(const Puzzle *initial, SearchResult *result, char *error, size_t error_size) {
    return a_star_solve_with_options(initial, NULL, result, error, error_size);
}

void search_result_print_path(const SearchResult *result) {
    const State **path;
    const State *current;
    size_t index;

    if (result == NULL || result->goal == NULL || result->size < PUZZLE_MIN_SIZE ||
        result->size > PUZZLE_MAX_SIZE) {
        fprintf(stderr, "Cannot render an invalid search result.\n");
        return;
    }
    path = malloc((result->moves + 1) * sizeof(*path));

    if (path == NULL) {
        fprintf(stderr, "Unable to allocate the solution path.\n");
        return;
    }
    current = result->goal;
    for (index = result->moves + 1; index > 0; index--) {
        if (current == NULL) {
            fprintf(stderr, "Cannot render an inconsistent search result.\n");
            free(path);
            return;
        }
        path[--index] = current;
        current = current->parent;
    }
    if (current != NULL) {
        fprintf(stderr, "Cannot render an inconsistent search result.\n");
        free(path);
        return;
    }
    for (index = 0; index <= result->moves; index++) {
        Puzzle puzzle = {.size = result->size, .cells = result->size * result->size};

        memcpy(puzzle.tiles, path[index]->tiles, puzzle.cells);
        printf("\nStep %zu", index);
        if (index > 0) {
            printf(" (move %c)", path[index]->move);
        }
        printf(":\n");
        puzzle_print(&puzzle);
    }
    free(path);
}

void search_result_destroy(SearchResult *result) {
    size_t index;

    for (index = 0; index < result->state_count; index++) {
        free(result->states[index]);
    }
    free(result->states);
    memset(result, 0, sizeof(*result));
}
