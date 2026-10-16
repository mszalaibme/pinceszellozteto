"""Produce scientific plots from the native 7-day baseline and sensitivity CSVs."""
import argparse
import csv
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('summary', type=Path)
parser.add_argument('--out', type=Path)
args = parser.parse_args()
out = args.out or args.summary.parent / 'figures'
out.mkdir(parents=True, exist_ok=True)
with args.summary.open(encoding='utf-8', newline='') as f:
    rows = list(csv.DictReader(f))
def group(name):
    return [r for r in rows if r['group'] == name]
def save(fig, name):
    fig.savefig(out / name, dpi=150)
    plt.close(fig)
    print(out / name)

colors = ['#527fa7', '#147d64', '#b48b34', '#b6673b', '#885e9d']
fig, axes = plt.subplots(3, 1, figsize=(13, 10), sharex=True, constrained_layout=True)
for r, color in zip(group('baseline'), colors):
    with (args.summary.parent / (r['runId'] + '.csv')).open(encoding='utf-8', newline='') as f:
        samples = list(csv.DictReader(f))
    days = [float(s['timestamp']) / 86400 for s in samples]
    label = f"{float(r['moistureIngressGPerHour']):g} g/h"
    axes[0].plot(days, [float(s['basementRH']) for s in samples], label=label, color=color, lw=1)
    axes[1].plot(days, [float(s['basementTempC']) for s in samples], color=color, lw=1)
    axes[2].plot(days, [float(s['cumulativeVentilationWaterRemovedL']) + float(s['cumulativeDehumidifierWaterRemovedL']) for s in samples], color=color, lw=1)
axes[0].axhline(65, color='#333333', ls='--', lw=1, label='65% target')
axes[0].set(ylim=(45, 102), ylabel='RH [%]', title='Derived RH: no direct RH interpolation')
axes[0].legend(ncol=3, fontsize=9)
axes[1].axhline(10, color='#333333', ls='--', lw=1)
axes[1].set(ylabel='Temperature [°C]')
axes[2].set(ylabel='Ventilation + tank [L]', xlabel='Simulation time [days]')
for ax in axes:
    ax.grid(alpha=0.2)
fig.suptitle('7-day winter baseline · 26.4 m³ · Q=100 m³/h · assumed thermal properties')
save(fig, 'baseline-7day.png')

subset = [r for r in group('factorial') if float(r['targetRH']) == 65 and float(r['minBasementTemperature']) == 10 and abs(float(r['referenceExtractionLh']) - 0.21) < 1e-9]
if subset:
    flows = sorted({float(r['ventilationAirflowM3h']) for r in subset})
    sources = sorted({float(r['moistureIngressGPerHour']) for r in subset})
    fig, axes = plt.subplots(1, 3, figsize=(15, 5), constrained_layout=True)
    for ax, key, title, cmap in zip(axes, ['averageBasementRH', 'minimumBasementTemperature', 'timeAbove80RHHours'], ['Mean RH [%]', 'Minimum T [°C]', 'Hours above 80% RH'], ['YlOrRd', 'viridis', 'YlOrRd']):
        matrix = np.full((len(sources), len(flows)), np.nan)
        for r in subset:
            matrix[sources.index(float(r['moistureIngressGPerHour'])), flows.index(float(r['ventilationAirflowM3h']))] = float(r[key])
        im = ax.imshow(matrix, aspect='auto', origin='lower', cmap=cmap)
        ax.set_xticks(range(len(flows)), [f'{v:g}' for v in flows])
        ax.set_yticks(range(len(sources)), [f'{v:g}' for v in sources])
        ax.set(title=title, xlabel='Airflow [m³/h]', ylabel='Ingress [g/h]')
        for i in range(len(sources)):
            for j in range(len(flows)):
                ax.text(j, i, f'{matrix[i,j]:.1f}', ha='center', va='center', color='black', fontsize=9, bbox={'facecolor': 'white', 'alpha': 0.6, 'edgecolor': 'none'})
        fig.colorbar(im, ax=ax, shrink=0.8)
    fig.suptitle('Factorial slice: target 65%, minimum 10°C, reference 0.21 L/h')
    save(fig, 'airflow-ingress-sensitivity.png')

for group_name, xkey, filename, xlabel in [('extraction-wet', 'referenceExtractionLh', 'extraction-sensitivity.png', 'Empirical reference [L/h]'), ('power-wet', 'dehumidifierPowerW', 'power-sensitivity.png', 'ASSUMED device power [W]')]:
    data = sorted(group(group_name), key=lambda r: float(r[xkey]))
    fig, axes = plt.subplots(1, 3, figsize=(14, 4.5), constrained_layout=True)
    for ax, key, title in zip(axes, ['averageBasementRH', 'dehumidifierRuntimeHours', 'estimatedElectricityKWh'], ['Mean RH [%]', 'Compressor runtime [h]', 'Accounted energy [kWh]']):
        ax.plot([float(r[xkey]) for r in data], [float(r[key]) for r in data], marker='o', color='#147d64')
        ax.set(title=title, xlabel=xlabel)
        ax.grid(alpha=0.2)
    fig.suptitle('Wet outdoor regime · 200 g/h ingress · daily tank emptying · fan electricity excluded')
    save(fig, filename)

data = group('thermal')
fig, axes = plt.subplots(1, 2, figsize=(12, 4.5), constrained_layout=True)
for cap, color in zip(sorted({float(r['effectiveThermalCapacityJPerK']) for r in data}), colors):
    selected = sorted([r for r in data if float(r['effectiveThermalCapacityJPerK']) == cap and float(r['wallAirHeatTransferWPerK']) == 35], key=lambda r: float(r['wallTemperatureC']))
    for ax, key, title in zip(axes, ['minimumBasementTemperature', 'averageBasementRH'], ['Minimum basement T [°C]', 'Mean RH [%]']):
        ax.plot([float(r['wallTemperatureC']) for r in selected], [float(r[key]) for r in selected], marker='o', label=f'{cap/1e6:g} MJ/K', color=color)
        ax.set(title=title, xlabel='ASSUMED wall/soil T [°C]')
        ax.grid(alpha=0.2)
        ax.legend()
fig.suptitle('Thermal sensitivity: effective building capacity + air capacity; wall transfer 35 W/K')
save(fig, 'thermal-sensitivity.png')

fit = group('observation-fit')
if fit:
    r = fit[0]
    with (args.summary.parent / (r['runId'] + '.csv')).open(encoding='utf-8', newline='') as f:
        data = list(csv.DictReader(f))
    hours = [float(s['timestamp']) / 3600 for s in data]
    fig, axes = plt.subplots(3, 1, figsize=(11, 8), sharex=True, constrained_layout=True)
    for ax, key, label in zip(axes, ['basementRH', 'basementTempC', 'tankLevelLiters'], ['Derived RH [%]', 'Temperature [°C]', 'Physical tank [L]']):
        ax.plot(hours, [float(s[key]) for s in data], color='#147d64')
        ax.axvline(24, color='#b58a36', ls='--', lw=1, label='24 h observation')
        if r['firstTankFullHours']:
            ax.axvline(float(r['firstTankFullHours']), color='#ab5348', ls=':', lw=1, label='Model tank full')
        ax.set_ylabel(label)
        ax.grid(alpha=0.2)
    axes[0].legend()
    axes[-1].set_xlabel('Simulation time [hours]')
    fig.suptitle('One illustrative fit to 5 L / ~24 h; climate and ingress are HYPOTHESES, not measurements')
    save(fig, 'observation-fit.png')
