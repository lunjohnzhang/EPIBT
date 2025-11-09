import matplotlib.pyplot as plt
import numpy as np
import sys
import matplotlib.colors
# import seaborn as sns
import pandas as pd
from PIL import Image
from matplotlib.ticker import FixedFormatter
from matplotlib.ticker import MaxNLocator

data = pd.read_csv('metrics.csv', sep=',')

# "map#test_id"
best_throughput = {}

for map_type, grouped in data.groupby('map type'):
    grouped = grouped.groupby('test id')
    for test_id, grouped2 in grouped:
        best_throughput[map_type + "#" + str(test_id)] = grouped2['throughput'].max()

print(best_throughput)

# [planner_type#map_type]
planner_throughput = {}

for planner_type, grouped in data.groupby('planner type'):
    for map_type, grouped2 in grouped.groupby('map type'):
        for test_id, grouped3 in grouped2.groupby('test id'):
            planner_throughput[planner_type + "#" + map_type + "#" + str(test_id)] = float(grouped3['throughput']) / \
                                                                                     best_throughput[
                                                                                         map_type + "#" + str(test_id)]

print(planner_throughput)

data = {
    'planner type': [],
    'map type': [],
    'test id': [],
    'score': [],
}

for key in planner_throughput:
    planner_type, map_type, test_id = key.split('#')
    print(planner_type, map_type, test_id, planner_throughput[key])
    data['planner type'].append(planner_type)
    data['map type'].append(map_type)
    data['test id'].append(test_id)
    data['score'].append(planner_throughput[key])

df = pd.DataFrame(data)
print(df)

df.to_csv('table.csv', index=False)

def build_spider_plot(df):
    # Подготовка данных
    planners = df['planner type'].unique()
    map_types = df['map type'].unique()
    angles = np.linspace(0, 2 * np.pi, 48, endpoint=False).tolist()

    # Создаем фигуру
    fig = plt.figure(figsize=(10, 10))
    ax = fig.add_subplot(111, polar=True)

    print(planners)

    # Define color mapping for related planners
    color_map = {
        'PIBT': '#1f77b4',  # Blue
        'Causal PIBT': '#ff7f0e',  # Orange
        'Causal PIBT+traffic flow': '#ff7f0e',  # Same as Causal PIBT
        'EPIBT(3)': '#2ca02c',  # Green
        'EPIBT(3)+LNS': '#2ca02c',  # Same as EPIBT(3)
        'EPIBT(3)+LNS+GG': '#2ca02c',  # Same as EPIBT(3)
        'WPPL': '#d62728',  # Red
        'WPPL+GG': '#d62728',  # Same as WPPL
        'LoRR24-Winner': '#9467bd',  # Purple
        'LoRR24-Winner+GG': '#9467bd',  # Same as LoRR24-Winner
    }

    # Для каждого planner type рисуем свой график
    for planner in ['PIBT', 'Causal PIBT', 'EPIBT(3)', 'WPPL', 'LoRR24-Winner', 'EPIBT(3)+LNS', 'Causal PIBT+traffic flow',
                    'WPPL+GG',
                    'LoRR24-Winner+GG', 'EPIBT(3)+LNS+GG']:  # planners:
        # Фильтруем данные по planner type
        planner_data = df[df['planner type'] == planner]

        # Создаем массив scores в правильном порядке углов
        scores = []
        for m in map_types:
            max_test = 10
            if m == "RANDOM":
                max_test = 8
            for test_id in range(max_test):
                # val = planner_data[planner_data['map type'] == m and planner_data['test id'] == test_id]['score'].values
                # scores.append(val[0] if len(val) > 0 else 0)
                kek = planner_data[planner_data['map type'] == m]
                val = kek[kek['test id'] == str(test_id)].values
                scores.append(float(val[0][3] if len(val) > 0 else 0))

        # Замыкаем график (добавляем первую точку в конец)
        scores += scores[:1]
        current_angles = angles + angles[:1]

        # Рисуем линию
        if planner in ['PIBT', 'Causal PIBT', 'EPIBT(3)']:
            ax.plot(current_angles, scores, linewidth=3, linestyle='-', label=planner, color=color_map[planner])
        elif "+GG" in planner or "+traffic flow" in planner:
            ax.plot(current_angles, scores, linewidth=3, linestyle='--', label=planner, color=color_map[planner])
        else:
            ax.plot(current_angles, scores, linewidth=3, linestyle=':', label=planner, color=color_map[planner])

    # Настройки графика
    ax.set_theta_offset(np.pi / 2)
    ax.set_theta_direction(-1)
    ax.set_rlabel_position(0)
    plt.yticks(fontweight='bold', fontsize=16)
    plt.xticks(np.linspace(0, 2 * np.pi, 5, endpoint=False).tolist(), ['Paris-1-256', 'brc202d', 'random-32-32-20', 'sortation', 'warehouse'], fontweight='bold', fontsize=20)  # map_types
    ax.tick_params(axis='x', which='major', pad=35)
    plt.ylim(0, 1)
    plt.legend(prop={'weight': 'bold', 'size': 15}, loc='lower left', bbox_to_anchor=(0.7, 0.8))
    plt.grid(True, linewidth=3)
    plt.tight_layout()
    plt.savefig("spider_plot.pdf", format='pdf', bbox_inches="tight", dpi=800, pad_inches=0.0)
    plt.show()


build_spider_plot(df)
