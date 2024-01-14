/*
 *  Filename: get_toke_val.c
 *  Purpose: Test utility sending led command
 *
 *  Copyright (C) 2019 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  2019-07-10  LNGUYEN    Initial release.
 *  2021-01-28  KFRANKEL   Added -f option, reads the QUERYSTRING saved in a file; allows POST forms
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 65 /* Dupe from conf_manager.h */
#define MAX_QUERY_STRING 2100
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

char * USAGE = "USAGE: get_token_val [-f] filename|query_string token\n";
/* 
 * TODO: If this is cgi-bin calling then returning printf does what?
 *     : If we return an error code to cgi-bin does it know?
 */
int main (int argc, char **argv) {
    char buffer[MAX_QUERY_STRING];
    char *query_str;        /* query string */
    char *cfg_item;

    if (argc < 3 || (argc > 1 && (!strcmp(argv[1], "-h") || !strncmp(argv[1],"--h",3)))){
        printf("%s", USAGE);
        return 1;
    }
    if (! strcmp(argv[1], "-f")) {
        FILE * fin = fopen(argv[2], "r");
        if(NULL != fin) {
            if (!fgets(buffer, sizeof(buffer), fin)) {
                printf("ERROR:UnableToRead:%s\n", argv[2]);
                fclose(fin);
                return 3;
            }
            fclose(fin);
            query_str = buffer;
            cfg_item = argv[3];
        } else {
            printf("ERROR:UnableToOpen:%s\n", argv[2]);
            return 2;
        }
    } else {
        query_str = argv[1];
        cfg_item = argv[2];
    }
    if((0 == strlen(query_str)) || (MAX_QUERY_STRING < strlen(query_str))){
        printf("ERROR:BogusQueryString:check length: 1 to %u\n",MAX_QUERY_STRING);
        return 4;
    }
    if((0 == strlen(cfg_item)) || (MAX_TOKEN_SIZE < strlen(cfg_item))){
        printf("ERROR:BogusToeknString:check length: 1 to %u\n",MAX_TOKEN_SIZE);
        return 5;
    }
    /* char  array[] =  "MAP=on&SCS=on&SRM=on&IPB=on&AMH=on";
     * char *array =  query_str;
     */
    char *query = strdup (query_str); /* duplicate array, &array is not char** */
    if(NULL != query) {
        char *tokens = query;
        char *p = query;
        while ((p = strsep (&tokens, "&\n"))){
            char *var = strtok (p, "=");    /* grab var */
            char *val = NULL;
            if (0 == strcmp(var, cfg_item)){
                val = strtok (NULL, "=");   /* grab val */
                if (val){
                    implodeURIComponent(val); /* URI convert */
                    printf ("%s\n", val);
                }
                break;
            }
        }
        free (query);
    } else {
        printf("ERROR:UnableToDupe:System failure\n");
        return 6;
    }
    return 0;
}
