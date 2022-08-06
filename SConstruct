import os

Decider('content-timestamp')

# Options
SetOption('silent', True)
SetOption('num_jobs', 8)

# Environment setup
env = Environment(ENV = {'PATH' : os.environ['PATH']})

env.Tool('compilation_db')
env.CompilationDatabase()

env['CC'] = 'clang'

term = os.environ.get('TERM')
if term is not None:
    env['ENV']['TERM'] = term

Export('env')

# SConscripts

SConscript('src/SConscript', variant_dir='build', duplicate=0)
SConscript('test/SConscript')
