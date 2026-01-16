from config_loader import load_config, validate_config
import json

def test():
    configs = ['config.json', 'high_perf_config.json']
    
    for config_path in configs:
        print(f"\n--- Testing Config Loading from {config_path} ---")
        try:
            # 1. Load the config
            config = load_config(config_path)
            print(f"Successfully loaded {config_path}")
            
            # 2. Validate the config
            is_valid = validate_config(config)
            if is_valid:
                print(f"[SUCCESS] {config_path} is valid.")
                
                # Show key differences
                l1_size = config['l1_cache']['size_kb']
                dram_banks = config['dram']['banks']
                print(f"Stats: L1 Size = {l1_size}KB, DRAM Banks = {dram_banks}")
                
        except Exception as e:
            print(f"[FAILURE] Testing failed for {config_path}: {e}")

if __name__ == "__main__":
    test()
