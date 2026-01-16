#!/usr/bin/env python3

import subprocess
import sys
from config_loader import load_config, validate_config

def main():
    # Default config file
    config_file = 'config.json'

    if len(sys.argv) > 1:
        config_file = sys.argv[1]

    print(f"Loading configuration from {config_file}...")

    try:
        # Load and validate config
        config = load_config(config_file)
        validate_config(config)

        # Extract config values
        l1 = config['l1_cache']
        dram = config['dram']

        # Prepare command line arguments for C++ program
        args = [
            './memory_sim',
            str(l1['size_kb']),
            str(l1['block_size']),
            str(l1['associativity']),
            str(dram['banks']),
            str(dram['tRCD']),
            str(dram['tCAS']),
            str(dram['tRP']),
            str(dram['tRAS'])
        ]

        print("Configuration loaded successfully:")
        print(f"  L1 Cache: {l1['size_kb']}KB, {l1['block_size']}B blocks, {l1['associativity']}-way")
        print(f"  DRAM: {dram['banks']} banks, tRCD={dram['tRCD']}, tCAS={dram['tCAS']}, tRP={dram['tRP']}, tRAS={dram['tRAS']}")
        print("Starting memory simulator...")

        # Run the C++ simulator
        subprocess.run(args)

    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()