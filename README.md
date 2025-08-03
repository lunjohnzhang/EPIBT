# Open Multi-Agent PathFinding System

The code repository of the paper "Enhancing PIBT via Multi-Action Operations"(AAAI-26).

## Tests

The tests are in the [tests](tests) folder. The tests store 5 maps: random (random-32-32-20), city (Paris-1-256), game (brc202d), sortation, warehouse. 

## Building

All you need is [CMake](https://cmake.org/) (≥v3.16).\
The code is written in C++(20).\
To build, use [CMakeLists.txt](CMakeLists.txt)\
There you can set up the project:
- `ENABLE_ROTATE_MODEL` — enabling rotation action model (actions: F, R, C, w), without it, the model of movement without turns is used (actions: E, S, W, N, w).
- `ENABLE_ASSERT` — enabling asserts to ensure the correctness of the project. If you are sure about it, you can disable it to speed up the project.
- `THREADS_NUM_DEFAULT` — the maximum number of threads that a project can use.
- `EPIBT_DEPTH` — the depth of operations of the EPIBT family of algorithms.

Creates the `bin/main` executable file

## Usage



### Example
```sh
build/main -i assets/random-32-32-10-random-1.scen -m assets/random-32-32-10.map -N 400 -v 3
```

## Licence

This software is released under the MIT License, see [LICENSE](LICENCE).

If possible, please consider contacting the author for commercial use. This is not a restriction, I just want to know about the use as an academic researcher.