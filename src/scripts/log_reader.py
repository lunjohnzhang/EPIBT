import os

def read_config_file(file_path):
    config = {}
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            _, line = line.split(']', 1)
            key, value = line.split(':', 1)
            # print(key, value)
            config[key.strip()] = value.strip()
    #print(config)
    return config


# Пример использования
# config = read_config_file('solutions/lmapf-t/winpibt/random/0/log.txt')
# print(config)

def process_log(map, agents_num, steps_num, test_id):
    path = f'solutions/lmapf/real_pibt/{map.lower()}/{test_id}/log.txt'
    if not os.path.exists(path):
        return
    config = read_config_file(path)
    if len(config) == 0:
        return

    output = f'solutions/lmapf/real_pibt/{map.lower()}/{test_id}/metrics.csv'
    with open(output, 'w') as file:
        print(
            f"metric,value\n\
task type,LMAPF\n\
map type,{map}\n\
test id,{test_id}\n\
scheduler type,CONST\n\
planner type,PIBT\n\
graph guidance type,disable\n\
agents num,{agents_num}\n\
steps num,{steps_num}\n\
finished tasks,{int(float(config['throughput']) * steps_num)}\n\
throughput,{config['throughput']}\n\
E,0\n\
S,0\n\
W,0\n\
N,0\n\
w,0\n\
avg step time (ms),{float(config['elapsed']) / steps_num}\n\
avg scheduler time (ms),0\n\
avg planner time (ms),{float(config['elapsed']) / steps_num}"
            , file=file
        )
        pass

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

#process_log('solutions/lmapf-t/winpibt/random/0/log.txt', 'RANDOM', 100, 1000, 0)

for x in X:
    process_log(*x)
