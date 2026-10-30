# Weather regression fixture

Run `python3 tests/test_weather.py --operations-root /path/to/wow-operations` on
the guarded WSL build host. The script compiles the actual weather method bodies
with small interface doubles, checks configured and fallback intensity ranges,
and verifies that five incorrect variants fail. Temporary outputs are created
under the operations repository's `work/` directory and removed after the run.
This does not establish a full server build or gameplay behavior.
