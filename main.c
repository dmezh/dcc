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
    bool debug;
    bool asm_out;
    const char* out_file;
    const char* in_file;
} opt = {
    .asm_out = false,
    .out_file = NULL,
};

static void get_options(int argc, char** argv) {
    int a;
    while ((a = getopt(argc, argv, "hvVSo:")) != -1) {
        switch (a) {
            case 'h':
                fprintf(stderr, "Usage: ./dcc [OPTIONS] input_file"
                                "\n Options:"
                                "\n   -h              show usage"
                                "\n   -o output_file  specify output file"
                                "\n   -S              output assembly only"
                                "\n   -V              debug mode"
                                "\n   -v              print version information\n");
                exit(0);
            case 'v':
                fprintf(stderr, "---- dcc - a C compiler ----"
                                "\n dcc " DCC_VERSION
                                "\n architecture: " DCC_ARCHITECTURE "\n"
                                "\n author: Dan Mezhiborsky"
                                "\n home: https://github.com/dmezh/dcc\n");
                exit(0);
            case 'V':
                opt.debug = true;
                break;
            case 'S':
                opt.asm_out = true;
                break;
            case 'o':
                opt.out_file = optarg;
                break;
            case '?':;
                RED_ERROR("Unknown option '%c'", optopt);
            default:
                die("unreachable");
        }
    }

    if (!opt.out_file)
        opt.out_file = opt.asm_out ? "a.S" : "a.out";

    if (optind >= argc) {
        RED_ERROR("No input file specified!");
    }

    opt.in_file = argv[optind];
}

static void preprocess() {
    int p[2];
    if (pipe(p)) {
        RED_ERROR("Error creating pipe for preprocessing: %s", strerror(errno));
    }

    switch (fork()) {
        case -1:
            RED_ERROR("Error forking for preprocessing: %s", strerror(errno));

        case 0:
            close(p[0]);
            dup2(p[1], STDOUT_FILENO);

            char* gcc_argv[] = {strdup("gcc"), strdup("-E"), strdup(opt.in_file), NULL};
            execvp(gcc_argv[0], gcc_argv);

            RED_ERROR("Error execing for preprocessing: %s", strerror(errno));

        default:
            close(p[1]);
            int status;
            wait(&status);
            if (status)
                RED_ERROR("Error during preprocessing");

            dup2(p[0], STDIN_FILENO);
            break;
    }
}

static void assemble() {
    switch (fork()) {
        case -1:
            RED_ERROR("Error forking for assembly: %s", strerror(errno));

        case 0:
            dup2(fileno(tmp), STDIN_FILENO);

            char* gcc_argv[] = {strdup("gcc"), strdup("-x"), strdup("assembler"), strdup("-"),
                                        strdup("-o"), strdup(opt.out_file), strdup("-m32"), NULL};
            execvp(gcc_argv[0], gcc_argv);

            RED_ERROR("Error execing for assembly: %s", strerror(errno))

        default:;
            int status;
            wait(&status);
            if (status)
                RED_ERROR("Error during preprocessing");
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
    tmp = tmpfile();

    yydebug = opt.debug;
    if (yyparse()) // <- entry to the rest of the compiler
        RED_ERROR("");

    fflush(tmp);
    fseek(tmp, 0, SEEK_SET);

    if (!opt.asm_out)
        assemble();
    else
        write_tmp_to_out();

    fclose(tmp);
}

void parse_done_cb(BBL* root) {
    asmgen(root, tmp);
}
