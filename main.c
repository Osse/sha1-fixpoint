#include <inttypes.h>
#include <string.h>
#include "sha1.h"

int main(int argc, char const *argv[]) {
    char input[20]  = {0};
    char output[20] = {0};
    char result[20] =
        { 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55,
          0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09 };

    sha1_buffer(input, 20, output);

    if (memcmp(output, result, 40) == 0)
        return 0;
    else
        return 1;
}
