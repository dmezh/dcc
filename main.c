#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#include "asmgen.h"
#include "debug.h"
#include "parser.tab.h"
#include "quads.h"
#include "util.h"

#define DCC_VERSION "0.2.0"
#define DCC_ARCHITECTURE "x86_32"

#define BRED "\033[1;31m"
#define RESET "\033[0m"

#define RED_ERROR(...) do {                         \
    fprintf(stderr, BRED);                          \
    fprintf(stderr, __VA_ARGS__);                   \
    fprintf(stderr, RESET "\n");                    \
    fprintf(stderr, "\nCompilation failed :(\n");   \
    exit(-1);                                       \
} while(0);

FILE* tmp;

static struct opt {
    int debug;
    bool asm_out;
    const char* out_file;
    const char* in_file;
} opt = {
    .debug = 0,
    .asm_out = false,
    .out_file = NULL,
};

static void print_usage_additional() {
    fprintf(stderr,
        "\n Pragmas:"
        "\n  Setting debug level:"
        "\n   #pragma dbg-info          set debug level going forward to INFO"
        "\n   #pragma dbg-verbose       set debug level going forward to VERBOSE"
        "\n   #pragma dbg-debug         set debug level going forward to DEBUG"
        "\n   #pragma dbg-none          set debug level going forward to NONE"
        "\n"
        "\n  Manual actions:"
        "\n   #pragma do-dumpsymtab     dump symbol table"
        "\n   #pragma do-examine var    examine identifier 'var'"
        "\n   #pragma do-perish         crash the compiler now"
        "\n");
}

static void print_usage() {
    fprintf(stderr, "Usage: ./dcc [OPTIONS] input_file"
        "\n Options:"
        "\n   -h              show extended usage"
        "\n   -o output_file  specify output file"
        "\n   -S              output assembly only"
        "\n   -v              debug mode:"
        "\n                         -v: enable INFO messages"
        "\n                        -vv: enable VERBOSE messages"
        "\n                       -vvv: enable DEBUG messages"
        "\n"
        "\n                     Note that this overrides any in-source directives."
        "\n   -V              print version information"
        "\n");
}

static void get_options(int argc, char** argv) {
    int a;
    opterr = 0;
    while ((a = getopt(argc, argv, "hvVSo:")) != -1) {
        switch (a) {
            case 'h':
                print_usage();
                print_usage_additional();
                exit(0);
            case 'V':
                fprintf(stderr, "---- dcc - a C compiler ----"
                                "\n dcc " DCC_VERSION
                                "\n architecture: " DCC_ARCHITECTURE "\n"
                                "\n author: Dan Mezhiborsky"
                                "\n home: https://github.com/dmezh/dcc\n");
                exit(0);
            case 'v':;
                opt.debug++;
                break;
            case 'S':
                opt.asm_out = true;
                break;
            case 'o':
                opt.out_file = optarg;
                break;
            case '?':
                print_usage();
                RED_ERROR("\nUnknown option '%c'", optopt);
            default:
                die("unreachable");
        }
    }

    switch (opt.debug) {
        case 0:
            break;
        case 1:
            debug_setlevel_INFO();
            break;
        case 2:
            debug_setlevel_VERBOSE();
            break;
        case 3:
        default:
            debug_setlevel_DEBUG();
    }

    if (!opt.out_file)
        opt.out_file = opt.asm_out ? "a.S" : "a.out";

    if (optind >= argc) {
        RED_ERROR("No input file specified!");
    }

    opt.in_file = argv[optind];
}

static FILE* new_tmpfile() {
    FILE *f = tmpfile();
    if (!f)
        RED_ERROR("Error allocating temporary file: %s", strerror(errno));

    return f;
}

static void preprocess() {
    FILE* f = new_tmpfile();

    switch (fork()) {
        case -1:
            RED_ERROR("Error forking for preprocessing: %s", strerror(errno));

        case 0:
            dup2(fileno(f), STDOUT_FILENO);

            const char* gcc_argv[] = {"gcc", "-E", opt.in_file, NULL};
            execvp(gcc_argv[0], (char**)gcc_argv);

            RED_ERROR("Error execing for preprocessing: %s", strerror(errno));

        default:;
            int status;
            wait(&status);
            if (status)
                RED_ERROR("Error during preprocessing");
            fseek(f, 0, SEEK_SET);
            dup2(fileno(f), STDIN_FILENO);
            break;
    }
}

static void assemble() {
    switch (fork()) {
        case -1:
            RED_ERROR("Error forking for assembly: %s", strerror(errno));

        case 0:
            dup2(fileno(tmp), STDIN_FILENO);

            const char* gcc_argv[] = {"gcc", "-x", "assembler", "-",
                                "-o", opt.out_file, "-m32", NULL};
            execvp(gcc_argv[0], (char**)gcc_argv);

            RED_ERROR("Error execing for assembly: %s", strerror(errno))

        default:;
            int status;
            wait(&status);
            if (status)
                RED_ERROR("Error during assembly");
    }
}

static void write_tmp_to_out() {
    FILE* out = fopen(opt.out_file, "w");
    if (!out) {
        RED_ERROR("Error opening output file: %s", strerror(errno));
    }

    char buf[512];
    size_t read;
    while ((read = fread(buf, 1, sizeof(buf), tmp)) > 0) {
        if (fwrite(buf, 1, read, out) != read) { // incomplete handling
            RED_ERROR("Error while writing output file");
        }
    }
    fclose(out);
}

int main(int argc, char** argv) {
    get_options(argc, argv);
    preprocess(); // now stdin is the preprocessed file

    // keep the assembly output in a tmpfile
    tmp = new_tmpfile();

    yydebug = 0;
    if (yyparse()) // <- entry to the rest of the compiler
        RED_ERROR("");

    fseek(tmp, 0, SEEK_SET);

    if (!opt.asm_out)
        assemble();
    else
        write_tmp_to_out();

    fclose(tmp);
}

void parse_done_cb(const BBL* root) {
    asmgen(root, tmp);
}
