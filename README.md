# Open Multi-Agent PathFinding System

The code repository of the paper "Enhancing PIBT via Multi-Action Operations"(AAAI-26).

## Tests

The tests are in the [tests](tests) folder. The tests store 5 maps: `random` (random-32-32-20), `city` (Paris-1-256), `game` (brc202d), `sortation`, `warehouse`. Each map contains the map itself `map.txt`, a list of agents `agents_*.csv`, a list of tasks with a different number of goals `tasks.csv`, and with one goal `tasks_one.csv`.

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
- `ENABLE_ASSERT` — enabling asserts to ensure the correctness of the project. If you are sure about it, you can disable it to speed up the project by several times.
- `ENABLE_ROTATE_MODEL` — enabling rotation action model (actions: F, R, C, w), without it, the model of movement without turns is used (actions: E, S, W, N, w).
- `THREADS_NUM` — the maximum number of threads that a project can use.
- `EPIBT_DEPTH` — the depth of operations of the EPIBT-like algorithms.
- `ENABLE_EPIBT_IO` — enabling inheritance of operations in the EPIBT-like algorithms.
- `EPIBT_REVISIT` — the maximum number of visits for each agent in the EPIBT-like algorithms.

Creates the `bin/main` executable file.

## Usage

### Example
```sh
./bin/main example.config
```

### Configuration
In the argument, you pass the path to the configuration file. It has the following settings:
- `map_type` — `RANDOM`, `CITY`, `GAME`, `SORTATION`, `WAREHOUSE`.
- `planner_type` — `PIBT`, `CAUSAL_PIBT`, `PIBT_TF` (Causal PIBT+traffic flow), `EPIBT`, `EPIBT_LNS`, `PEPIBT_LNS` (parallel `EPIBT_LNS`).
- `scheduler_type` — `CONST` (а constant sequence of tasks for each agent), `GREEDY` (stores a pool of available tasks of size 1.5*number of agents, using a greedy algorithm, it gives each agent the nearest available task.).
- `graph_guidance_type` — `ENABLE` (enables Graph Guidance Optimization)
- `map_file` — the path to the file `map.txt` where the map is stored.
- `tasks_path` — the path to the tasks file.
- `agents_path` — the path to the agent file/folder. If it's going to be a folder, then it will read the `agents_*.csv` files and in this case, `THREADS_NUM_DEFAULT` threads will solve instances with different numbers of agents in parallel.
- `steps_num` — number of steps.
- `step_time` — time limit in ms for the path planner. It may exceed this value, but not much.
- `output_path` — where to record the logs of the solution. For each test instance with the test_id number, it will create a `test_id/` folder. He will write the files there.
  - `heatmap_*.csv`, where * is all, F, C, R, w (for rotation action model), otherwise all, E, S, W, N, w. Each such file contains a table where the cell indicates the number of events when such an action occurred or -1 if it is an impassable cell of the map. With this, you can see where agents often move, where wait a lot, and even visualize it.
  - `log.csv` — contains information about each step.
  - `metrics.csv` — contains total information about instance.
    |metric|value|
    |-|-|
    |task type|LMAPF-T|
    |map type|RANDOM|
    |test id|0|
    |scheduler type|CONST|
    |planner type|EPIBT(3)|
    |graph guidance type|disable|
    |agents num|100|
    |steps num|1000|
    |finished tasks|2788|
    |throughput|2.788|
    |F|71.686|
    |R|14.751|
    |C|10.386|
    |w|3.177|
    |avg step time (ms)|0.188965|
    |avg scheduler time (ms)|0.000979093|
    |avg planner time (ms)|0.158248|

### Scripts

- `src/scripts/tester.py` — a script to run bin/main on all tests and write results to the appropriate directories.
- `src/scripts/build_metrics.py` — a script for collecting information from `metrics.csv` files inside a given folder and creating a table.
- `src/scripts/build_metrics_plot.py` — a script for visualization a tables of metrics with different algorithms.
  <img width="3668" height="1506" alt="image" src="https://github.com/user-attachments/assets/3cee76a9-d24b-4583-b628-47bb4f924233" />
- `src/scripts/build_heatmap.py` — a script for visualizing a heat map of agents actions.
  <img width="2103" height="1865" alt="image" src="https://github.com/user-attachments/assets/1387fd2e-92a7-42a1-a1c9-4998d9364913" />
- `src/scripts/build_spider_plot.py` — a script for visualization a spider plot of algorithms.
  <img width="2024" height="1833" alt="image" src="https://github.com/user-attachments/assets/b7bac381-1bdf-40c9-bf27-9759a7c38eda" />

## License

This software is released under the MIT License, see [LICENSE](LICENSE).

If possible, please consider contacting the author for commercial use. This is not a restriction, I just want to know about the use as an academic researcher.
