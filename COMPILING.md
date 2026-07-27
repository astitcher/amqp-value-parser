# Quick build & usage (developer notes)

Status
- Experimental / work-in-progress. There is no packaged install artifact; this repo is intended for development and testing.

Minimum requirements
- CMake (3.25+ recommended)
- Apache Qpid Proton (0.35+) development files (headers & libs)
- A C toolchain (gcc/clang, make or ninja)
- lemon
- gperf
- re2c
- readline

Build (from a fresh clone)
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

If generator tools (lemon/gperf/re2c) are not available, either install them or provide the generated sources in the project root before configuring.

Example run (after build)
```bash
# parse a single value and show encoded frame
./amqp-value-test '["hello", 42, :symbol]'

# interactive frame inspection (readline) if no args
./frame-dump-tester
```

Troubleshooting
- "Proton not found": ensure Proton dev package is installed, or point CMake to Proton via CMAKE_PREFIX_PATH.
- Missing generator tools: install lemon, gperf, re2c, or check in generated files.

Notes
- Public API and generated headers may change while this repo is in WIP state.
