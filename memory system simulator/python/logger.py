import logging
import datetime

class SimLogger:
    def __init__(self, log_file=None):
        """
        Initialize the logger.
        
        Args:
            log_file (str, optional): If provided, logs will be written to this file.
        """
        self.logger = logging.getLogger("SimLogger")
        self.logger.setLevel(logging.INFO)
        self.logger.handlers = [] # Clear existing handlers

        formatter = logging.Formatter(
            '%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

        # 1. Console Handler
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(formatter)
        self.logger.addHandler(console_handler)

        # 2. File Handler (Optional)
        if log_file:
            file_handler = logging.FileHandler(log_file)
            file_handler.setFormatter(formatter)
            self.logger.addHandler(file_handler)

    def info(self, message):
        self.logger.info(message)
        
    def error(self, message):
        self.logger.error(message)
        
    def warning(self, message):
        self.logger.warning(message)
