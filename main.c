#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include "sha1.h"

#define TRUE 1
#define FALSE 0

//the SHA function wants these in byte-wise little-endian order (so that when
//  you print them from low to high indices, the output is the desired ASCII
//  output
uint8_t input[20]  = {0};
uint8_t output[20] = {0};
uint8_t target[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                      0xFF, 0xFF};

clock_t oldTicks;
uint32_t printReportEvery = 1000000;
uint32_t printHeaderEvery = 30;
int     gargc;
char const  **gargv;

void display_help(const char* progName) {
    printf("SHA1 Fixpoint calculation program\nWill try to find a value x for which x==sha1(x) holds\n");
    #ifdef GITVERSION
    printf("Version: %s\n", GITVERSION);
    #endif
    printf("\n");
    printf("Usage:\n    %s [-s <starting SHA>] [-t <target SHA>] [other options...]\n\n", progName);
    printf("Options:\n");
    printf("  -h            print this help and exit\n");
    printf("  -s <SHA>      starting SHA for this search\n");
    printf("  -t <SHA>      target SHA for this search (ending point)\n");
    printf("  -r <num>      print a report line every <num> SHAs (0 to disable)\n");
    printf("  -H <num>      print report header every <num> reports (0 to disable)\n");
    exit(0);
}

void increment_input() {
    for (int i = 0; i < 20; i++) {
        if (input[19-i] < 255) { //we can increment this digit
            input[19-i]++;
            break;
        } else { //digit overflow: set 0, go to next digit
            input[19-i]=0;
        }
    }
}

double calc_converage_ratio() {
    double sha = 0;
    double exp = 1;
    for (uint8_t i = 0; i < 20; i++) {
        sha += input[19-i] * exp;
        exp *= 256.0;
    }
    return sha / exp;
}

double calc_space_size_left() {
    double sha = 0;
    double exp = 1;
    for (uint8_t i = 0; i < 20; i++) {
        sha += (target[19-i] - input[19-i]) * exp;
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
        sha += (target[19-i] - input[19-i]) * exp;
        exp *= 256.0;
    }
    double etaYears = sha * stepSecs / printReportEvery / 3600 / 24 / 365;
    printf(" %6.3lfs | %.3le", stepSecs, etaYears);
    oldTicks = newTicks;
}

void print_sha(uint8_t appendNewline, const uint8_t sha[]) {
    for(uint8_t i = 0; i < 20; i++)
        printf("%02x", sha[i]);
    if (appendNewline)
        printf("\n");
}

void print_report_header(uint8_t sepAbove, uint8_t sepBelow) {
    if (sepAbove)
        printf("-------------------------------------------------------------------------------------------------------------\n");
    printf("             Last tried SHA              |          total search space coverage         | step len| ETA [yrs]\n");
    //      00000000000000000000000000000000001e8482 |  0.0000000000000000000000000000000000000001% |  0.530s | 2.456e+34
    if (sepBelow)
        printf("-------------------------------------------------------------------------------------------------------------\n");
}

void print_report() {
    print_sha(FALSE, input);
    printf(" | %43.40lf%% |", calc_converage_ratio()*100);
    printETA();
    printf("\n");
}

void convert_string_to_sha(const char* instr, uint8_t outarr[]) {
    uint8_t idx = 19;
    int16_t i = strlen(instr);

    if (i > 40) { //sanity check (SHA1 is max 40 chars long)
        printf("Input is too long! (%i chars long, max 40 allowed)\n", i);
        exit(1);
    }

    char bufr[42], *pStart, *pEnd;
    strcpy(bufr, instr); //copy the input so we can manipulate it
    pStart = bufr + i; //start at the end of the string

    while (i >= 2 && idx >= 0) { //while there's still two chars left
        pStart -= 2;    //move the offset two to the left
        i -= 2;
        outarr[idx] = (uint8_t)strtol(pStart, &pEnd, 16); //parse those two hex digits
        idx--;          //move up the array index
        *pStart = 0;    //write a bin 0 to the string to terminate the next group here
    }
    if (i == 1) { //if we got an uneven number of hex digits, we have a single digit left to parse
        pStart--; //move the offset pointer only one to the left
        i--;
        outarr[idx] = (uint8_t)strtol(pStart, &pEnd, 16); //parse the remaining digit
    }
}

void handle_signal(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        printf("\nExecution interrupted at SHA ");
        print_sha(FALSE, input);
        printf("; to continue at this SHA next time, you can use:\n  ");
        uint8_t printedStart = FALSE;
        for (int i = 0; i < gargc; i++) {
            if (strcmp(*(gargv+i), "-s") == 0 && i < gargc-1) {
                printedStart = TRUE;
                printf("-s ");
                print_sha(FALSE, input);
                printf(" ");
                i++;
            }
            else {
                printf("%s ", *(gargv+i));
            }
        }
        if (!printedStart) {
            printf("-s ");
            print_sha(FALSE, input);
        }
        printf("\n\n");
        exit(0);
    }
}

int main(int argc, char const *argv[]) {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    #ifndef _WIN32
    signal(SIGUSR1, handle_signal);
    #endif

    gargc = argc;
    gargv = argv;

    uint8_t nonDefaultTarget = FALSE;

    //process command line arguments (if any)
    for (uint8_t j = 1; j < argc; j++) {
        if (strcmp(argv[j], "-h") == 0)
            display_help(argv[0]);
        else if (strcmp(argv[j], "-s") == 0) {
            if (j < argc-1) {
                convert_string_to_sha(argv[++j], input);
            }
        }
        else if (strcmp(argv[j], "-t") == 0) {
            if (j < argc-1) {
                for(uint8_t i = 0; i < 20; i++)
                    target[i] = 0;
                convert_string_to_sha(argv[++j], target);
            }
        }
        else if (strcmp(argv[j], "-H") == 0) {
            if (j < argc-1) {
                printHeaderEvery = (uint32_t)atol(argv[++j]);
            }
        }
        else if (strcmp(argv[j], "-r") == 0) {
            if (j < argc-1) {
                printReportEvery = (uint32_t)atol(argv[++j]);
            }
        }
    }

    //check if target is non-default (less than all FFs)
    for(uint8_t i = 0; i < 20; i++)
        if (target[i] != 0xFF) {
            nonDefaultTarget = TRUE;
            break;
        }

    //Print starting data
    printf("Starting with SHA: ");
    print_sha(TRUE, input);
    printf("Target SHA:        ");
    print_sha(TRUE, target);
    printf("That means      %43.40lf%% of total search space are assumed covered\n", calc_converage_ratio()*100);
    if (nonDefaultTarget)
        printf("Considering     %43.40lf%% of total search space for this run\n", calc_space_size_left()*100);
    printf("Printing a report line every %u SHAs\n", printReportEvery);
    printf("\n");

    uint32_t i = 0;
    uint32_t hdrCtr = -1;
    oldTicks = clock();
    print_report_header(FALSE, TRUE);
    sha1_buffer((char*)input, 20, output); //calc sha of starting input value
    while (memcmp(output, input, 20) != 0) { //while input and output are different
        increment_input();
        if ((printReportEvery != 0) && (++i >= printReportEvery)) {
            if ((printHeaderEvery != 0) && (++hdrCtr >= printHeaderEvery)) {
                hdrCtr = 0;
                print_report_header(TRUE,TRUE);
            }
            i = 0;
            print_report();
        }
        sha1_buffer((char*)input, 20, output);
    }

    printf("Fixpoint found: ");
    print_sha(TRUE, input);

    return 0;
}
