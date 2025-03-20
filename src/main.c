#include <stdlib.h>

#include "cpu.h"

int main(int argc, char *argv[])
{
    struct CPU cpu;
    cpu_reset(&cpu);

    while (1) {
        cpu_execute(&cpu);
    }

    return EXIT_SUCCESS;
}
