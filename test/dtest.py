#!/usr/bin/env python3

import os, sys
import shlex
import subprocess
import toml

VERSION = '0.02'

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class Test:
    def __init__(self,
                 name: str,
                 description: str,
                 program_path: str,
                 expect_returncode: int,
                 skipped: bool,
                 complete: bool):
        self.name = name
        self.description = description
        self.program_path = program_path
        self.expect_returncode = expect_returncode
        self.skipped = skipped
        self.complete = complete

def get_test_cfg(p):
    name = p[:-2] # remove '.c'
    description = None
    program_path = os.path.join(tests_path, p)
    expect_returncode = 0
    skipped = False
    f = open(os.path.join(tests_path, p), "r")

    for line in f:
        tok = shlex.split(line)
        if len(tok) >= 2 and tok[0] == "//!dtest":
            if tok[1] == "skip":
                skipped = True

            if tok[1] == "description":
                description = tok[2]

            if tok[1] == "expect":
                #ignore tok[2] / test type for time being
                expect_returncode = int(tok[3])
        else:
            break

    complete = not (name is None or description is None or program_path is None)
    
    return Test(name, description, program_path, expect_returncode, skipped, complete)

# ------

fails = 0
skips = 0
passes = 0

os.chdir(sys.path[0])

conf = toml.load("dtest_cfg.toml")
projinfo = conf['project']
tests_path = os.path.abspath(projinfo['tests_path'])

print(bcolors.BOLD + f"Testing {projinfo['name']}..." + bcolors.ENDC)
print()

tests = [get_test_cfg(f) for f in sorted(os.listdir(tests_path)) if os.path.isfile(os.path.join(tests_path, f))]

for test in tests:
    if test.skipped:
        print(bcolors.BOLD + bcolors.OKCYAN + f"[SKIP] Skipping test {test.name}" + bcolors.ENDC)
        skips += 1
        continue

    if not test.complete:
        print(bcolors.WARNING + bcolors.BOLD + f"[FAIL] Incomplete test description for {test.program_path}" + bcolors.ENDC)
        fails += 1
        continue

    print(f"{'Test: ' + test.name:<40}", end='')

    command = projinfo['exec_prep'].replace(
        "[SOURCE]", test.program_path)

    compile = subprocess.run(command, shell=True)
    if compile.returncode:
        print(bcolors.WARNING + bcolors.BOLD + '[FAIL]' + bcolors.ENDC)
        print(f'Error: Failed compiling test {test.name}!')
        fails += 1
        continue

    result = subprocess.run(projinfo['exec'])
    if result.returncode == test.expect_returncode:
        print(bcolors.OKCYAN + bcolors.BOLD + '[PASS]' + bcolors.ENDC)
        passes += 1

    else:
        print(bcolors.WARNING + bcolors.BOLD + '[FAIL]' + bcolors.ENDC)
        print(f"\tretcode was {result.returncode}, expect was {test.expect_returncode}")
        fails += 1
        continue

print('')

if skips:
    print(bcolors.BOLD + f"-- {skips} tests skipped. --" + bcolors.ENDC)

print(bcolors.OKBLUE + bcolors.BOLD + '-- TESTING COMPLETE --' + bcolors.ENDC)

print('')

if fails:
    print(bcolors.FAIL + bcolors.BOLD + f"[ SUITE FAIL ]: {passes} tests passed, {fails} tests failed." + bcolors.ENDC)
    exit(-9)
else:
    print(bcolors.OKGREEN + bcolors.BOLD + "[ SUITE PASS ]: All tests passed!" + bcolors.ENDC)
    subprocess.run(projinfo['run_after'], shell=True)
    exit(0)

