/*
*  Filename: enable_route.c
*  Purpose: Utility to Execute Routing Rules
*
*  Copyright (C) 2019 DENSO International America, Inc.
*
*  The purpose of this utility is to ensure ONLY routing
*  rules are invoked from the customer file and not random
*  apps.
*
*  Revision History:
*
*  Date        Author        Comments
* --------------------------------------------------
*  12-12-19    JJG        Initial release.
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define RULEFILE   "/rwflash/customer/routing.rules"

/* just read in the routing.rules and only invoke route commands */

/**
* main()
*
* Main function
*
* returns 0 (success); -1 (failure)
*
*/
int main(int argc, char *argv[])
{ 
    char rulefile[200] = {0};    /* waste of space */
    char cmd[200] = {0};
    char *ptr, rule[400] = {0};   /* really? - a rule this long shouldn't exist */
    FILE *f;

    if (argc > 1) {
        /* cppcheck fix from strcpy to strncpy; good catch */
        strncpy(rulefile, argv[1], sizeof(rulefile));
    } else {
        /* cppcheck fix from sprintf to snprintf; good catch */
        snprintf(rulefile, sizeof(rulefile), RULEFILE);
    }

    sprintf(cmd, "[ -e %s ]", rulefile);
    if (system(cmd)) {
        /* rule file does not exist */
        printf("%s: ERROR: rule file does not exist (%s)\n", argv[0], rulefile);
        return -1;
    }

    if (NULL == (f = fopen(rulefile, "r"))) {
        /* error opening file */
        printf("%s: ERROR: unable to open rule file (%s)\n", argv[0], rulefile);
        return -1;
    }

    /* read in a line; if the line is longer than 400 bytes, holy cow */
    while (NULL != fgets(rule, sizeof(rule), f)) {
        /* remove '\n' if necessary */
        ptr = rule + strlen(rule) - 1;
        *ptr = '\n' ? 0 : *ptr;
        /* validate that it is a routing rule */
        sprintf(cmd, "echo '%s' | sed 's/^[[:space:]]*//' | grep \"^route \" >/dev/null", rule);
        if (!system(cmd)) {
            /* valid routing command, begins with command route */
            system(rule);
        }
    }
    fclose(f);

    return -1;
}

