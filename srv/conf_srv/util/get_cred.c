/*
 *  Filename: get_cred.c
 *  Purpose: Extract username and password from query string.
 *           Decode URI format of the password
 *
 *  Copyright (C) 2019 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  07-10-19    LNGUYEN    Initial release.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * App to take a user and password URI format input and print out username or password
 */

#define implodeURIComponent(url) decodeURIComponent(url, url)


/*
 * ishex()
 * Check if int is hex
 */
inline int ishex(int x)
{
    return  (x >= '0' && x <= '9')  ||
        (x >= 'a' && x <= 'f')  ||
        (x >= 'A' && x <= 'F');
}

/*
 * decodeURIComponent()
 * 
 * Decode URI format strings
 */
int decodeURIComponent (char *sSource, char *sDest) {
    int nLength;
    for (nLength = 0; *sSource; nLength++) {
        if (*sSource == '%' && sSource[1] && sSource[2] && ishex(sSource[1]) && ishex(sSource[2])) {
            sSource[1] -= sSource[1] <= '9' ? '0' : (sSource[1] <= 'F' ? 'A' : 'a')-10;
            sSource[2] -= sSource[2] <= '9' ? '0' : (sSource[2] <= 'F' ? 'A' : 'a')-10;
            sDest[nLength] = 16 * sSource[1] + sSource[2];
            sSource += 3;
            continue;
        }
        sDest[nLength] = *sSource++;
    }
    sDest[nLength] = '\0';
    return nLength;
}

int main(int argc, char **argv)
{
    char *data;
    char username[32]={0};
    char password[32]={0};
    int  cred_option = 0;       // 1: UserName, 2: Password

    data = argv[1];

    if(data == NULL){
        printf("Error! no arg");
        return 1;
    }

    if (argc < 3){
        printf("Usage: get_cred <URI UserName&Password> <1:UserName, 2:Password>\n");
        exit(1);
    }

    // Error condition handling
    
    cred_option = atoi(argv[2]);
    
    /*
     * Given passing param - extract user named password
     * extract 1st param
     * extract 2nd param
     * string manipulation
     *      user: root
     *      password: 
     */

    char *tmp_data;

    /* extract username */
    tmp_data = strtok(data, "&");
    sscanf(tmp_data,"username=%s",username);

    /* extract password */
    tmp_data = strtok(NULL, "&");
    sscanf(tmp_data,"password=%s",password);

    /* Decode password URI special character */
    int nNewLength = implodeURIComponent(password);

    if (cred_option == 1) printf ("%s\n", username);
    if (cred_option == 2) printf ("%s\n", password);

    return 0;
}
