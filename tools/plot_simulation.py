"""Plot simulator CSV (requires matplotlib); never integrates relative humidity."""
import argparse
import csv
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('csv', type=Path)
parser.add_argument('--output', type=Path)
args = parser.parse_args()
with args.csv.open(encoding='utf-8', newline='') as stream:
    rows = list(csv.DictReader(stream))
if not rows:
    parser.error('CSV contains no samples')
def values(key):
    return [float(r[key]) if r[key] else float('nan') for r in rows]
time = [t / 3600 for t in values('timestamp')]
specs = [
    ('Temperature', '°C', [('basementTemp', 'Basement'), ('outsideTemp', 'Outdoor')]),
    ('Relative humidity (derived)', '%', [('basementRH', 'Basement'), ('outsideRH', 'Outdoor')]),
    ('Absolute humidity', 'g/m³', [('basementAH', 'Basement'), ('outsideAH', 'Outdoor')]),
    ('Humidity advantage', 'g/m³', [('deltaAH', 'Delta AH')]),
    ('Ventilation', 'ON', [('ventilationCommand', 'Command')]),
    ('Dehumidifier', 'ON', [('dehumidifierCommand', 'Socket'), ('compressorRunning', 'Compressor')]),
    ('Water tank', 'L', [('tankLevelLiters', 'Physical model'), ('estimatedTankLiters', 'ESTIMATED')]),
    ('Dehumidifier power', 'W', [('dehumidifierPowerW', 'Power')]),
]
fig, axes = plt.subplots(8, 1, figsize=(12, 20), sharex=True, constrained_layout=True)
for ax, (title, unit, series) in zip(axes, specs):
    for key, label in series:
        style = 'steps-post' if key in ('ventilationCommand', 'dehumidifierCommand', 'compressorRunning') else 'default'
        ax.plot(time, values(key), label=label, drawstyle=style, linewidth=1.2)
    ax.set_title(title, loc='left', fontsize=11)
    ax.set_ylabel(unit)
    ax.grid(alpha=0.2)
    ax.legend(loc='upper right', fontsize=8)
axes[-1].set_xlabel('Simulation time [hours]')
fig.suptitle(args.csv.stem)
destination = args.output or args.csv.with_suffix('.png')
destination.parent.mkdir(parents=True, exist_ok=True)
fig.savefig(destination, dpi=150)
print(destination.resolve())
