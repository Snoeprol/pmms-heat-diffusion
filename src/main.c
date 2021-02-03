#include "compute.h"
#include "input.h"

int main(int argc, char **argv)
{
    struct parameters p;
    struct results r;

    read_parameters(&p, argc, argv);

    do_compute(&p, &r);

    return 0;
}
