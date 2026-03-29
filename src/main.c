#include <stdio.h>
#include <stdlib.h>
#include "vm.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <bytecode_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    VM vm;
    vm_init(&vm);
    if (!vm_load_program(&vm, argv[1])) {
        fprintf(stderr, "Error loading program: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "MAIN: Program loaded successfully, calling vm_run\n");
    vm_run(&vm);
    vm_cleanup(&vm);

    return EXIT_SUCCESS;
}
