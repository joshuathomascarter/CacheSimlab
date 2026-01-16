# Contributing to Cache-Sim

Thank you for your interest in contributing to Cache-Sim! This project is an educational collection of cache simulators and analysis tools.

## Getting Started

1. Fork the repository and clone it locally.
2. Set up the development environment:
   ```sh
   make venv
   make install-reqs
   ```
3. Build and test:
   ```sh
   make
   ```

## Code Style

- Use C++17 standard.
- Format code with clang-format: `make format`
- Follow consistent naming conventions (camelCase for functions, PascalCase for classes).

## Adding New Features

- Add tests for new functionality.
- Update documentation in README files.
- Ensure CI passes (runs `make` and format check).

## Reporting Issues

- Use GitHub Issues for bugs or feature requests.
- Include steps to reproduce and expected vs. actual behavior.

## Pull Requests

- Create a feature branch from `main`.
- Ensure all tests pass and code is formatted.
- Provide a clear description of changes.

For questions, contact the maintainer.