"""scripts/install_qt.py — installs Qt6 for local dev."""
import platform
import subprocess
import os
import sys

if platform.system() == "Darwin":
    subprocess.run(["brew", "install", "qt@6"], check=True)
elif "MSYSTEM" in os.environ:
    subprocess.run(
        ["pacman", "-S", "--noconfirm", "--needed",
         "mingw-w64-ucrt-x86_64-qt6-base",
         "mingw-w64-ucrt-x86_64-qt6-tools",
         "mingw-w64-ucrt-x86_64-qt6-charts"],
        check=True,
    )
else:
    sys.exit("Run on macOS, or on Windows from an MSYS2 UCRT64 shell.")
