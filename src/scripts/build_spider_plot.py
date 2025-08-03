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

# planner_type#map_type
planner_throughput = {}

for planner_type, grouped in data.groupby('planner type'):
    for map_type, grouped2 in grouped.groupby('map type'):
        planner_throughput[planner_type + "#" + map_type] = 0
        for test_id, grouped3 in grouped2.groupby('test id'):
            planner_throughput[planner_type + "#" + map_type] += float(grouped3['throughput']) / best_throughput[
                map_type + "#" + str(test_id)]
        if map_type == "RANDOM":
            planner_throughput[planner_type + "#" + map_type] = planner_throughput[planner_type + "#" + map_type] / 8
        else:
            planner_throughput[planner_type + "#" + map_type] = planner_throughput[planner_type + "#" + map_type] / 10

print(planner_throughput)

data = {
    'planner type': [],
    'map type': [],
    'score': [],
}

for key in planner_throughput:
    planner_type, map_type = key.split('#')
    print(planner_type, map_type, planner_throughput[key])
    data['planner type'].append(planner_type)
    data['map type'].append(map_type)
    data['score'].append(planner_throughput[key])

df = pd.DataFrame(data)
print(df)

df.to_csv('src/scripts/table.csv', index=False)


def build_spider_plot(df):
    # Подготовка данных
    planners = df['planner type'].unique()
    map_types = df['map type'].unique()
    angles = np.linspace(0, 2 * np.pi, len(map_types), endpoint=False).tolist()

    # Создаем фигуру
    fig = plt.figure(figsize=(12, 12))
    ax = fig.add_subplot(111, polar=True)

    print(planners)
    # Для каждого planner type рисуем свой график
    for planner in ['PIBT', 'Causal PIBT', 'WPPL', 'EPIBT(3)', 'Causal PIBT+traffic flow', 'LoRR24-Winner', 'EPIBT(3)+LNS', 'WPPL+GG',
                    'LoRR24-Winner+GG', 'EPIBT(3)+LNS+GG']:  # planners:
        # Фильтруем данные по planner type
        planner_data = df[df['planner type'] == planner]

        # Создаем массив scores в правильном порядке углов
        scores = []
        for m in map_types:
            val = planner_data[planner_data['map type'] == m]['score'].values
            scores.append(val[0] if len(val) > 0 else 0)

        # Замыкаем график (добавляем первую точку в конец)
        scores += scores[:1]
        current_angles = angles + angles[:1]

        # Рисуем линию
        if planner in ['PIBT', 'Causal PIBT', 'EPIBT(3)']:
            ax.plot(current_angles, scores, linewidth=3, linestyle='-', label=planner)
        elif "+GG" in planner:
            ax.plot(current_angles, scores, linewidth=3, linestyle='-.', label=planner)
        else:
            ax.plot(current_angles, scores, linewidth=3, linestyle='--', label=planner)
        # ax.fill(current_angles, scores, alpha=0.01)

    # Настройки графика
    ax.set_theta_offset(np.pi / 2)
    ax.set_theta_direction(-1)
    ax.set_rlabel_position(0)
    plt.yticks(fontweight='bold', fontsize=14)
    plt.xticks(angles, ['Paris-1-256', 'brc202d', 'random-32-32-20', 'sortation', 'warehouse'], fontweight='bold', fontsize=17)  # map_types
    ax.tick_params(axis='x', which='major', pad=35)
    plt.ylim(0, 1)
    # plt.title('Сравнение planner types по разным map types', pad=20)
    plt.legend(prop={'weight': 'bold', 'size': 15}, loc='lower left', bbox_to_anchor=(0.7, 0.8))
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig("spider_plot.pdf", format='pdf', bbox_inches="tight", dpi=800, pad_inches=0.2)
    plt.show()


build_spider_plot(df)
