
#include <stdio.h>  // for FILE

// Possible EOF and error codes for timefgets
#define TIMEFGETS_EOF -1
#define TIMEFGETS_SETUP_ERROR -11

int timefgets(char * buffer, size_t buff_size, FILE * stream, unsigned int timeout_msec);

