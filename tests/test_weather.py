#!/usr/bin/env python3
"""Check the module's actual weather methods with a small native fixture."""
import argparse
from pathlib import Path
import subprocess
import tempfile


def block(source, marker):
    start = source.index(marker)
    opening = source.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


parser = argparse.ArgumentParser()
parser.add_argument('--operations-root', required=True, type=Path)
args = parser.parse_args()
ops = args.operations_root.resolve(strict=True)
subprocess.run([str(ops / 'scripts/wow-build-host-guard')], check=True)
work = ops / 'work'
work.mkdir(exist_ok=True)
if not work.resolve().is_relative_to(ops):
    raise SystemExit('Operations work directory must be inside the operations root')
module = Path(__file__).resolve().parents[1]
source = (module / 'src/core/mod_wv_core.cpp').read_text()
fixture = (module / 'tests/weather_fixture.cpp').read_text()
variants = {
    'candidate': source,
    'wrong-direction': source.replace('percentage = 1.0f - percentage;', 'percentage = 1.0f - (1.0f - percentage);'),
    'ignore-range': source.replace('return r.min + percentage * (r.max - r.min);', 'return r.min * 0.0f + percentage;'),
    'wrong-config-default': source.replace('ws == WEATHER_STATE_FINE ? Range{ 0.00f, 0.30f } : Range{ 0.30f, 1.00f }', 'Range{ 0.30f, 1.00f }'),
    'wrong-map-default': source.replace('state == WEATHER_STATE_FINE ? Range{ 0.00f, 0.30f } : Range{ 0.30f, 1.00f }', 'Range{ 0.30f, 1.00f }'),
    'nonfinite-range': source.replace(' && std::isfinite(a) && std::isfinite(b)', ''),
}
with tempfile.TemporaryDirectory(prefix='weather-fixture-', dir=work) as directory:
    for name, variant in variants.items():
        if name != 'candidate' and variant == source:
            raise SystemExit(f'{name}: mutation did not change source')
        body = '\n'.join(block(variant, marker) for marker in (
            'void WeatherVibeCore::LoadIntensityRangesConfig(',
            'float WeatherVibeCore::MapPercentToRawGrade(', 'Range WeatherVibeCore::ParseRangePair(',
            'bool WeatherVibeCore::PushWeatherPercent(', 'bool WeatherVibeCore::PushWeatherDebug('))
        cpp = Path(directory) / f'{name}.cpp'
        binary = Path(directory) / name
        cpp.write_text(fixture.replace('@BODY@', body))
        subprocess.run(['clang++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-pedantic', '-O0',
                        '-fsanitize=undefined,float-cast-overflow', '-fno-sanitize-recover=all',
                        str(cpp), '-o', str(binary)], check=True, timeout=60)
        result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        if (result.returncode == 0) != (name == 'candidate'):
            raise SystemExit(f'{name}: unexpected result\n{result.stdout}{result.stderr}')
        print(f'{name}: expected outcome ({result.stdout.strip() or result.stderr.strip()})')
