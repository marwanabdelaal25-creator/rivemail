#!/usr/bin/env python3
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

def run(cmd):
    print('+', ' '.join(cmd))
    subprocess.check_call(cmd, cwd=ROOT)

if __name__ == '__main__':
    run(['make', 'test'])
    run(['python3', 'scripts/audit_duplicate_bodies.py', 'rivemail'])
    print('local verification completed')
