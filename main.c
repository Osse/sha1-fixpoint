#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "sha1.h"

#define REPORT_EVERY 1000000
#define TRUE 1
#define FALSE 0

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

double calc_converage_ratio() {
    double sha = 0;
    double exp = 1;
    for (uint8_t i = 0; i < 20; i++) {
        sha += input[i] * exp;
        exp *= 256.0;
    }
    return sha / exp;
}

void print_input(uint8_t appendNewline) {
    for(uint8_t i = 20; i-- > 0;)
        printf("%02x", input[i]);
    if (appendNewline)
        printf("\n");
}

void print_report() {
    printf("Trying SHA ");
    print_input(FALSE);
    printf("; covered %.40lf%% of search space", calc_converage_ratio()*100);
    //Todo: calc and display ETA
    printf("\n");
}

void handle_signal(int sig) {
    print_input(TRUE);
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

    print_input(TRUE);

    return 0;
}
