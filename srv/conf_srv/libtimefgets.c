
#include <stdio.h>      // for fgetc() & EOF
#include <string.h>     // for memset()
#include <signal.h>     // for signals
#include <time.h>
#include <pthread.h>
#include "libtimefgets.h"   // for TIMEFGETS_* values

#define REASON_EOF 1
#define REASON_EOL 2
#define REASON_MAXSIZE 3
#define REASON_TIMEOUT 4

// Uncomment for debug output
// #define DEBUG_TIMEFGETS

struct timefgets_work_data {
    int bytes_read;
    size_t buffer_size;
    char * buffer;
    char eol_character;
    FILE * stream;
    int loop_exit_reason;
};

pthread_t timeReaderThreadID;

static void * timeReaderThreadFunc(void * arg)
{
    struct timefgets_work_data * data = (struct timefgets_work_data *) arg;
    char * cptr;
    int c;

    data->bytes_read = 0;
    cptr = data->buffer;

    while ((c = fgetc(data->stream)) != EOF) {
        if (data->eol_character != 0 && c == data->eol_character) {     // Done reading line
            data->loop_exit_reason = REASON_EOL;
            return NULL;
        }
        // Add to received string, unless its a null (serial-IO gives us nulls)
        if (c != 0) {
            *cptr++ = c;
            ++data->bytes_read;
            if (data->bytes_read >= data->buffer_size) {
                data->loop_exit_reason = REASON_MAXSIZE;
                return NULL;
            }
        }
    }
    data->loop_exit_reason = REASON_EOF;
    return NULL;
}


// Timeout handler - kills the reader thread
void callback_for_thread_timer1(int __attribute__((unused)) sig)
{
    pthread_cancel(timeReaderThreadID);
}


/*
** TIMEFGETS
**   A fgets() replacement that also has a timeout factor
**   Reads a single line out of passed stream, within the time limit, up to buff_size bytes
**   NOTE: The line-termination character (ie \n) is stripped off the end
**   Always null-terminates the string, the last character is truncated if necessary
**   Returns the number of bytes read, or zero for timeout, or -1 for EOF,
**       or -11 on any errors (errors in timer setup).
*/
int timefgets(char * buffer, size_t buff_size, FILE * stream, unsigned int timeout_msec)
{
    struct timefgets_work_data thread_data;
    struct itimerspec tv;
    timer_t timer1_id;

    thread_data.buffer_size = buff_size;
    thread_data.buffer = buffer;
    thread_data.stream = stream;
    thread_data.eol_character = '\n';
    thread_data.bytes_read = 0;
    thread_data.loop_exit_reason = REASON_TIMEOUT;

    // Create our timeout timer

    struct sigevent sige;
    memset(&sige, 0, sizeof(sige));
    sige.sigev_notify = SIGEV_THREAD;
    sige.sigev_notify_function = (void(*)(union sigval)) callback_for_thread_timer1;  // Casts handler's int into a sigval
    if (timer_create(CLOCK_REALTIME, &sige, &timer1_id) == -1) {
        fprintf(stderr, "Timer create failed\n");
        return TIMEFGETS_SETUP_ERROR;
    }
    memset(&tv, 0, sizeof(tv));

    tv.it_value.tv_sec = timeout_msec / 1000;
    tv.it_value.tv_nsec = (timeout_msec % 1000) * 1000000;       /* ms to ns */
    if (timer_settime(timer1_id, 0, &tv, NULL) == -1) {
        timer_delete(timer1_id);
        fprintf(stderr, "Timer settime failed\n");
        return TIMEFGETS_SETUP_ERROR;
    }

    // Start our worker thread
    if (pthread_create(&timeReaderThreadID, NULL, &timeReaderThreadFunc, &thread_data) != 0)
    {
        fprintf(stderr, "Thread creation failed\n");
        return TIMEFGETS_SETUP_ERROR;
    }

    // Wait for our worker thread to exit or timeout to kill it
    pthread_join(timeReaderThreadID, NULL);

    // Stop (in case reader thread exited before timer) and delete our timer
    memset(&tv, 0, sizeof(tv));
    timer_settime(timer1_id, 0, &tv, NULL);     // Note no fail check, fails if already timed out
    timer_delete(timer1_id);

    // Null terminate string we read to be nice to the caller
    if (thread_data.bytes_read < buff_size - 1) {
        buffer[thread_data.bytes_read] = 0;
    // Hit end of buffer!  We'll sacrifice the last byte to guarantee a null-terminated string
    } else {
        buffer[buff_size-1] = 0;
        /* NOTE: We're leaving bytes_read==buff_size, which does create an inconsistency with
         *       the length of data in the buffer but it gives the caller a way of detecting
         *       if the input was truncated.  If you use the bytes_read value to manipulate
         *       buffer content, you must first detect truncation by testing to see if it
         *       equals buff_size and adjust by one (and also handle truncated input).
         */
    }

#ifdef DEBUG_TIMEFGETS
    // DEBUG FOR KENN 20210222
    fprintf(stderr, "timefgets done, read %d, exit reason %d (%d=EOF,%d=EOL,%d=MAX,%d=TIME)\n",
        thread_data.bytes_read, thread_data.loop_exit_reason,
        REASON_EOF, REASON_EOL, REASON_MAXSIZE, REASON_TIMEOUT);
#endif

    // If we hit EOF, return -1
    if (thread_data.bytes_read == 0 && thread_data.loop_exit_reason == REASON_EOF) {
        return TIMEFGETS_EOF;
    }

    // Return our results
    return thread_data.bytes_read;
}
