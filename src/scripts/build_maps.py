import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.backends.backend_pdf import PdfPages
import os

images = {
    'random': 'image/random.png',
    'brc202d': 'image/game.png',
    'Paris-1-256': 'image/city.png',
    'sortation': 'image/sortation.png',
    'warehouse': 'image/warehouse.png'
}

fig = plt.figure(figsize=(6, 6))
gs = gridspec.GridSpec(2, 3, width_ratios=[1, 1, 0.1], height_ratios=[1, 1])

ax1 = plt.subplot(gs[0, 0])
ax1.imshow(plt.imread(images['random']))
ax1.set_title('random-32-32-20')
ax1.set_xticks([])
ax1.set_yticks([])

ax2 = plt.subplot(gs[0, 1])
ax2.imshow(plt.imread(images['brc202d']))
ax2.set_title('brc202d')
ax2.set_xticks([])
ax2.set_yticks([])

ax3 = plt.subplot(gs[1, 0])
ax3.imshow(plt.imread(images['Paris-1-256']))
ax3.set_title('Paris-1-256')
ax3.set_xticks([])
ax3.set_yticks([])

ax4 = plt.subplot(gs[1, 1:])
inner_gs = gridspec.GridSpecFromSubplotSpec(2, 1, subplot_spec=gs[1, 1:], hspace=1)

ax4_1 = plt.subplot(inner_gs[0])
ax4_1.imshow(plt.imread(images['sortation']), aspect='auto')
ax4_1.set_title('sortation')
ax4_1.set_xticks([])
ax4_1.set_yticks([])

ax4_2 = plt.subplot(inner_gs[1])
ax4_2.imshow(plt.imread(images['warehouse']), aspect='auto')
ax4_2.set_title('warehouse')
ax4_2.set_xticks([])
ax4_2.set_yticks([])

plt.tight_layout()
plt.savefig("src/scripts/metrics_plot.pdf", format='pdf', bbox_inches="tight", dpi=800, pad_inches=0.2)
