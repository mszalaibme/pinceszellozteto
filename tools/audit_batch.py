"""Independent dimensional and bookkeeping audit of the batch summary/export."""
import argparse
import collections
import csv
import json
import math
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('summary', type=Path)
parser.add_argument('--output', type=Path)
args = parser.parse_args()
with args.summary.open(encoding='utf-8', newline='') as f:
    rows = list(csv.DictReader(f))
assert rows, 'empty summary'
max_water = max_heat = max_energy = 0.0
for r in rows:
    n = lambda key: float(r[key])
    for key, value in r.items():
        if key not in ('group', 'runId', 'powerStatus') and value:
            assert math.isfinite(float(value)), (r['runId'], key)
    hours = n('hours')
    assert 0 <= n('averageBasementRH') <= 100 and 0 <= n('maximumBasementRH') <= 100
    assert 0 <= n('timeAbove80RHHours') <= n('timeAbove70RHHours') + 1e-8 <= hours + 1e-8
    assert 0 <= n('timeBelowMinimumTemperatureHours') <= hours + 1e-8
    assert 0 <= n('dehumidifierRuntimeHours') <= n('dehumidifierCommandHours') + 1e-8 <= hours + 1e-8
    assert 0 <= n('ventilationRuntimeHours') <= hours + 1e-8
    assert n('referenceExtractionIsRated') == 0 and r['powerStatus'] == 'SIMULATION_ASSUMPTION'
    water = (1000 * (n('waterRemovedByVentilationL') + n('waterRemovedByDehumidifierL') + n('condensedLiters'))
             + n('vaporStorageChangeGram') - n('moistureIngressGPerHour') * hours)
    assert abs(water) < 1e-5, (r['runId'], 'water units/bookkeeping', water)
    device_energy = (n('dehumidifierRuntimeHours') * n('dehumidifierPowerW')
                     + (n('dehumidifierCommandHours') - n('dehumidifierRuntimeHours')) * n('standbyPowerW')) / 1000
    fan_energy = n('ventilationRuntimeHours') * n('ventilationPowerW') / 1000
    energy = device_energy + fan_energy - n('estimatedElectricityKWh')
    assert abs(energy) < 1e-7, (r['runId'], 'Wh/kWh/runtime accounting', energy)
    assert bool(n('energyIsPartial')) == (n('ventilationPowerW') == 0 and n('ventilationRuntimeHours') > 0)
    assert abs(n('massBalanceErrorGram')) < 1e-5 and abs(n('thermalBalanceErrorJ')) < 0.1
    max_water = max(max_water, abs(water))
    max_heat = max(max_heat, abs(n('thermalBalanceErrorJ')))
    max_energy = max(max_energy, abs(energy))

report = {'rows': len(rows), 'groups': dict(collections.Counter(r['group'] for r in rows)),
          'max_reconstructed_water_error_g': max_water, 'max_thermal_error_j': max_heat,
          'max_reconstructed_energy_error_kwh': max_energy, 'status': 'PASS',
          'note': 'Physical bookkeeping PASS does not imply RH target achieved or hardware calibration validated.'}
destination = args.output or args.summary.with_name('audit.json')
destination.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
print(json.dumps(report, indent=2))
