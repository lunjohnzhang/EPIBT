import os

# Add the parent directory to the system path
_current_dir = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.dirname(_current_dir)
import sys

sys.path.append(WORKSPACE_DIR)

# Then add the build directory to the system path
from py_src import LOG_DIR, BUILD_DIR, RUN_DIR, BIN_DIR

sys.path.append(BUILD_DIR)
import gin
import copy
import json
import time
import fire
import pathlib
import numpy as np
import shutil
import multiprocessing
import dataclasses
import hashlib
import subprocess

from itertools import repeat
from py_src.config import WarehouseConfig
from logdir import LogDir
from typing import List


def get_epibt_executable(config: WarehouseConfig) -> str:
    window = config.pibt_window_size
    rotation_flag = "_R" if config.rotation else ""
    exe_path = pathlib.Path(BIN_DIR)
    exe_name = exe_path / f"EPIBT{rotation_flag}_w={window}"
    # Assert the file exists
    if not exe_name.is_file():
        raise FileNotFoundError(f"EPIBT executable not found at {exe_name}")
    return exe_name.absolute().as_posix()


def write_kwargs_to_tmp_file(lines: List[str]) -> str:
    os.makedirs(RUN_DIR, exist_ok=True)
    hash_obj = hashlib.sha256()
    time_str = time.strftime("%y%m%d_%H%M")
    raw_name = time_str.encode() + os.urandom(16)
    hash_obj.update(raw_name)
    file_name = hash_obj.hexdigest()
    file_path = os.path.join(RUN_DIR, file_name)
    with open(file_path, 'w') as f:
        f.write("\n".join(lines))
    return file_path


def write_config_file(
    config: WarehouseConfig,
    map_filepath: str,
    seed: int,
    output_dir: str,
) -> str:
    """Write an OMAPFS-style config file."""
    lines = [
        f"map_type = {config.epibt_map_type}",
        f"planner_type = {config.epibt_planner_type}",
        f"scheduler_type = {config.epibt_scheduler_type}",
        f"graph_guidance_type = {config.epibt_gg_type}",
        "",
        f"map_file = {map_filepath}",
        # f"tasks_path = {tasks_path}",
        # f"agents_path = {agents_path}",
        f"output_path = {output_dir}",
        "",
        f"steps_num = {config.simulation_time}",
        f"step_time = 1000",
        f"n_robots = {config.num_agents}",
        f"n_tasks = {config.num_tasks}",
        f"seed = {seed}",
    ]

    return write_kwargs_to_tmp_file(lines)


def single_simulation(
    warehouse_config: WarehouseConfig,
    map_filepath: str,
    seed: int,
    results_dir: str,
):
    num_agents = warehouse_config.num_agents
    output_dir = os.path.join(results_dir,
                              f"sim-num_agent={num_agents}-seed={seed}")

    # Write warehouse config and map to a temporary file compatible with EPIBT
    config_filepath = write_config_file(
        warehouse_config,
        map_filepath,
        seed,
        output_dir,
    )

    # Run EPIBT executable. result.json will be written to output_dir
    epibt_exe = get_epibt_executable(warehouse_config)
    cmd = [epibt_exe, config_filepath]
    subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Write config to logdir
    # Replace callables with names
    warehouse_config_ = copy.deepcopy(warehouse_config)
    warehouse_config_.iter_update_model_type = getattr(
        warehouse_config_.iter_update_model_type, '__name__', "null")
    with open(os.path.join(output_dir, "config.json"), "w") as f:
        f.write(json.dumps(dataclasses.asdict(warehouse_config_), indent=4))

    # Delete the tmp EPIBT config file
    os.remove(config_filepath)


def main(
    warehouse_config,
    map_filepath,
    num_agent=10,
    num_agent_step_size=1,
    seed=0,
    n_evals=10,
    n_sim=2,  # Run `inc_agents` `n_sim`` times
    mode="constant",
    n_workers=32,
    reload=None,
):
    """
    For testing purposes. Graph a map and run one simulation.

    Args:
        mode: "constant", "inc_agents", or "inc_timesteps".
              "constant" will run `n_eval` simulations with the same
              `num_agent`.
              "increase" will run `n_eval` simulations with an inc_agents
              number of `num_agent`.
    """
    # setup_logging(on_worker=False)

    gin.parse_config_file(warehouse_config)
    warehouse_config = WarehouseConfig()

    # import torch
    # torch.set_num_threads(1)
    os.environ["MKL_NUM_THREADS"] = "1"
    os.environ["NUMEXPR_NUM_THREADS"] = "1"
    os.environ["OMP_NUM_THREADS"] = "1"

    # # Read in map
    # with open(map_filepath, "r") as f:
    #     raw_env_json = json.load(f)
    # map_json = json.dumps(raw_env_json)

    # Create log dir
    map_name = pathlib.Path(map_filepath).stem
    mapf_solver = warehouse_config.epibt_planner_type
    pibt_window_size = warehouse_config.pibt_window_size
    pibt_w_str = "" if mapf_solver == "PIBT" else f"w={pibt_window_size}"
    rotation_str = "r" if warehouse_config.rotation else ""
    log_dir = LogDir(f"{map_name}_{mapf_solver}_{pibt_w_str}_{rotation_str}",
                     rootdir=LOG_DIR,
                     custom_dir=reload,
                     uuid=8)
    results_dir = log_dir.dir("results", touch=True)

    # Write map file to logdir
    with open(log_dir.file("map.txt"), "w") as f:
        with open(map_filepath, "r") as mf:
            f.write(mf.read())

    have_run = set()
    if reload is not None and reload != "":
        all_results_dir = os.path.join(reload, "results")
        for result_dir in os.listdir(all_results_dir):
            result_dir_full = os.path.join(all_results_dir, result_dir)
            if os.path.exists(os.path.join(result_dir_full, "result.json")) and\
               os.path.exists(os.path.join(result_dir_full, "config.json")):
                curr_configs = result_dir.split("-")
                curr_num_agent = int(curr_configs[1].split("=")[1])
                curr_seed = int(curr_configs[2].split("=")[1])
                have_run.add((curr_num_agent, curr_seed))
            else:
                # breakpoint()
                shutil.rmtree(result_dir_full)

    pool = multiprocessing.Pool(n_workers)
    n_runs = None
    if mode == "inc_agents":
        seeds = []
        # num_agents = []
        configs = []
        num_agent_range = range(0, n_evals, num_agent_step_size)
        actual_n_evals = len(num_agent_range)
        for i in range(n_sim):
            for j in num_agent_range:
                # Overwrite the config, if necessary
                curr_config = copy.deepcopy(warehouse_config)
                curr_seed = seed + i
                curr_num_agent = num_agent + j
                if (curr_num_agent, curr_seed) in have_run:
                    continue
                seeds.append(curr_seed)
                # num_agents.append(curr_num_agent)
                curr_config.num_agents = curr_num_agent
                configs.append(curr_config)
        n_runs = actual_n_evals * n_sim - len(have_run)

        pool.starmap(
            single_simulation,
            zip(
                configs,
                repeat(map_filepath, n_runs),
                seeds,
                repeat(results_dir, n_runs),
            ),
        )
    elif mode == "constant":
        raise NotImplementedError("Constant mode not implemented")

    print(f"Finished running {n_runs} simulations.")


if __name__ == "__main__":
    fire.Fire(main)
