import matplotlib.pyplot as plt
import numpy as np
import sys
import matplotlib.colors
import pandas as pd

assert len(sys.argv) == 6, "usage: <input> <output> <div> <figsize_row> <figsize_col>"

INPUT_FILENAME = sys.argv[1]  # "solutions/0/heatmap_all.csv"
OUTPUT_FILENAME = sys.argv[2]  # "src/scripts/heatmap.pdf"
DIV_NUM = int(sys.argv[3])  # 1000
FIGSIZE_ROW = int(sys.argv[4])  # 10
FIGSIZE_COL = int(sys.argv[5])  # 10

df = pd.read_csv(INPUT_FILENAME)
df = df.drop(df.columns[0], axis=1)

good_cmap = matplotlib.colors.LinearSegmentedColormap.from_list("", [(0, '#008064'), (.1, "#FFFF64"), (0.5, "#FF6464"),
                                                                     (1, "#960064")])

fig, ax = plt.subplots(1, 1, figsize=(FIGSIZE_COL, FIGSIZE_ROW), constrained_layout=True)
images = []
map = np.array(df)
mask = map == -1
map = map / DIV_NUM

images.append(ax.imshow(map, cmap=good_cmap, vmin=0, vmax=1))

red_mask = np.zeros((*map.shape, 4))
red_mask[mask] = [0, 0, 0, 1]
ax.imshow(red_mask)

ax.axis('off')
fig.colorbar(images[-1], ax=ax)
plt.savefig(OUTPUT_FILENAME, format='pdf', dpi=400, bbox_inches='tight')
plt.show()
