#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

#include "options.h"

/**
 * Configurations for getopt_long.
 */
static struct option configs[] = {
  {"preprocess", no_argument, NULL, 'p'},
  {"compile", no_argument, NULL, 'c'},
  {"link", no_argument, NULL, 'l'},
  {"script", no_argument, NULL, 's'},
  {"output", required_argument, NULL, 'o'},
  {"help", no_argument, NULL, 'h'},
  {0, 0, 0, 0}
};

/**
 * Static shared space for default parsed command line options.
 */
static struct _Options options = {
  ScriptingMode,
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
static char* CopyFileName(char* source, char* suffix) {
  char* begin = strrchr(source, '/');
  if (begin == NULL) {
    begin = source;
  }
  char* end = strrchr(begin, '.');
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
 * Show help and exit.
 */
static void Help(void) {
  fprintf(stderr, "Overview: brainfuck interpreter and compiler.\n\n");

  fprintf(stderr, "Usage: brainfuck [OPTIONS] <source-file>\n\n");

  fprintf(stderr, "OPTIONS:\n\n");

  fprintf(stderr, "  -c/--compile\n");
  fprintf(stderr, "    Enable compile mode. Emit native object (.o).\n\n");

  fprintf(stderr, "  -l/--link\n");
  fprintf(stderr, "    Enable link mode. Emit executable file.\n\n");

  fprintf(stderr, "  -p/--preprocess\n");
  fprintf(stderr, "    Enable preprocess mode. Emit LLVM representation (.ll).\n\n");

  fprintf(stderr, "  -s/--script\n");
  fprintf(stderr, "    [DEFAULT] Enable scripting mode. Parse and execute in memory.\n\n");
  
  fprintf(stderr, "  -o/--output <output-file>\n");
  fprintf(stderr, "    Write output to file.\n\n");

  fprintf(stderr, "  -h/--help\n");
  fprintf(stderr, "    Show this help and exit.\n\n");

  fprintf(stderr, "Home page: <https://github.com/redraiment/brainfuck/>.\n");
  fprintf(stderr, "E-mail bug reports to: <redraiment@gmail.com>.\n");
  exit(EXIT_FAILURE);
}

/**
 * Parse command line arguments with getopt_long and return parsed options.
 */
Options ParseCommandLineArguments(int argc, char* argv[]) {
  while (1) {
    int index = 0;
    int charactor = getopt_long(argc, argv, "hpclso:", configs, &index);
    if (charactor < 0) {
      break;
    }

    switch (charactor) {
    case 'p':
      options.mode = PreprocessMode;
      break;
    case 'c':
      options.mode = CompileMode;
      break;
    case 'l':
      options.mode = LinkMode;
      break;
    case 's':
      options.mode = ScriptingMode;
      break;
    case 'o':
      options.output = optarg;
      break;
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

  options.object = CopyFileName(options.source, ".o");
  if (options.output == NULL) {
    if (options.mode == CompileMode) {
      options.output = CopyFileName(options.source, ".o");
    } else if (options.mode == LinkMode) {
      options.output = CopyFileName(options.source, "");
    }
  } else {
    // Clone a copy
    options.output = CopyFileName(options.output, "");
  }

  return &options;
}
