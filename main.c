#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include "sha1.h"

#define REPORT_EVERY 1000000
#define TRUE 1
#define FALSE 0

uint8_t input[20]  = {0};
uint8_t output[20] = {0};
uint8_t target[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

clock_t oldTicks;

void display_help() {
    printf("Help!\n"); //Todo: add actual help content
    exit(0);
}

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

void printETA() {
    clock_t newTicks = clock();
    double stepSecs = ((double)(newTicks-oldTicks))/CLOCKS_PER_SEC;
    double sha = 0;
    double exp = 1;
    for (uint8_t i = 0; i < 20; i++) {
        sha += (target[i] - input[i]) * exp;
        exp *= 256.0;
    }
    double etaYears = sha * stepSecs / REPORT_EVERY / 3600 / 24 / 365;
    printf("step: %.3lfs; ETA: %.3le years", stepSecs, etaYears);
    oldTicks = newTicks;
}

void print_sha(uint8_t appendNewline, const uint8_t sha[]) {
    for(uint8_t i = 20; i-- > 0;)
        printf("%02x", sha[i]);
    if (appendNewline)
        printf("\n");
}

void print_report() {
    printf("Trying SHA ");
    print_sha(FALSE, input);
    printf("; covered %.40lf%% of search space; ", calc_converage_ratio()*100);
    printETA();
    printf("\n");
}

void convert_string_to_sha(const char* instr, uint8_t outarr[]) {
    uint8_t idx = 0;
    int16_t i = strlen(instr);

    if (i > 40) { //sanity check (SHA1 is max 40 chars long)
        printf("Input is too long! (%i chars long, max 40 allowed)\n", i);
        exit(1);
    }

    char bufr[42], *pStart, *pEnd;
    strcpy(bufr, instr); //copy the input so we can manipulate it
    pStart = bufr + i; //start at the end of the string

    while (i >= 2 && idx < 20) { //while there's still two chars left
        pStart -= 2;    //move the offset two to the left
        i -= 2;
        outarr[idx] = (uint8_t)strtol(pStart, &pEnd, 16); //parse those two hex digits
        idx++;          //move up the array index
        *pStart = 0;    //write a bin 0 to the string to terminate the next group here
    }
    if (i == 1) { //if we got an uneven number of hex digits, we have a single digit left to parse
        pStart--; //move the offset pointer only one to the left
        i--;
        outarr[idx] = (uint8_t)strtol(pStart, &pEnd, 16); //parse the remaining digit
    }

    printf("Starting with SHA ");
    print_sha(FALSE, input);
    printf("; that means the first %.40lf%% of search space are already assumed covered\n", calc_converage_ratio()*100);
}

void handle_signal(int sig) {
    print_sha(TRUE, input);
    if (sig == SIGTERM)
        exit(0);
}

int main(int argc, char const *argv[]) {
    signal(SIGTERM, handle_signal);
    #ifndef _WIN32
    signal(SIGUSR1, handle_signal);
    #endif

    //process command line arguments (if any)
    for (uint8_t j = 1; j < argc; j++) {
        if (strcmp(argv[j], "-h") == 0)
            display_help();
        else if (strcmp(argv[j], "-s") == 0) {
            if (j < argc-1) {
                convert_string_to_sha(argv[j+1], input);
            }
        }
    }

    uint32_t i = 0;
    oldTicks = clock();
    do {
        increment_input();
        if (i++ >= REPORT_EVERY) {
            i = 0;
            print_report();
        }
        sha1_buffer((char*)input, 0, output);
    } while (memcmp(output, input, 20) != 0);

    print_sha(TRUE, input);

    return 0;
}
