import os

Decider('content-timestamp')

# Options
SetOption('silent', True)
SetOption('num_jobs', 8)

# Environment setup
env = Environment(ENV = {'PATH' : os.environ['PATH']},
                  COMPILATIONDB_USE_ABSPATH=True,
                  tools=["default", "compilation_db"])

env.CompilationDatabase('build/compile_commands.json')

env['CC'] = 'clang'

term = os.environ.get('TERM') # for color
if term is not None:
    env['ENV']['TERM'] = term

Export('env')

# SConscripts
SConscript('src/SConscript', variant_dir='build', duplicate=0)
SConscript('test/SConscript')
