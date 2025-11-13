import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import sys
import matplotlib.colors
import pandas as pd
from PIL import Image
from matplotlib.ticker import FixedFormatter
from matplotlib.ticker import MaxNLocator

mpl.rcParams['pdf.fonttype'] = 42
mpl.rcParams['ps.fonttype'] = 42
mpl.rcParams['text.usetex'] = False

ENABLE_PRINT_TIME = True
ENABLE_TIME_SCALE_LOG = True

PLANNERS = [
    'Causal PIBT+traffic flow',
    'EPIBT(3)+LNS',
    'EPIBT(4)+LNS',
    'EPIBT(5)+LNS',
    'LoRR24-Winner',
    'WPPL'
]

MARKERS = ['o', 'v', 's', 'p', '*', 'x', 'D', 'P']

data = pd.read_csv('metrics.csv', sep=',')

maps = set(data.groupby('map type').groups)
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

    for planner_type in PLANNERS:
        if not planner_type in grouped.groups:
            pass
        df = grouped.get_group(planner_type)

        times = np.maximum(0.1, df['avg planner time (ms)'])

        if not planner_type in planner_to_marker:
            planner_to_marker[planner_type] = MARKERS[marker_it % len(MARKERS)]
            marker_it += 1

        if True:
            if len(maps) == 1:
                ax = axes[0]
            elif not ENABLE_PRINT_TIME:
                ax = axes[column]
            else:
                ax = axes[0][column]
            ax.title.set_text(map_text)
            ax.plot(df['agents num'], df['throughput'], alpha=1, label=planner_type,
                    marker=planner_to_marker[planner_type])
            if is_first:
                ax.set_ylabel('Throughput')
            ax.grid(True)

        if ENABLE_PRINT_TIME:
            if len(maps) == 1:
                ax = axes[1]
            elif not ENABLE_PRINT_TIME:
                ax = axes[column]
            else:
                ax = axes[1][column]
            ax.plot(df['agents num'], times, alpha=1, label=planner_type,
                    marker=planner_to_marker[planner_type])
            if is_first:
                ax.set_ylabel('Decision Time (ms)')
            ax.grid(True)
            ax.set_xlabel('Number of Agents')

            if ENABLE_TIME_SCALE_LOG:
                ax.set_yscale('log')
                ax.set_ylim(0.07, 1500)

    if is_first:
        is_first = False

    if len(maps) == 1:
        ax = axes[0]
    elif not ENABLE_PRINT_TIME:
        ax = axes[column]
    else:
        ax = axes[0][column]
    ax.set_ylim(0, None)
    ax.set_yticks(np.unique(np.round(ax.get_yticks()).astype(int)))


if __name__ == '__main__':
    row_len = 1
    if ENABLE_PRINT_TIME:
        row_len = 2
    fig, axes = plt.subplots(row_len, len(maps), figsize=(16, 3 * row_len))

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
    fig.legend(lines, labels, loc='lower center', ncol=4, borderaxespad=0.2)
    plt.savefig("metrics_plot.pdf", format='pdf', bbox_inches="tight", dpi=800, pad_inches=0.35)
