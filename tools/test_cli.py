"""Windows CLI/export smoke tests, including non-ASCII output paths."""
import csv
import json
from pathlib import Path
import subprocess
import sys
import tempfile

exe = Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='pince-test-') as folder:
    output = Path(folder) / 'árvíztűrő tükörfúrógép'
    result = subprocess.run([str(exe), '--scenario', 'summer', '--hours', '1', '--step', '60', '--out', str(output)], capture_output=True)
    assert result.returncode == 0, result.stderr
    with (output / 'summer.csv').open(encoding='utf-8', newline='') as f:
        rows = list(csv.DictReader(f))
    assert len(rows) == 61 and rows[0]['timestamp'] == '0' and rows[-1]['timestamp'] == '3600'
    assert any(r['dehumidifierCommand'] == '1' for r in rows)
    assert all(r['ventilationCommand'] == '0' for r in rows)
    assert all(0 <= float(r['basementRH']) <= 100 for r in rows)
    report = (output / 'report.html').read_text(encoding='utf-8')
    data = json.loads(report.split('const data=', 1)[1].split(';\nconst $=', 1)[0])
    assert len(data) == 1 and len(data[0]['rows']) == len(rows)
    for bad in [ ['--step', '0'], ['--hours', 'NaN'], ['--source', '-2'], ['--scenario', 'missing'], ['--hours', '2oops'] ]:
        failed = subprocess.run([str(exe), *bad, '--out', str(output)], capture_output=True)
        assert failed.returncode != 0, bad
    print('CLI: Unicode path, CSV schema, JSON report, actuator outputs and invalid arguments passed.')
