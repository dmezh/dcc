#!/usr/bin/python3

import os
import subprocess
import toml

AUTHOR = 'dmezh'
VERSION = '0.01'

conf = toml.load("dtest_cfg.toml")
tests = conf['tests']
projinfo = conf['project']
tests_path = os.path.abspath(projinfo['tests_path'])

print(f"Testing {projinfo['name']}...")
print()

fails = 0

for t in tests:
    test = tests[t]
    print(f'Test: {t}')
    print(f"\tDescription: {test['desc']}")
    print(f"\tTesting: ", end="")
    type = test['type']
    if type == 'returncode': # only one type supported for now
        print(f"return code must be {test['expect']}")
        command = projinfo['exec_prep'].replace(
            "[SOURCE]", f"{tests_path}/{t}.c")
        print(f"\t\tRunning: {command}")
        compile = subprocess.run(command, shell=True)
        if compile.returncode:
            print('failed compiling test!')
            exit(-1)
        result = subprocess.run(projinfo['exec'])
        if result.returncode == test['expect']:
            print('\t\t[PASS]')
        else:
            print(f"retcode was {result.returncode}, expect was {test['expect']}")
            print('\t\t[FAIL]')
            fails += 1

if fails:
    print(f"{fails} tests failed!")
    exit(-9)
else:
    print("\nAll tests passed!\n[-----PASS-----]\n")
    subprocess.run(projinfo['run_after'], shell=True)
    exit(0)
