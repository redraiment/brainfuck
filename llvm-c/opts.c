#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

#include "opts.h"

static struct option configs[] = {
  {"compile", no_argument, NULL, 'c'},
  {"script", no_argument, NULL, 's'},
  {"preprocess", no_argument, NULL, 'p'},
  {"output", required_argument, NULL, 'o'},
  {"help", no_argument, NULL, 'h'},
  {0, 0, 0, 0}
};

static struct _Options options = {
  ScriptingMode,
  NULL,
  NULL,
};

#define FILENAME_MAX_LENGTH 60

static char defaultObjectFileName[FILENAME_MAX_LENGTH + 3] = {0};

static char* CreateDefaultObjectFileName(char* sourceFileName) {
  char* begin = strrchr(sourceFileName, '/');
  if (begin == NULL) {
    begin = sourceFileName;
  }
  char* end = strrchr(begin, '.');
  if (end == NULL) {
    end = begin + strlen(begin);
  }

  int length = end - begin;
  if (length > FILENAME_MAX_LENGTH) {
    length = FILENAME_MAX_LENGTH;
  }

  strncpy(defaultObjectFileName, begin, length);
  defaultObjectFileName[length++] = '.';
  defaultObjectFileName[length++] = 'o';
  defaultObjectFileName[length++] = 0;

  return defaultObjectFileName;
}

/**
 * Show help and exit.
 */
static void help(void) {
  fprintf(stderr, "Overview: brainfuck interpreter and compiler.\n\n");

  fprintf(stderr, "Usage: brainfuck [OPTIONS] <source-file>\n\n");

  fprintf(stderr, "OPTIONS:\n\n");

  fprintf(stderr, "  -c/--compile\n");
  fprintf(stderr, "    Enable compile mode. Emit native object (.o).\n\n");

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
Options CommandLineOptions(int argc, char* argv[]) {
  while (1) {
    int index = 0;
    int charactor = getopt_long(argc, argv, "hcpso:", configs, &index);
    if (charactor < 0) {
      break;
    }

    switch (charactor) {
    case 'c':
      options.mode = CompileMode;
      break;
    case 'p':
      options.mode = PreprocessMode;
      break;
    case 's':
      options.mode = ScriptingMode;
      break;
    case 'o':
      options.output = optarg;
      break;
    default:
      help();
      break;
    }
  }

  if (optind + 1 == argc) {
    options.source = argv[optind];
  } else {
    help();
  }

  if (options.output == NULL && options.mode == CompileMode) {
    // Use default output name
    options.output = CreateDefaultObjectFileName(options.source);
  }

  return &options;
}
