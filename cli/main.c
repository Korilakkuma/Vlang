#include "main.h"

int main(int argc, char **argv) {
  if ((argc != 2) && (argc != 3)) {
    fputs("./bin/x source [--code]\n", stderr);
    exit(EXIT_FAILURE);
  }

  open_file(argv[1]);

  if (compile()) {
    if ((argc == 3) && (strncmp(argv[2], "--code", 6) == 0)) {
      dump();

      return 0;
    }

    return execute();
  }

  return 1;
}
