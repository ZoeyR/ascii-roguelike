#include <stdlib.h>
#include <limits.h>

#include <util/util.h>

Unit unit() {
    return (Unit){};
}
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

int better_rand(int limit) {
unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned long) limit + 1,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  long x;
  do {
   x = random();
  }
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x/bin_size;
}