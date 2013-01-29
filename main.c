#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "sha1.h"

#define REPORT_EVERY 1000000

uint8_t input[20]  = {0};
uint8_t output[20] = {0};

void increment_input() {
    int n;
    input[0]++;
    if (input[0] == 0) {
        n = 1;
        while (input[n] == 255)
            n++;
        input[n]++;
        for(int i = 0; i < n; i++)
            input[i] = 0;
    }
}

void print_input() {
    for(size_t i = 19; i-- > 0; )
        printf("%02x", input[i]);
    printf("\n");
}

void print_report() {
    printf("Trying SHA ");
    for (uint8_t i = 0; i < 20; i++) {
        printf("%02x", input[19-i]);
    }
    printf("; covered %7.4f%% of search space", 0.0 /*Todo: write function to calc coverage percentage*/);
    //Todo: calc and display ETA
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

    uint32_t i = 0;
    do {
        increment_input();
        if (i++ >= REPORT_EVERY) {
            i = 0;
            print_report();
        }
        sha1_buffer((char*)input, 0, output);
    } while (memcmp(output, input, 20) != 0);

    print_input();

    return 0;
}
