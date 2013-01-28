#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "sha1.h"

uint8_t input[20]  = {0};
uint8_t output[20] = {0};

void increment_input() {
    int i, n;
    input[0]++;
    if (input[0] == 0) {
        n = 1;
        while (input[n] == 255)
            n++;
        input[n]++;
        for(i = 0; i < n; i++)
            input[i] = 0;
    }
}

void print_input() {
    size_t i;
    for(i = 19; i-- > 0; )
        printf("%02x", input[i]);
    printf("\n");
}

void handle_signal(int sig) {
    print_input();
    if (sig == SIGTERM)
        exit(0);
}

int main(int argc, char const *argv[]) {
    signal(SIGTERM, handle_signal);
    signal(SIGUSR1, handle_signal);

    do {
        increment_input();
        sha1_buffer((char*)input, 0, output);
    } while (memcmp(output, input, 20) != 0);

    print_input();

    return 0;
}
