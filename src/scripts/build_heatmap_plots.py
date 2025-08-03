import matplotlib.pyplot as plt
import numpy as np
import sys
import matplotlib.colors
import pandas as pd

DIV_NUM = 5000
BEGIN_PATH = "solutions_super/lmapf-t/"
END_PATH = "/sortation/9/heatmap_w.csv"
OUTPUT_FILE = "src/scripts/kek.pdf"

good_cmap = matplotlib.colors.LinearSegmentedColormap.from_list("", [(0, '#008064'), (.1, "#FFFF64"), (0.5, "#FF6464"),
                                                                     (1, "#960064")])

if __name__ == '__main__':
    algos = [
        ["causal_pibt", "Causal PIBT"],
        ["pibt_tf", "PIBT+traffic flow"],
        ["epibt(3)", "EPIBT(3)"],
        ["epibt(3)+gg", "EPIBT(3)+GG"],
        ['lorr24_winner', "LoRR24-Winner"],
        ['lorr24_winner+gg', "LoRR24-Winner+GG"],
        ["epibt(3)+lns", "EPIBT(3)+LNS"],
        ["epibt(3)+lns+gg", "EPIBT(3)+LNS+GG"],
        ["wppl", "WPPL"],
        ["wppl+gg", "WPPL+GG"],
    ]

    '''["causal_pibt", "Causal PIBT"],
        ["epibt(3)", "EPIBT(3)"],
        ['lorr24_winner', "LoRR24-Winner"],
        ["epibt(3)+lns", "EPIBT(3)+LNS"],
        ["wppl", "WPPL"],

        ["pibt_tf", "PIBT+traffic flow"],
        ["epibt(3)+gg", "EPIBT(3)+GG"],
        ['lorr24_winner+gg', "LoRR24-Winner+GG"],
        ["epibt(3)+lns+gg", "EPIBT(3)+LNS+GG"],
        ["wppl+gg", "WPPL+GG"],'''

    '''["causal_pibt", "Causal PIBT"],
        ["pibt_tf", "PIBT+traffic flow"],
        ["epibt(3)", "EPIBT(3)"],
        ["epibt(3)+gg", "EPIBT(3)+GG"],
        ['lorr24_winner', "LoRR24-Winner"],
        ['lorr24_winner+gg', "LoRR24-Winner+GG"],
        ["epibt(3)+lns", "EPIBT(3)+LNS"],
        ["epibt(3)+lns+gg", "EPIBT(3)+LNS+GG"],
        ["wppl", "WPPL"],
        ["wppl+gg", "WPPL+GG"],'''

    row_size = 2
    col_size = 5
    # fig, axes = plt.subplots(col_size, row_size, figsize=(10, 4), constrained_layout=True)
    fig, axes = plt.subplots(col_size, row_size, figsize=(7, 6), constrained_layout=True)

    row = 0
    col = 0
    images = []
    for [algo, title] in algos:
        filename = BEGIN_PATH + algo + END_PATH
        df = pd.read_csv(filename)
        df = df.drop(df.columns[0], axis=1)

        map = np.array(df)
        mask = map == -1
        map = map / DIV_NUM

        ax = axes[row][col]
        col += 1
        if col == row_size:
            col = 0
            row += 1

        images.append(ax.imshow(map, cmap=good_cmap, vmin=0, vmax=1))

        if True:
            red_mask = np.zeros((*map.shape, 4))
            red_mask[mask] = [0, 0, 0, 1]
            ax.imshow(red_mask)

        ax.set_title(title)
        ax.axis('off')

    fig.colorbar(images[-1], ax=axes)
    plt.savefig(OUTPUT_FILE, format='pdf', dpi=800)
