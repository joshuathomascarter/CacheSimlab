import yaml
import json


def load_config(path):
    """
    Load configuration from a JSON or YAML file.
    
    Args:
        path (str): Path to the config file (either .json or .yaml/.yml)
        
    Returns:
        dict: Configuration dictionary
    """
    if not path:
        raise ValueError("Config path cannot be empty")
        
    try:
        if path.endswith('.json'):
            with open(path, 'r') as f:
                return json.load(f)
        elif path.endswith(('.yaml', '.yml')):
            with open(path, 'r') as f:
                return yaml.safe_load(f)
        else:
            raise ValueError(f"Unsupported configuration file format: {path}")
            
    except FileNotFoundError:
        print(f"Error: Config file not found at {path}")
        return {}
    except Exception as e:
        print(f"Error loading config: {e}")
        return {}

def validate_config(config):
    """
    Validate the configuration dictionary against required fields.
    
    Args:
        config (dict): The loaded configuration
        
    Returns:
        bool: True if valid, raises ValueError otherwise
    """
    required_sections = ['l1_cache', 'dram']
    
    for section in required_sections:
        if section not in config:
            raise ValueError(f"Missing required configuration section: {section}")
            
    # Validate L1 Cache
    cache_keys = ['size_kb', 'block_size', 'associativity']
    for key in cache_keys:
        if key not in config['l1_cache']:
             raise ValueError(f"Missing L1 Cache parameter: {key}")

    # Validate DRAM
    dram_keys = ['banks', 'tRCD', 'tCAS', 'tRP', 'tRAS']
    for key in dram_keys:
        if key not in config['dram']:
             raise ValueError(f"Missing DRAM parameter: {key}")

    return True
