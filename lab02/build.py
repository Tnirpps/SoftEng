"""
Build script for the whole MaDisk monorepo.
Usage:
    python build.py bootstrap       - Install dependencies with Conan
    python build.py configure       - Configure CMake build directory
    python build.py build           - Build everything
    python build.py test            - Run all unit tests (if built)
    python build.py lint            - Run clang-format on all source files
    python build.py docker-build    - Build Docker images for all services
    python build.py docker-up       - Start all services with docker-compose
    python build.py docker-down     - Stop all services
    python build.py start <service> - Run a selected service (user_service or directory_service)
    python build.py all             - bootstrap + configure + build
"""
import subprocess
import sys
import os
from pathlib import Path

ROOT_DIR = Path(__file__).parent
BUILD_DIR = ROOT_DIR / "build" / "Release"

def run(cmd, desc):
    print(f"\n==> {desc}\n{' '.join(map(str,cmd))}\n")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"❌ {desc} failed")
        sys.exit(result.returncode)
    print(f"✅ {desc} completed\n")

def bootstrap():
    run(["conan", "install", ".", "--build=missing", "-o", "userver/*:with_clickhouse=False",
         "-pr:b=deps", "-pr:h=app"], "Installing Conan dependencies")

def configure():
    run(["cmake", "--preset", "conan-release"], "Configuring CMake")

def build():
    run(["cmake", "--build", "--preset", "conan-release"], "Building all")

def test():
    # поиск всех бинарников *_unittest
    for ut in BUILD_DIR.rglob("*_unittest"):
        run([str(ut)], f"Running tests: {ut.name}")

def lint() -> bool:
    """Run clang-format on all C++ source files in the project."""
    # Find all C++ files in user_service and directory_service
    cpp_files = []
    hpp_files = []
    
    for service_dir in [ROOT_DIR / "user_service", ROOT_DIR / "directory_service"]:
        src_dir = service_dir / "src"
        if src_dir.exists():
            cpp_files.extend(src_dir.rglob("*.cpp"))
            hpp_files.extend(src_dir.rglob("*.hpp"))
        # Also check tests
        tests_dir = service_dir / "tests"
        if tests_dir.exists():
            cpp_files.extend(tests_dir.rglob("*.cpp"))
            hpp_files.extend(tests_dir.rglob("*.hpp"))
    
    if not cpp_files and not hpp_files:
        print("No C++ source files found!")
        return False
    
    all_files = [str(f) for f in cpp_files + hpp_files]
    
    return run_command(
        ["clang-format", "-i"] + all_files,
        f"Running clang-format on {len(all_files)} files"
    )


def run_command(cmd, desc) -> bool:
    """Run a command and return True if successful."""
    print(f"\n==> {desc}\n{' '.join(map(str, cmd))}\n")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"❌ {desc} failed")
        return False
    print(f"✅ {desc} completed\n")
    return True


def start_service(service_name: str):
    binary = BUILD_DIR / service_name / service_name
    config = ROOT_DIR / service_name / "configs" / "static_config.yaml"
    config_vars = ROOT_DIR / service_name / "configs" / "config_vars.testing.yaml"
    if not binary.exists():
        print(f"❌ {binary} not found, please build first")
        sys.exit(1)
    run([str(binary), "-c", str(config), "--config_vars", str(config_vars)],
        f"Starting {service_name}")

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(0)

    cmd = sys.argv[1]

    if cmd == "bootstrap": bootstrap()
    elif cmd == "configure": configure()
    elif cmd == "build": build()
    elif cmd == "test": test()
    elif cmd == "lint":
        success = lint()
        sys.exit(0 if success else 1)
    elif cmd == "docker-build":
        run(["docker-compose", "build"], "Building Docker images")
    elif cmd == "docker-up":
        run(["docker-compose", "up", "-d"], "Starting all services")
    elif cmd == "docker-down":
        run(["docker-compose", "down"], "Stopping all services")
    elif cmd == "start" and len(sys.argv) >= 3:
        start_service(sys.argv[2])
    elif cmd == "all":
        bootstrap(); configure(); build()
    else:
        print(__doc__)
        sys.exit(1)

if __name__ == "__main__":
    os.chdir(ROOT_DIR)
    main()
