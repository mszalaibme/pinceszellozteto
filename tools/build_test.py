"""Portable CMake build/CTest runner; no third-party Python dependencies."""
import argparse
import os
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--cmake', default='cmake')
parser.add_argument('--config', default='Debug')
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
build = root / 'build'
env = {k.upper(): v for k, v in os.environ.items()} if os.name == 'nt' else dict(os.environ)
ctest = Path(args.cmake).with_name('ctest.exe' if os.name == 'nt' else 'ctest') if Path(args.cmake).parent != Path('.') else 'ctest'
for command in [
    [args.cmake, '-S', str(root), '-B', str(build)],
    [args.cmake, '--build', str(build), '--config', args.config],
    [str(ctest), '--test-dir', str(build), '-C', args.config, '--output-on-failure', '-V'],
]:
    subprocess.run(command, check=True, env=env)
