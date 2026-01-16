Cache Sim — Variants & Experiments
==================================

This folder contains multiple cache simulator variants (C++ implementations and Python analysis tools) used for teaching and experimentation.

Structure
---------
- `4-way cache/` — A 4-way set-associative cache project (C++ sources, tests, python trace/analysis scripts)
- `direct-way/` — A direct-mapped cache project (C++ sources and tests)

Which repo strategy?
---------------------
Keep everything together in this repo. Each variant is small and benefits from shared examples, traces, and analysis scripts. Use subfolder READMEs to describe how to build/run each project.

How to use
----------
1. Inspect the subfolder you want (for example `4-way cache/`) and follow that folder's README.
2. For C++ tests you can usually run:

```sh
# from the subfolder with the C++ files and tests
g++ -std=c++17 -I./c++ tests\ c++/test_set_associative.cpp c++/set_associative_cache.cpp -o test_cache
./test_cache
```

3. For Python analysis tools, create a venv and install the `requirements.txt` found in the analysis folder:

```sh
python -m venv .venv
source .venv/bin/activate  # macOS / Linux
python -m pip install -r "4-way cache/python/analysis/requirements.txt"
python "4-way cache/python/analysis/working_set.py"
```

Feedback
--------
If you plan to publish this on GitHub, decide whether you want to keep this as a monorepo or split into smaller repos. I'm happy to help create CI (GitHub Actions) and a `Makefile` to automate builds and tests.
