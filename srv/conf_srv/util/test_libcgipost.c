
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // for unlink()
#include "libcgipost.h"

char * TMP1 = "/tmp/zrm.test1.tmp";


int test1()
{

    FILE * fd;
    int i;
    int checks_passed = 0;

    // From https://stackoverflow.com/questions/59659968/how-to-get-key-values-from-http-post-request-using-c-and-boost-asio
    char * samp1 =  "POST /cgi-bin/process.cgi HTTP/1.1\r\n"
                    "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
                    "Host: www.tutorialspoint.com\r\n"
                    "Content-Type: application/x-www-form-urlencoded\r\n"
                    "Content-Length: length\r\n"
                    "Accept-Language: en-us\r\n"
                    "Accept-Encoding: gzip, deflate\r\n"
                    "Connection: Keep-Alive\r\n"
                    "\r\n"
                    "licenseID=string1&content=string2&/paramsXML=string3\r\n";


    fd = fopen(TMP1, "w");
    fwrite(samp1, 1, strlen(samp1), fd);
    fclose(fd);

    // Create cgi-post-parse structures
    char * wanted_fields[3] = { "licenseID", "content", "paramsXML" };
    char value_array[3][100] = { "x", "x", "x" };
    char * value_dest[3] = { value_array[0], value_array[1], value_array[2] };
    size_t max_value_sizes[3] = { 100, 100, 100 };

    fd = fopen(TMP1, "r");
    int ret = receive_and_process_post(fd, 3, wanted_fields, value_dest, max_value_sizes);
    fclose(fd);
    unlink(TMP1);

    printf("%s: munch_post parsed %d values (want 3)\n", ret == 3 ? "YAY" : "BOO", ret);
    if (ret == 3) ++checks_passed;

    printf("     Values gotten:\n");
    printf("     -------------\n");
    for (i=0; i<3; i++) {
        printf(" %10s %s\n", wanted_fields[i], value_dest[i]);
    }
    // Checks
    if (strcmp(value_dest[0], "string1") == 0) ++checks_passed;
    if (strcmp(value_dest[1], "string2") == 0) ++checks_passed;
    if (strcmp(value_dest[2], "string3") == 0) ++checks_passed;

    if (checks_passed == 4) {
        printf("All checks passed: Test1 OK\n");
        return 1;
    } else {
        printf("%d/4 checks passed: Test1 FAILED\n", checks_passed);
        return 0;
    }
}

int test2()
{

    FILE * fd;
    int i;
    int checks_passed = 0;
    // From https://developer.mozilla.org/en-US/docs/Learn/Forms/Sending_and_retrieving_form_data
    char * samp2 = 
        "POST / HTTP/2.0\r\n"
        "Host: foo.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "say=Hi&to=Mom\r\n";

    fd = fopen(TMP1, "w");
    fwrite(samp2, 1, strlen(samp2), fd);
    fclose(fd);

    // Create cgi-post-parse structures
    char * wanted_fields[3] = { "say", "to" };
    char value_array[2][100] = { "x", "x" };
    char * value_dest[2] = { value_array[0], value_array[1] };
    size_t max_value_sizes[2] = { 100, 100 };

    fd = fopen(TMP1, "r");
    int ret = receive_and_process_post(fd, 2, wanted_fields, value_dest, max_value_sizes);
    fclose(fd);
    unlink(TMP1);

    printf("%s: munch_post parsed %d values (want 2)\n", ret == 2 ? "YAY" : "BOO", ret);
    if (ret == 2) ++checks_passed;

    printf("     Values gotten:\n");
    printf("     -------------\n");
    for (i=0; i<2; i++) {
        printf(" %10s %s\n", wanted_fields[i], value_dest[i]);
    }
    // Checks
    if (strcmp(value_dest[0], "Hi") == 0) ++checks_passed;
    if (strcmp(value_dest[1], "Mom") == 0) ++checks_passed;

    if (checks_passed == 3) {
        printf("All checks passed: Test2 OK\n");
        return 1;
    } else {
        printf("%d/3 checks passed: Test2 FAILED\n", checks_passed);
        return 0;
    }
}

int test3()
{
    char *tests[5][2] = {
        {"hello+hello+hello", "hello hello hello"},
        {"http%3A%2F%2ffoo+bar%2fabcd", "http://foo bar/abcd"},
        {"google.com/search?q=%60Abdu%27l-Bah", "google.com/search?q=`Abdu'l-Bah"},
        {"diestart%21yes%40", "diestart!yes@"},
        {"xx1%21,2%40,3%23,4%24,5%25,6%5e,7%26,8%2A,9%28,0%29xx", "xx1!,2@,3#,4$,5%,6^,7&,8*,9(,0)xx"},
    };
    int i;
    char testbuffer[100];
    int checks_passed = 0;

    for (i=0; i<5; i++) {
        strcpy(testbuffer, tests[i][0]);
        uri_decode_string(testbuffer);
        if (strcmp(testbuffer, tests[i][1]) == 0) {
            ++checks_passed;
        } else {
            printf("Uhoh: Wanted %s -> %s but got %s\n",
                tests[i][0], tests[i][1], testbuffer);
        }
    }
    if (checks_passed == 5) {
        printf("All checks passed: Test3 OK\n");
        return 1;
    } else {
        printf("%d/5 checks passed: Test3 FAILED\n", checks_passed);
        return 0;
    }
}

int main()
{
    test1();
    test2();
    test3();
    return 0;
}
