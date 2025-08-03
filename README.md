# Open Multi-Agent PathFinding System

The code repository of the paper "Enhancing PIBT via Multi-Action Operations"(AAAI-26).

## Tests

The tests are in the [tests](tests) folder. The tests store 5 maps: random (random-32-32-20), city (Paris-1-256), game (brc202d), sortation, warehouse. Each map contains the map itself `map.txt`, a list of agents `agents_*.csv`, a list of tasks with a different number of goals `tasks.csv`, and with one goal `tasks_one.csv`.

| Map | Size | \|V\| | Agents num|
|-----|------|---|-----------|
|random-32-32-20|32x32|819|100, 200, ..., 800|
|Paris-1-256|256x256|47240|1000, 2000, ..., 10000|
|brc202d|481x530|43151|500, 1000, ..., 5000|
|sortation|140x500|54320|1000, 2000, ..., 10000|
|warehouse|140x500|38586|1000, 2000, ..., 10000|

<img width="1791" height="1788" alt="image" src="https://github.com/user-attachments/assets/0feb1ce8-eecb-41ec-ad0e-56bec8e906b5" />

## Building

You need [CMake](https://cmake.org/) (≥v3.16), [Boost](https://www.boost.org/) (≥v1.49.0)\
The code is written in C++(20).\
To build, use [CMakeLists.txt](CMakeLists.txt)\
There you can set up the project:
- `ENABLE_ROTATE_MODEL` — enabling rotation action model (actions: F, R, C, w), without it, the model of movement without turns is used (actions: E, S, W, N, w).
- `ENABLE_ASSERT` — enabling asserts to ensure the correctness of the project. If you are sure about it, you can disable it to speed up the project.
- `THREADS_NUM_DEFAULT` — the maximum number of threads that a project can use.
- `EPIBT_DEPTH` — the depth of operations of the EPIBT family of algorithms.

Creates the `bin/main` executable file.

## Usage

### Example
```sh
bin/main example.config
```

In the argument, you pass the path to the configuration file. It has the following settings:
- `map_type` — `RANDOM`, `CITY`, `GAME`, `SORTATION`, `WAREHOUSE`.
- `planner_type` — `PIBT`, `CAUSAL_PIBT`, `PIBT_TF` (Causal PIBT+traffic flow), `EPIBT`, `EPIBT_LNS`, `EPIBT_LNS_OLD` (LoRR24-Winner), `PEPIBT_LNS`, `WPPL` (LoRR23-Winner).
- `scheduler_type` — `CONST` (а constant sequence of tasks for each agent), `GREEDY` (stores a pool of available tasks of size 1.5*number of agents, using a greedy multithreaded algorithm, it gives each agent the nearest available task.).
- `graph_guidance_type` — `ENABLE` (enables Graph Guidance Optimization)
- `map_file` — the path to the file `map.txt` where the map is stored.
- `tasks_path` — the path to the tasks file.
- `agents_path` — the path to the agent file/folder. If it's going to be a folder, then it will read the `agents_*.csv` files and in this case, THREADS_NUM_DEFAULT threads will solve instances with different numbers of agents in parallel.
- `steps_num` — number of steps.
- `output_path` — where to record the logs of the solution. For each test instance with the test_id number, it will create a `test_id/` folder. He will write the files there.
  - `heatmap_*.csv`, where * is all, F, C, R, w (for rotation action model), otherwise all, E, S, W, N, w. Each such file contains a table where the cell indicates the number of events when such an action occurred or -1 if it is an impassable cell of the map. With this, you can see where agents often move, where wait a lot, and even visualize it.
  - `log.csv` — contains information about each step.
  - `metrics.csv` — contains total information about instance. 

## License

This software is released under the MIT License, see [LICENSE](LICENSE).

If possible, please consider contacting the author for commercial use. This is not a restriction, I just want to know about the use as an academic researcher.
