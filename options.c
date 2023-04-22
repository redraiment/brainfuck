#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

#include "options.h"

/**
 * Configurations for getopt_long.
 */
static struct option configs[] = {
  {"compile", no_argument, NULL, 'c'},
  {"representation", no_argument, NULL, 'r'},
  {"script", no_argument, NULL, 's'},
  {"enable-single-line-comment", no_argument, NULL, 'm'},
  {"output", required_argument, NULL, 'o'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {0, 0, 0, 0}
};

/**
 * Shared command line options.
 */
struct _Options options = {
  LinkMode,
  false,
  NULL,
  NULL,
  NULL,
};

/**
 * Free file name spaces.
 */
static void TearDownOptions(void) {
  if (options.object != NULL) {
    free(options.object);
    options.object = NULL;
  }
  if (options.output != NULL) {
    free(options.output);
    options.output = NULL;
  }
}

/**
 * Create file name: strip the source file's extension and append with suffix.
 */
static char* CopyFileName(char* source, bool withExtension, char* suffix) {
  char* begin = strrchr(source, '/');
  if (begin == NULL) {
    begin = source;
  }
  char* end = withExtension ? NULL : strrchr(begin, '.');
  if (end == NULL) {
    end = begin + strlen(begin);
  }

  int prefixLength = end - begin;
  int suffixLength = strlen(suffix);
  char* target = (char*)calloc(sizeof(char), prefixLength + suffixLength + 1);
  strncpy(target, begin, prefixLength);
  strncpy(target + prefixLength, suffix, suffixLength);

  return target;
}

/**
 * Show bug report and exit.
 */
static void BugReport(void) {
  fprintf(stderr, "Home page: <%s>.\n", PROJECT_HOMEPAEG_URL);
  fprintf(stderr, "E-mail bug reports to: <%s>.\n", PROJECT_BUG_REPORT);
  exit(EXIT_FAILURE);
}

/**
 * Show help and exit.
 */
static void Help(void) {
  fprintf(stderr, "Overview: brainfuck compiler and interpreter.\n\n");

  fprintf(stderr, "Usage: %s [OPTIONS] <source-file>\n\n", PROJECT_NAME);

  fprintf(stderr, "  It will create an executable file without options.\n\n");

  fprintf(stderr, "OPTIONS:\n\n");

  fprintf(stderr, "  -c/--compile\n\n");
  fprintf(stderr, "    Only run preprocess, compile and assemble steps, then emit native object (.o) to output file.\n\n");
  fprintf(stderr, "    By default, the object file name for a source file is made by replacing the extension with .o.\n\n");

  fprintf(stderr, "  -r/--representation\n\n");
  fprintf(stderr, "    Emit LLVM representation (.ll) to standard output.\n\n");

  fprintf(stderr, "  -s/--script\n\n");
  fprintf(stderr, "    Run source file as Brainfuck script.\n\n");

  fprintf(stderr, "  -m/--enable-single-line-comment\n\n");
  fprintf(stderr, "    Enable single line comment command `#`.\n\n");
  fprintf(stderr, "    It's useful used with Shebang.\n\n");

  fprintf(stderr, "  -o/--output <output-file>\n\n");
  fprintf(stderr, "    Write output to file.\n\n");
  fprintf(stderr, "    This applies to whatever sort of output is being produced, whether it be an executable file, an object file, an IR file.\n\n");
  fprintf(stderr, "    If -o is not specified, the default executable file name for a source file is made by removing the extension.\n\n");

  fprintf(stderr, "  -h/--help\n\n");
  fprintf(stderr, "    Show this help and exit.\n\n");

  fprintf(stderr, "EXAMPLES:\n\n");

  fprintf(stderr, "  1. Creating an executable file:\n\n");
  fprintf(stderr, "    brainfuck helloworld.bf\n\n");

  fprintf(stderr, "  2. Running a file as scripting:\n\n");
  fprintf(stderr, "    brainfuck -s helloworld.bf\n\n");

  fprintf(stderr, "  3. Using with Shebang:\n\n");
  fprintf(stderr, "    #!/usr/local/bin/brainfuck -ms\n\n");

  fprintf(stderr, "  4. Creating native object file:\n\n");
  fprintf(stderr, "    brainfuck -c helloworld.bf\n\n");

  fprintf(stderr, "  5. Creating LLVM representation file:\n\n");
  fprintf(stderr, "    brainfuck -r helloworld.bf\n\n");

  BugReport();
}

/**
 * Show version and exit.
 */
void Version(void) {
  fprintf(stderr, "%s v%s\n\n", PROJECT_NAME, PROJECT_VERSION);

  BugReport();
}

/**
 * Parse command line arguments with getopt_long and return parsed options.
 */
void ParseCommandLineArguments(int argc, char* argv[]) {
  atexit(TearDownOptions);

  while (true) {
    int index = 0;
    int charactor = getopt_long(argc, argv, "crsmo:hv", configs, &index);
    if (charactor < 0) {
      break;
    }

    switch (charactor) {
    case 'c':
      options.mode = CompileMode;
      break;
    case 'r':
      options.mode = RepresentationMode;
      break;
    case 's':
      options.mode = ScriptingMode;
      break;
    case 'm':
      options.singleLineCommentEnabled = 1;
      break;
    case 'o':
      options.output = optarg;
      break;
    case 'v':
      Version();
    default:
      Help();
      break;
    }
  }

  // Only accept one source file.
  if (optind + 1 == argc) {
    options.source = argv[optind];
  } else {
    Help();
  }

  options.object = CopyFileName(options.source, false, ".o");
  if (options.output == NULL) {
    if (options.mode == CompileMode) {
      options.output = CopyFileName(options.source, false, ".o");
    } else if (options.mode == LinkMode) {
      options.output = CopyFileName(options.source, false, "");
    }
  } else {
    // Clone a copy
    options.output = CopyFileName(options.output, false, "");
  }
}
