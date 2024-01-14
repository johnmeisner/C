
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // for unlink()
#include "libtimefgets.h"

char * TMP1 = "/tmp/zrm.test1.tmp";
extern int timefgets_timeout_delay_ms;
extern int timefgets_timeout_delay_secs;


void test1()
{
    FILE * fd;
    char buffer[1000];
    int bytes_read;

    fd = fopen("/etc/passwd","r");

    bytes_read = timefgets(buffer, sizeof(buffer), fd, 50);

    fclose(fd);

    if (bytes_read > 1) {
        printf("Read %d bytes from /etc/passwd (want > 1)\n", bytes_read);
    }
}

void test2()
{
    FILE * fd;
    char buffer[1000];
    int bytes_read;
    time_t start_time, delta_time;
    int done = 0, cycles = 0;

    fd = fopen("/dev/random","r");      // This is slow

    while (!done) {
        ++cycles;
        start_time = time(0L);
        bytes_read = timefgets(buffer, sizeof(buffer), fd, 5000);     // 5 sec timeout
        delta_time = time(0L) - start_time;
        // Might need a second or third reading for random's buffer is cleared
        //    and then it will slow down to generate new rando chars.   If we
        //    are on a weird system where it doesn't slow down, exit after 10 cycles
        if (cycles > 10 || delta_time > 1) {
            done = delta_time > 1; // We're only done if we had to wait for new ones to be generated
        }
    }

    fclose(fd);

    printf("Read %d bytes in %ld secs from /dev/random (want < 1000 bytes, 4 or 5 secs)\n", bytes_read, delta_time);
}

void test3()
{
    FILE * fd;
    char buffer[1000];
    int bytes_read;

    fd = fopen("/dev/null","r");      // This is slow

    bytes_read = timefgets(buffer, sizeof(buffer), fd, 5000);     // 5 sec timeout

    fclose(fd);

    printf("Read %d bytes from /dev/null (want -1 bytes)\n", bytes_read);

}

void test4()
{
    FILE * fd;
    char buffer[1000];
    int bytes_read;
    time_t start_time, delta_time;

    fd = fopen("/dev/urandom","r");      // This is fast

    start_time = time(0L);
    bytes_read = timefgets(buffer, sizeof(buffer), fd, 50);     // 50 ms timeout
    delta_time = time(0L) - start_time;

    fclose(fd);

    printf("Read %d bytes in %ld secs from /dev/urandom (want < 1000 bytes, 0 secs)\n", bytes_read, delta_time);
}

// Need to be root to attach to 
void test5()
{
    FILE * fd;
    char buffer[1000];
    int bytes_read;
    time_t start_time, delta_time;

    fd = fopen("/dev/console","r");      // This is slow, should be zero bytes, but must be root
    if (!fd) {
        printf("Skipping test 5 as you need to be root to open /dev/console\n");
        return;
    }

    start_time = time(0L);
    bytes_read = timefgets(buffer, sizeof(buffer), fd, 5000);     // 5 sec timeout
    delta_time = time(0L) - start_time;

    fclose(fd);

    printf("Read %d bytes in %ld secs from /dev/console (want 0 bytes, 4 or 5 secs)\n", bytes_read, delta_time);
}

// Test end-of-file from a process
void test6()
{
    FILE * fd;
    char buffer[1000];
    int bytes_read;
    int cnt_empty_reads=0, cnt_content_reads=0, cnt_eof_reads=0;
    time_t start_time, delta_time;
    int i;

    fd = popen("vmstat 2 2","r");

    for (i=0; i<100; i++) {
        start_time = time(0L);
        bytes_read = timefgets(buffer, sizeof(buffer), fd, 50);     // 50 ms timeout
        delta_time = time(0L) - start_time;
        if (bytes_read > 0) { ++cnt_content_reads; }
        if (bytes_read == 0) { ++cnt_empty_reads; }
        if (bytes_read < 0) { ++cnt_eof_reads; }
        if (bytes_read >=0 && cnt_eof_reads > 0) {
            printf("WARNING: timefgets # %d returned bytes=%d after previous EOF read, should be EOF always!\n",
                i, bytes_read);
        }
    }
    pclose(fd);

    printf("After %d timefgets reads, got %d content reads, %d empty reads, %d EOF reads (want 4 content, many empties, more of EOFs, and no warnings about empties after EOF)\n",
        i, cnt_content_reads, cnt_empty_reads, cnt_eof_reads);
}



int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    return 0;
}
