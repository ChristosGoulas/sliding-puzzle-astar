[![CI](https://github.com/ChristosGoulas/sliding-puzzle-astar/actions/workflows/ci.yml/badge.svg)](https://github.com/ChristosGoulas/sliding-puzzle-astar/actions/workflows/ci.yml)

# Sliding Puzzle A* Solver

A command-line solver for $2 \times 2$ and $3 \times 3$ sliding-tile puzzles,
implemented in C17 with the A* search algorithm and Manhattan-distance heuristic.

The solver accepts a puzzle from the command line or a text file, validates the
input, rejects unsolvable boards before search, and reports search metrics.

## Highlights

- Optimal A* search using the admissible Manhattan-distance heuristic.
- Reproducible puzzle generation with `--generate` and `--seed`.
- Solvability checks and strict input validation before search begins.
- Verbose trace mode that exposes expansion, pruning, and frontier decisions.
- Native regression tests, Clang static analysis, and GitHub Actions CI on macOS
	and Ubuntu.

## Example

```console
$ ./a-star --size 2 --generate 8 --seed 42
A* sliding puzzle solver (2x2)
Generated with 8 random moves (seed=42).
Initial board:
3 1
2 0
Solved in 4 moves.
Expanded: 5 | Generated: 5 | Peak frontier: 2 | Time: 0.000 s
```

## Requirements

- A C17 compiler. The project defaults to Clang.
- `make`
- `clang-format` for `make format` and `make format-check`

## Build

```sh
make
```

This creates the `a-star` executable in the repository root.

## Usage

Use `0` for the blank tile. Tile values must be unique and cover every value from
`0` through $n^2 - 1$.

```sh
./a-star --size 3 --board "3 7 5 1 4 8 0 6 2"
./a-star --size 3 --board "3,7,5,1,4,8,0,6,2" --show-path
./a-star --size 2 --file examples/2x2-one-move.txt
./a-star --size 3 --generate 20 --seed 42 --show-path
./a-star --size 3 --file examples/3x3-example.txt --verbose 2> trace.log
./a-star --help
```

Input files contain whitespace- or comma-separated tile values. Newlines may be
used to represent board rows:

```text
3 7 5
1 4 8
0 6 2
```

`--show-path` prints every state in the optimal solution. Without it, the solver
prints the move count and search statistics only.

`--generate MOVES` creates a solvable puzzle by applying random legal moves to the
solved board. Add `--seed NUMBER` to generate the same board again, which is useful
for demonstrations and bug reports. A generated puzzle reports its seed before it
is solved.

`--verbose` traces the A* process to standard error: each expanded state, legal
and illegal moves, enqueued successors, pruned duplicates, and goal discovery.
This keeps standard output suitable for summaries while allowing the full trace to
be captured separately with `2> trace.log`.

## Supported Puzzles

The project deliberately supports only $2 \times 2$ and $3 \times 3$ puzzles.
The current in-memory A* implementation is reliable and appropriate for this
state-space size. Larger puzzles need a different search strategy, such as IDA*,
to provide practical memory use.

For supported board sizes, Manhattan distance is admissible, so A* returns a
minimum-move solution.

## Implementation

The solver separates command-line parsing, puzzle-domain logic, and A* search into
small C modules. Search states retain parent links for solution reconstruction,
while permutation ranking tracks the best known cost for each board arrangement
and prevents redundant expansion.

## Development

Run the regression suite:

```sh
make test
```

The tests cover puzzle validation, solvability, Manhattan distance, and known
optimal A* solution lengths, including the bundled 20-move $3 \times 3$ example.

Format sources and verify formatting without modifying files:

```sh
make format
make format-check
```

Run Clang static analysis:

```sh
make lint
```

Remove generated binaries with:

```sh
make clean
```

## Project Layout

```text
src/
	main.c       Program orchestration
	cli.c        Command-line and input-file handling
	puzzle.c     Board validation, solvability, and heuristic logic
	astar.c      A* search and solution reconstruction
tests/         Native C regression tests
examples/      Sample puzzle inputs
```

## License

Distributed under the [MIT License](LICENSE).
