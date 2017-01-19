#include "util.h"
#include <stdlib.h>
#include <limits.h>

IntParseResult parse_int(char* str) {
    char* endptr = "hold";
    long int large = strtol(str, &endptr, 10);

    if (large > INT_MAX || large < INT_MIN) {
        return (IntParseResult)err(TOO_LARGE);
    }

    if (endptr[0] != '\0') {
        return (IntParseResult)err(INVALID_STRING);
    }

    return (IntParseResult)ok((int) large);
}