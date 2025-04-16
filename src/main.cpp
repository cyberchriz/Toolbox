#include <ngrid.h>

int main() {
    NGrid A({10, 10});
    A.fill_random_uniform_int(0, 9);
    A.print("A:");
}
