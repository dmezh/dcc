#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "debug.h"
#include "ir_print.h"
#include "parser.tab.h"
#include "util.h"

#define DCC_VERSION "1.0.2"
#define DCC_ARCHITECTURE "x86_64"

FILE *tmp, *tmp2;

static struct opt {
    int debug;
    bool asm_out;
    bool link;
    const char* out_file;
    const char* in_file;
} opt = {
    .debug = 0,
    .asm_out = false,
    .out_file = NULL,
    .link = 1,
};

static struct {
    struct utsname uname_data;
    bool is_darwin;
} host_info;

static void print_usage_additional(void) {
    eprintf(
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

static void print_usage(void) {
    eprintf("Usage: ./dcc [OPTIONS] input_file"
        "\n Options:"
        "\n   -h              show extended usage"
        "\n   -c              do not link"
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
    while ((a = getopt(argc, argv, "hvcVSo:")) != -1) {
        switch (a) {
            case 'h':
                print_usage();
                print_usage_additional();
                exit(0);
            case 'V':
                eprintf("---- dcc - a C compiler ----"
                        "\n dcc " DCC_VERSION
                        "\n architecture: " DCC_ARCHITECTURE " - on %s" "\n"
                        "\n author: Dan Mezhiborsky"
                        "\n home: https://github.com/dmezh/dcc\n",
                        host_info.uname_data.sysname);
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
            case 'c':
                opt.link = false;
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

    if (!opt.out_file) {
        if (opt.asm_out)
            opt.out_file = "out.ll";
        else
            opt.out_file = opt.link ? "out.out" : "a.o";
    }
        //opt.out_file = opt.asm_out ? "a.S" : "a.out";
        //opt.out_file = "out.IR";

    if (optind >= argc) {
        RED_ERROR("No input file specified!");
    }

    opt.in_file = argv[optind];
}

static FILE* new_tmpfile(void) {
    FILE *f = tmpfile();
    if (!f)
        RED_ERROR("Error allocating temporary file: %s", strerror(errno));

    return f;
}

static void preprocess(void) {
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
            if (WIFEXITED(status) && WEXITSTATUS(status)) {
                RED_ERROR("Error during preprocessing");
            }
            fseek(f, 0, SEEK_SET);
            dup2(fileno(f), STDIN_FILENO);
            break;
    }
}

static void assemble(void) {
    switch (fork()) {
        case -1:
            RED_ERROR("Error forking for assembly: %s", strerror(errno));

        case 0:
            dup2(fileno(tmp2), STDIN_FILENO);

            const char *link_cmd = opt.link ? "" : "-c";

            if (dcc_is_host_darwin()) {
                const char* gcc_argv[] = {"clang", "-x", "assembler", link_cmd, "-", "-o", opt.out_file, /*"-mmacosx-version-min=10.15",*/ "-arch", "x86_64", "-Og", NULL};
                execvp(gcc_argv[0], (char**)gcc_argv);
            } else {
                const char* gcc_argv[] = {"gcc", "-x", "assembler", link_cmd, "-fPIC", "-", "-o", opt.out_file, NULL};
                execvp(gcc_argv[0], (char**)gcc_argv);
            }

            RED_ERROR("Error execing for assembly: %s", strerror(errno))

        default:;
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status)) {
                RED_ERROR("Error during assembly");
            }
    }
}

static void llvm_convert(void) {
    tmp2 = new_tmpfile();

    switch(fork()) {
        case -1:
            RED_ERROR("Error forking for llcing: %s", strerror(errno));

        case 0:
            dup2(fileno(tmp), STDIN_FILENO);
            dup2(fileno(tmp2), STDOUT_FILENO);

            const char* llc_argv[] = {"llc", "--march", "x86-64", "-opaque-pointers", "-relocation-model=pic", "-", "-o", "-", NULL};
            execvp(llc_argv[0], (char**)llc_argv);

            RED_ERROR("Error execing for llcing: %s", strerror(errno));

        default:;
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status)) {
                RED_ERROR("Error during llcing");
            }

            fseek(tmp2, 0, SEEK_SET);
            dup2(fileno(tmp2), STDIN_FILENO);
    }
}

static void write_tmp_to_out(void) {
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
    if (uname(&host_info.uname_data))
        RED_ERROR("Error calling uname() for host information.");

    host_info.is_darwin = !strcmp(host_info.uname_data.sysname, "Darwin");

    get_options(argc, argv);
    preprocess(); // now stdin is the preprocessed file

    // keep the assembly output in a tmpfile
    tmp = new_tmpfile();

    yydebug = 0;
    if (yyparse()) // <- entry to the rest of the compiler
        RED_ERROR("\n");

    fseek(tmp, 0, SEEK_SET);

    if (!opt.asm_out) {
        llvm_convert();
        assemble();
    } else {
        write_tmp_to_out();
    }

    fclose(tmp);
    (void)assemble;
}

bool dcc_is_host_darwin(void) {
    return host_info.is_darwin;
}

void parse_done_cb(void) {
    fprintf(stderr, "Parse done!\n");
    quads_dump_llvm(stderr);
    quads_dump_llvm(tmp);
}
