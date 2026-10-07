"""Run the portable SBUS regression suite with a host C++ compiler."""
import argparse
import shutil
import subprocess
import tempfile
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--compiler", default=shutil.which("g++"))
args = parser.parse_args()
if not args.compiler:
    parser.error("Install a C++ compiler or pass --compiler /path/to/g++")
root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="sbus-tests-") as tmp:
    executable = Path(tmp) / "sbus_test.exe"
    subprocess.run([args.compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    "-static", str(root / "test/sbus_test.cpp"), "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
