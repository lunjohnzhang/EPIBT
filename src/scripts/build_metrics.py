import pandas as pd
import os
from pathlib import Path

INPUT_DIRS = ["solutions"]
OUTPUT_FILENAME = 'metrics.csv'
REMOVE_ACTIONS = True

table = {
}


def process_dir(path):
    filename = path / "metrics.csv"
    if os.path.exists(filename):
        print("process:", filename)
        df = pd.read_csv(filename)
        df = df.T
        df.columns = df.iloc[0]
        df = df[1:]

        for column_name in df.columns:
            value = df[column_name].iloc[0]
            if REMOVE_ACTIONS and (column_name == "F" or column_name == "R" or column_name == "C" \
                                   or column_name == "E" or column_name == "S" or column_name == "W" or column_name == "N" \
                                   or column_name == "w"):
                pass
            if not column_name in table:
                table[column_name] = []
            table[column_name].append(value)


def scan_directory(path):
    path = Path(path)
    for item in path.rglob('*'):
        if item.is_dir():
            process_dir(item)


for dir in INPUT_DIRS:
    if len(dir) != 0 and dir[-1] != '/' and dir[-1] != '\\':
        dir += '/'
    scan_directory(dir)

table = pd.DataFrame(data=table)
table['test id'] = table['test id'].astype(int)
table['agents num'] = table['agents num'].astype(int)
table['steps num'] = table['steps num'].astype(int)
table['finished tasks'] = table['finished tasks'].astype(int)
table = table.sort_values(by=['task type', 'map type', 'test id', 'finished tasks'], ascending=[True, True, True, True])
table.to_csv(OUTPUT_FILENAME, index=False)

grouped = table.groupby('map type')

total_table = pd.DataFrame(data={"map type": [], "throughput": []})
for map_type, df in grouped:
    df['throughput'] = df['throughput'].astype(float)
    throughput = df['throughput'].sum() / len(df['throughput'])
    total_table = pd.concat([pd.DataFrame({'map type': [map_type], 'throughput': [throughput]}), total_table],
                            ignore_index=True)

total_table = total_table.sort_values(by=['map type'], ascending=[True])
print(total_table)
# total_table.to_csv("total_metrics.csv", index=False)
