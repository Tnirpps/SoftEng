"""
Build script for the whole MaDisk monorepo.
Usage:
    python build.py bootstrap   - Install dependencies with Conan
    python build.py configure   - Configure CMake build directory
    python build.py build       - Build everything
    python build.py test        - Run all unit tests (if built)
    python build.py start <service> - Run a selected service (user_service or directory_service)
    python build.py all         - bootstrap + configure + build
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