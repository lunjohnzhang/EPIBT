#!/bin/bash

USAGE="Usage: bash scripts/run_single_sim.sh SIM_CONFIG MAP_FILE AGENT_NUM AGENT_NUM_STEP_SIZE N_EVALS MODE N_SIM N_WORKERS DOMAIN -r RELOAD -p PROJECT_DIR"

SIM_CONFIG="$1"
MAP_FILE="$2"
AGENT_NUM="$3"
AGENT_NUM_STEP_SIZE="$4"
N_EVALS="$5"
MODE="$6"
N_SIM="$7"
N_WORKERS="$8"
DOMAIN="$9"

shift 9
while getopts "p:r:" flag; do
  case "$flag" in
      p) PROJECT_DIR=$OPTARG;;
      r) RELOAD=$OPTARG;;
      *) echo "Invalid option. ${USAGE}"
  esac
done

if [ -n "$PROJECT_DIR" ]; then
  SINGULARITY_OPTS="$SINGULARITY_OPTS --bind ${PROJECT_DIR}:${PROJECT_DIR}"
fi

RELOAD_ARG=""
if [ -n "$RELOAD" ]; then
  RELOAD_ARG="--reload ${RELOAD}"
fi

if [ "${DOMAIN}" = "kiva" ]
then
    if [ "${MODE}" = "inc_agents" ]
    then
        python py_src/run_epibt.py \
                --warehouse-config "$SIM_CONFIG" \
                --map-filepath "$MAP_FILE" \
                --num-agent "$AGENT_NUM" \
                --num-agent-step-size "$AGENT_NUM_STEP_SIZE" \
                --n_evals "$N_EVALS" \
                --mode "$MODE" \
                --n_sim "$N_SIM" \
                --n_workers "$N_WORKERS" \
                ${RELOAD_ARG}
        sleep 2
    fi
fi