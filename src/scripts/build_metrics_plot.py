import matplotlib.pyplot as plt
import numpy as np
import sys
import matplotlib.colors
# import seaborn as sns
import pandas as pd
from PIL import Image
from matplotlib.ticker import FixedFormatter
from matplotlib.ticker import MaxNLocator

PRINT_TIME = True

'''
'PIBT+MAO(3)',
'PIBT+MAO(3)+Revisit(∞)',
'PIBT+MAO(3)+Revisit(10)',
'PIBT+MAO(3)+Revisit(10)+IO',

'PIBT+MAO(4)+Revisit(10)',
'PIBT+MAO(4)+Revisit(10)+IO',

'PIBT+MAO(5)+Revisit(10)',
'PIBT+MAO(5)+Revisit(10)+IO',
'''

'''
'PIBT',
'PIBT+Revisit(10)',
'Causal PIBT',
'EPIBT(3)',
'''

'''
'PIBT+traffic flow',
'EPIBT(3)+LNS',
'LoRR24-Winner',
'WPPL',
'''

'''
'Causal PIBT+traffic flow',
'EPIBT(3)+LNS',
'EPIBT(4)+LNS',
'EPIBT(5)+LNS',
'LoRR24-Winner',
'WPPL',
'''

'''
'Causal PIBT+traffic flow',
'EPIBT(3)+LNS+GG',
'EPIBT(4)+LNS+GG',
'EPIBT(5)+LNS+GG',
'LoRR24-Winner+GG',
'WPPL+GG',
'''

PLANNERS = [
    'PIBT',
    'winPIBT',
    'EPIBT(1)',
    'EPIBT(2)',
    'EPIBT(3)',
]

# plan_algos = ["EPIBT(1)", "EPIBT(2)", "EPIBT(3)"]
# colors = ['lime', 'dodgerblue', 'orange', 'red', 'blueviolet', 'aqua', 'deeppink', 'brown']
# colors = ['green', 'blue', 'orange', 'red', 'blueviolet', 'aqua', 'deeppink', 'brown']
# plan_algos_name = ['EPIBT+LNS+GG', 'EPIBT+LNS', 'EPIBT+GG', 'EPIBT', 'PIBT+GG', 'PIBT', 'WPPL+GG', 'PIBT+traffic flow']
markers = ['o', 'v', 's', 'p', '*', 'x', 'D', 'P']

# color_palette = sns.color_palette("tab10", 8)
# plt.rcParams['axes.prop_cycle'] = plt.cycler(color=color_palette)

data = pd.read_csv('metrics.csv', sep=',')

maps = {'RANDOM', 'CITY'}#set(data.groupby('map type').groups)
print("maps:", maps)

is_first = True

planner_to_marker = dict()
marker_it = 0


def add_map(map_name, map_text, column):
    global marker_it
    global is_first
    if not map_name in maps:
        return
    grouped = data.groupby('map type').get_group(map_name)
    grouped = grouped.groupby('planner type')

    if False:
        if len(maps) == 1:
            ax = axes[0]
        else:
            ax = axes[0][column]
        ax.imshow(np.asarray(Image.open("image/" + map_name.lower() + '.png')))
        ax.title.set_text(map_text)
        ax.set_xticks([])
        ax.set_yticks([])

    # grouped.groups
    for planner_type in PLANNERS:
        # if len(PLANNERS) != 0 and not planner_type in PLANNERS:
        #    continue
        if not planner_type in grouped.groups:
            pass  # continue
        df = grouped.get_group(planner_type)
        # planner_type = PLANNERS.get(kek)

        times = np.maximum(0.1, df['avg planner time (ms)'])

        if not planner_type in planner_to_marker:
            planner_to_marker[planner_type] = markers[marker_it % len(markers)]
            marker_it += 1

        if True:
            if len(maps) == 1:
                ax = axes[0]
            elif not PRINT_TIME:
                ax = axes[column]
            else:
                ax = axes[0][column]
            ax.title.set_text(map_text)
            ax.plot(df['agents num'], df['throughput'], alpha=1, label=planner_type,
                    marker=planner_to_marker[planner_type])
            if is_first:
                ax.set_ylabel('Throughput')
            ax.grid(True)

        if PRINT_TIME:
            if len(maps) == 1:
                ax = axes[1]
            elif not PRINT_TIME:
                ax = axes[column]
            else:
                ax = axes[1][column]
            ax.plot(df['agents num'], times, alpha=1, label=planner_type,
                    marker=planner_to_marker[planner_type])
            ax.set_yscale('log')
            if is_first:
                ax.set_ylabel('Decision Time (ms)')
            ax.grid(True)
            ax.set_xlabel('Number of Agents')
            ax.set_ylim(0.07, 1500)

    if is_first:
        is_first = False

    if len(maps) == 1:
        ax = axes[0]
    elif not PRINT_TIME:
        ax = axes[column]
    else:
        ax = axes[0][column]
    ax.set_ylim(0, None)
    ax.set_yticks(np.unique(np.round(ax.get_yticks()).astype(int)))


if __name__ == '__main__':
    row_len = 1
    if PRINT_TIME:
        row_len = 2
    fig, axes = plt.subplots(row_len, len(maps), figsize=(8, 3 * row_len))

    add_map('RANDOM', 'random-32-32-20', 0)  # \nSize: 32x32\n|V|=819
    add_map('CITY', 'Paris-1-256', 1)  # \nSize: 256x256\n|V|=47240
    add_map('GAME', 'brc202d', 2)  # \nSize: 481x530\n|V|=43151
    add_map('SORTATION', 'sortation', 3)  # \nSize: 140x500\n|V|=54320
    add_map('WAREHOUSE', 'warehouse', 4)  # \nSize: 140x500\n|V|=38586

    lines_labels = [ax.get_legend_handles_labels() for ax in fig.axes]
    lines, labels = [sum(lol, []) for lol in zip(*lines_labels)]
    # remove not unique lines
    while True:
        kek = labels.copy()
        if len(kek) == 0:
            break
        kek.pop(-1)
        if labels[-1] in kek:
            lines.pop(-1)
            labels.pop(-1)
        else:
            break
    # print(labels)
    fig.legend(lines, labels, loc='lower center', ncol=10, borderaxespad=0.2)
    #plt.tight_layout()
    plt.savefig("metrics_plot.pdf", format='pdf', bbox_inches="tight", dpi=800, pad_inches=0.35)
    # plt.show()
