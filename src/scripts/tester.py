import os

for map in ["RANDOM",
            "CITY",
            "GAME",
            "SORTATION",
            "WAREHOUSE",
            ]:
    steps_num = 5000
    if map == "RANDOM":
        steps_num = 1000

    with open("tmp.config", 'w') as file:
        print(
            f"\n\
# OMAPFS Configuration Example\n\
# Map: RANDOM, CITY, GAME, SORTATION, WAREHOUSE\n\
map_type = {map}\n\
\n\
# Planner algorithms: PIBT, CAUSAL_PIBT, PIBT_TF, EPIBT, EPIBT_LNS, PEPIBT_LNS, WPPL\n\
planner_type = PIBT\n\
\n\
# Scheduler types: CONST, GREEDY\n\
scheduler_type = CONST\n\
\n\
# Graph Guidance types: DISABLE, ENABLE\n\
graph_guidance_type = DISABLE\n\
\n\
# TestSystem file paths\n\
map_file = tests/{map.lower()}/map.txt\n\
tasks_path = tests/{map.lower()}/tasks_one.csv\n\
agents_path = tests/{map.lower()}/\n\
steps_num = {steps_num}\n\
output_path = solutions/{map.lower()}", file=file
        )
    ret = os.system("./bin/main tmp.config")
    assert ret == 0, "invalid return code: " + str(ret)
