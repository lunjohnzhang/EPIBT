import os
from concurrent.futures import ThreadPoolExecutor
import threading

def process_test(map, agents_num, steps_num, test_id):
    print("start test: ", map, test_id, flush=True)
    with open(f"tmp_{map.lower()}_{test_id}.config", 'w') as file:
        print(
            f"PROBLEM_TYPE=LMAPF\n\
SOLVER_TYPE=PIBT\n\
field=./tests/{map.lower()}/map.txt\n\
agentnum={agents_num}\n\
timesteplimit={steps_num}\n\
tasknum=5\n\
taskfrequency=1.0\n\
scenario=0\n\
scenariofile=\n\
goalsequence=0\n\
goalsequencefile=\n\
startpositions=1\n\
startpositionsfile=./tests/{map.lower()}/agents_{test_id}.csv\n\
csvformat=1\n\
goalpool=./tests/{map.lower()}/tasks_one.csv\n\
seed=0\n\
log=1\n\
printlog=1\n\
printtime=1\n\
detailedlog=0\n\
WarshallFloyd=0\n\
ID=1\n\
suboptimal=1.5\n\
softmode=1\n\
runtime_limit={steps_num * 1000}", file=file
        )

    os.makedirs(f"solutions/lmapf/real_pibt/{map.lower()}/{test_id}/", exist_ok=True)
    ret = os.system(
        f"./src/planner/winpibt/bin/testapp -p tmp_{map.lower()}_{test_id}.config > solutions/lmapf/real_pibt/{map.lower()}/{test_id}/log.txt")
    assert ret == 0, "invalid return code: " + str(ret)
    print("done test: ", map, test_id, ", ret:", ret, flush=True)

X = [
    ["RANDOM", 100, 1000, 0],
    ["RANDOM", 200, 1000, 1],
    ["RANDOM", 300, 1000, 2],
    ["RANDOM", 400, 1000, 3],
    ["RANDOM", 500, 1000, 4],
    ["RANDOM", 600, 1000, 5],
    ["RANDOM", 700, 1000, 6],
    ["RANDOM", 800, 1000, 7],

    ["CITY", 1000, 5000, 0],
    ["CITY", 2000, 5000, 1],
    ["CITY", 3000, 5000, 2],
    ["CITY", 4000, 5000, 3],
    ["CITY", 5000, 5000, 4],
    ["CITY", 6000, 5000, 5],
    ["CITY", 7000, 5000, 6],
    ["CITY", 8000, 5000, 7],
    ["CITY", 9000, 5000, 8],
    ["CITY", 10000, 5000, 9],

    ["GAME", 500, 5000, 0],
    ["GAME", 1000, 5000, 1],
    ["GAME", 1500, 5000, 2],
    ["GAME", 2000, 5000, 3],
    ["GAME", 2500, 5000, 4],
    ["GAME", 3000, 5000, 5],
    ["GAME", 3500, 5000, 6],
    ["GAME", 4000, 5000, 7],
    ["GAME", 4500, 5000, 8],
    ["GAME", 5000, 5000, 9],

    ["SORTATION", 1000, 5000, 0],
    ["SORTATION", 2000, 5000, 1],
    ["SORTATION", 3000, 5000, 2],
    ["SORTATION", 4000, 5000, 3],
    ["SORTATION", 5000, 5000, 4],
    ["SORTATION", 6000, 5000, 5],
    ["SORTATION", 7000, 5000, 6],
    ["SORTATION", 8000, 5000, 7],
    ["SORTATION", 9000, 5000, 8],
    ["SORTATION", 10000, 5000, 9],

    ["WAREHOUSE", 1000, 5000, 0],
    ["WAREHOUSE", 2000, 5000, 1],
    ["WAREHOUSE", 3000, 5000, 2],
    ["WAREHOUSE", 4000, 5000, 3],
    ["WAREHOUSE", 5000, 5000, 4],
    ["WAREHOUSE", 6000, 5000, 5],
    ["WAREHOUSE", 7000, 5000, 6],
    ["WAREHOUSE", 8000, 5000, 7],
    ["WAREHOUSE", 9000, 5000, 8],
    ["WAREHOUSE", 10000, 5000, 9],
]

def f(id):
    process_test(*X[id])

# Флаг для остановки
stop_event = threading.Event()

try:
    with ThreadPoolExecutor(max_workers=10) as executor:
        executor.map(f, range(len(X)))
except KeyboardInterrupt:
    print("\nПолучен Ctrl+C, останавливаем потоки...", flush=True)
    stop_event.set()  # Говорим задачам завершиться
    executor.shutdown(wait=False)  # Не ждём завершения (можно wait=True для graceful shutdown)
    print("Пул потоков остановлен", flush=True)

print("Done", flush=True)
