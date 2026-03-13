#!/usr/bin/env python3
"""
Build script for MaDisk user_service project.

Usage:
    python build.py bootstrap   - Install dependencies with Conan
    python build.py configure   - Configure CMake preset
    python build.py build       - Build the project
    python build.py lint        - Run clang-format on source files
    python build.py start       - Start the service
    python build.py test        - Run unit tests
    python build.py all         - Run all steps in order
"""

import subprocess
import sys
import os
from pathlib import Path


def run_command(command: list[str], description: str) -> bool:
    """Run a command and return True if successful."""
    print(f"\n{'='*60}")
    print(f">>> {description}")
    print(f">>> {' '.join(command)}")
    print(f"{'='*60}\n")
    
    result = subprocess.run(command)
    
    if result.returncode != 0:
        print(f"\n❌ Error: {description} failed!")
        return False
    
    print(f"\n✅ {description} completed successfully!")
    return True


def bootstrap() -> bool:
    """Install dependencies with Conan."""
    return run_command(
        ["conan", "install", ".", "-o", "userver/*:with_clickhouse=False"],
        "Installing dependencies with Conan"
    )


def configure() -> bool:
    """Configure CMake preset."""
    return run_command(
        ["cmake", "--preset", "conan-release"],
        "Configuring CMake preset"
    )


def build() -> bool:
    """Build the project."""
    return run_command(
        ["cmake", "--build", "--preset", "conan-release"],
        "Building project"
    )


def lint() -> bool:
    """Run clang-format on source files."""
    script_dir = Path(__file__).parent
    src_dir = script_dir / "src"
    
    # Find all C++ files
    cpp_files = list(src_dir.rglob("*.cpp"))
    hpp_files = list(src_dir.rglob("*.hpp"))
    
    if not cpp_files and not hpp_files:
        print("No C++ source files found!")
        return False
    
    all_files = [str(f) for f in cpp_files + hpp_files]
    
    return run_command(
        ["clang-format", "-i"] + all_files,
        f"Running clang-format on {len(all_files)} files"
    )


def start() -> bool:
    """Start the service."""
    script_dir = Path(__file__).parent
    binary = script_dir / "build" / "Release" / "MaDisk"
    config = script_dir / "configs" / "static_config.yaml"
    config_vars = script_dir / "configs" / "config_vars.testing.yaml"
    
    return run_command(
        [str(binary), "-c", str(config), "--config_vars", str(config_vars)],
        "Starting MaDisk service"
    )


def test() -> bool:
    """Run unit tests."""
    script_dir = Path(__file__).parent
    test_binary = script_dir / "build" / "Release" / "runtests-MaDisk"
    
    return run_command(
        [str(test_binary)],
        "Running unit tests"
    )


def print_usage():
    """Print usage information."""
    print(__doc__)


def main():
    if len(sys.argv) < 2:
        print_usage()
        sys.exit(1)
    
    command = sys.argv[1].lower()
    
    commands = {
        "bootstrap": bootstrap,
        "configure": configure,
        "build": build,
        "lint": lint,
        "start": start,
        "test": test,
        "all": lambda: all([bootstrap(), configure(), build(), lint()]),
    }
    
    if command not in commands:
        print(f"Unknown command: {command}")
        print_usage()
        sys.exit(1)
    
    # Change to script directory
    os.chdir(Path(__file__).parent)
    
    success = commands[command]()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
