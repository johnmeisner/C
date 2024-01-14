/*
*  Filename: cfgmgr.c  (compiles to utility "dcu")
*  Purpose: Configuration Manager
*
*  Copyright (C) 2021 DENSO International America, Inc.
*
*  dcu is a utility to manipulate the read-only config blobs and
*     to edit values within them.  Called by webgui and startup.sh.
*  dcu is a stateless utility; it is not a service.
*
*/
/* 20191017: TODO: cfgmgr is actually RSU specific; to use on OBU requires
   changes in the following:
   -- initialize -> IP handling assumes an HD ifc; OBU may not have that
   -- cfg files are assumed in /rwflash/configs; OBU uses /rwflash/configs/v2x
   -- possibly some other things that may need to be reviewed before OBU use
*/
// TODO: Replace all system(mount | grep ...) with bool_detect_if_mounted()
// TODO: Make our blobs *MUCH* more tamperproof by encrypting them!!  We
//       have openssl in our BSP already and it will encrypt/decrypt!
//       http://stackoverflow.com/questions/16056135/how-to-use-openssl-to-encrypt-decrypt-files
//       Create a /tmp file with crypt key and pass to openssl with -kfile, delete imm afterwardsd
// TODO: Randomize our mount points and TMP_CFG_BLOB names
// NOTE TO SELF: grep returns 0 if it finds something!

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "dn_types.h"
#include "cfgmgr.h"

#define DEFAULT_MNTPNT   "/tmp/zk_default_mntpnt"
#define IN_USE_MNTPNT    "/tmp/zk_inuse_mntpnt"
#define INIT_MNTPNT      "/tmp/zk_init_mntpnt"

#define PERM_CFG_BLOB       "/usr/lib/ihr"
#define ACTV_CFG_BLOB       "/rwflash/zk_active_config_blob"
#define TMP_CFG_BLOB        "/tmp/zk_tmp_config_blob"

// On Linux, grep for the blob not _MNTED so we make them the same:
#define TMP_CFG_BLOB_MNTD   TMP_CFG_BLOB
#define ACTV_CFG_BLOB_MNTD  ACTV_CFG_BLOB

#define RWFLCFGS      "/rwflash/configs"
#if 0
#define EMERGNCYBAK   "/rwflash/.bkcfgs"
#endif
#define DCU_JOURNAL_MAGIC_WORD  "MAGIC"
//
//track what user changes so when we re-install we preserve their values.
//Otherwise no way to know if what is in their confs are orig or changed.
//
#define DCU_JOURNAL_FILE      "/rwflash/dcu_journal.txt"
#define DEFAULT_VERSION_FILE  "/usr/share/BuildInfo.txt"
#define BLOB_VERSION_FILE     "BuildInfo.txt"
/* this directory needs to match the directory name found in <RSU or OBU>/configs/default/configs */
#define MERGE_DIR             "mergeconfigs"
#define PROGRESS_TMP_FILE     "/tmp/.ctf"

#define MAX_STRING     200
#define MAX_I_FILENAME 50
#define MAXIPSTR       100      /* Size of line buffer when reading an IP from a file */
#define MAXBUFFER      400      /* Size of line buffer when reading values from a file */

/* DO NOT SCREW WITH THIS -- unless you understand what's going on */
#define MERGE_CMD  "for i in `find " INIT_MNTPNT "/" MERGE_DIR "`; do if [[ ! -z \"`file $i | grep -v directory`\" && -e " RWFLCFGS "/`echo ${i#" INIT_MNTPNT "/" MERGE_DIR "/}` ]]; then for p in `cat $i`; do t=`echo ${i#" INIT_MNTPNT "/" MERGE_DIR "/}`; mp=`sed -n \"/^[[:space:]]*#*$p[[:space:]]/p\" " RWFLCFGS "/$t`; if [[ ! -z $mp ]]; then echo replacing $t $mp; sed -i \"s/^[[:space:]]*#*$p[[:space:]].*/$mp/\" " INIT_MNTPNT "/$t; fi; done; fi; done"

/* minimize debug prints; the goal is that this utility is not user friendly */
//K #define DBGPRINT(fmt, args...)  if (debugmode) printf(fmt, ##args)
// #define DBGPRINT(fmt, ...)  printf("DCU:%s:" fmt, __FUNCTION__, ##__VA_ARGS__)
#define DBGPRINT(fmt, ...)  mylog("DCU:%s:" fmt, __FUNCTION__, ##__VA_ARGS__)
//        ^-- NOTE: Prepends DCU and function name -- dont double-add this info into your log messages!


// A /tmp logger to help with cgi-bin debuggin
#include <stdarg.h>  // for va_list

//Do NOT output to stdout or you will clobber WEB GUI.
//If debugging something with shutdown in the process then send logs to rwflash.
static int mylog_output_selector = 0; // 0 = none 1= /tmp 2 = /rwflash
static void mylog(char * fmt, ...)
{
    static char buffer[4000];
    static va_list argptr;
    static int fd;

    if (mylog_output_selector) {
        va_start(argptr, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, argptr);
        va_end(argptr);
        if(2 == mylog_output_selector) { /* Preserve logs across reset. */
            fd = open("/rwflash/dcu_log.txt", O_CREAT|O_WRONLY|O_APPEND, 644);
        } else { /* Logs will not survive reset. */
            fd = open("/tmp/dcu_log.txt", O_CREAT|O_WRONLY|O_APPEND, 644);
        }
        if (fd >= 0) {
            write(fd, buffer, strlen(buffer));
            close(fd);
        }
#if 0 // Don't do it!
        if (mylog_output_selector > 1) {
            printf("%s", buffer);
        }
#endif
    }
}
  
enum {
    NOACTION,
    ACTION_BACKUP,
    ACTION_COMMIT,
    ACTION_GET,
    ACTION_INITIALIZE,
    ACTION_MANAGEMENT,
    ACTION_NETWORK,
    ACTION_OPTIONAL,
    ACTION_REVERT,
    ACTION_SET,
    ACTION_UNLOCK,
    ACTION_EXIT,
};

// Function to spell out action enums for debug
char * getActionString(int action)
{
    switch(action) {
        case NOACTION: return("NOACTION"); break;
        case ACTION_BACKUP: return("BACKUP"); break;
        case ACTION_COMMIT: return("COMMIT"); break;
        case ACTION_GET: return("GET"); break;
        case ACTION_INITIALIZE: return("INITIALIZE"); break;
        case ACTION_MANAGEMENT: return("MANAGEMENT"); break;
        case ACTION_NETWORK: return("NETWORK"); break;
        case ACTION_OPTIONAL: return("OPTIONAL"); break;
        case ACTION_REVERT: return("REVERT"); break;
        case ACTION_SET: return("SET"); break;
        case ACTION_UNLOCK: return("UNLOCK"); break;
        case ACTION_EXIT: return("EXIT"); break;
    }
    return "ILLEGAL_VALUE";
}

static int debugmode = 0;

/* notes on operation:
   - /rwflash/configs is now a link to the mounted files
   - cfgmgr only updates *.conf files
   - initialize copies the active blob to a temporary file that gets removed on commit
   - active cfg will get updated in cfg file, but this can be undone
*/


/* UTILITY FUNCTION - DETECT_IF_MOUNTED */
/* Returns TRUE (1) if mounted, FALSE (0) IF NOT DETECTED */
/* Note this is the opposite of all the shell-script-oriented tests with system() etc */
int bool_detect_if_mounted(char * name)
{
    char cmd[1000];
    int rc;

    snprintf(cmd,sizeof(cmd), "mount | grep '%s' >>/dev/null", name);
    rc = system(cmd);
    if (rc == 0) {
//        DBGPRINT("MountDetect: Detected %s is mounted\n", name);
        return 1; // TRUE
    } else {
//        DBGPRINT("MountDetect: Returning FALSE, %s is not mounted\n", name);
        return 0; // FALSE
    }
}

/* UTILITY FUNCTION - DETECT_IF_FILE_EXISTS */
/* Returns TRUE (1) if exists, FALSE (0) IF NOT DETECTED */
/* Note this is the opposite of all the shell-script-oriented tests with system() etc */
int bool_detect_file_exists(char * filepath)
{
    struct stat st;
    memset(&st, 0, sizeof(st));
    return (stat(filepath, &st) == 0);
}


/* UTILITY FUNCTION - REMOVE_TRAILING_SLASH
 * Makes a copy of passed path, and removes trailing slash if any
 * NOTE: Needs to make a copy because paths passed to detect_if_dir/slink
 *       are constants, segfault if modified
 */
char * remove_trailing_slash(char * filepath)
{
    static char ret_str[200];
    int pathlen;
    pathlen = strlen(filepath);
    if (pathlen == 0) return filepath;
    if (filepath[pathlen-1] != '/') {
        return filepath;
    }
    if (pathlen > sizeof(ret_str)) {
        DBGPRINT("NEED PROGRAMMING!  I can handle strings up to %u but got passed a %d-long string\n",
            sizeof(ret_str), pathlen);
        // Printing filepath separately so if this segfaults fprintf at least we got above warning
        DBGPRINT("NEED PROGRAMMING!  too-big filepath was \"%s\"\n", filepath);
        return filepath; // Can't chop, hopefully doesnt screw up our caller
    }
    
    /* Copy and chop */
    strncpy(ret_str, filepath, sizeof(ret_str));
    ret_str[pathlen-1] = 0;

    return ret_str;
}

/* UTILITY FUNCTION - DETECT_IF_IS_DIRECTORY
 * Returns TRUE (1) if exists and is a dir, FALSE (0) IF NOT
 * NOTE: this is the opposite of the shell-script-oriented tests with system() etc
 * NOTE: The IS_DIR(m) macro that comes with stat() doesn't seem to exist for yocto
 * NOTE: lstat() is needed to detect symlinks, not stat() !!
 * NOTE: A symlink to a directory will have S_IFDIR true in addition to S_IFLNK !!
 *           ^-- Actually this is NOT true for our embedded linux BSP
 */
int bool_detect_is_directory(char * filepath)
{
    int rc;
    struct stat st;
    memset(&st, 0, sizeof(st));
    if ((rc=lstat(remove_trailing_slash(filepath), &st)) != 0) {
        DBGPRINT("lstat returned %d (want 0) -- doesn't exist?\n", rc);
        return 0;   // FALSE - doesn't exist
    }
    printf("st_mode(%s) = %08x\n", filepath, st.st_mode);
    printf("   (S_IFLNK=%x S_IFDIR=%x)\n", S_IFLNK, S_IFDIR);
    // Test if we have a symlink.  We can't add testing if filepath is
    // also a dir (S_IFDIR) as symlinks to dirs dont get marked as a dir!
    // We will assume symlinks are dirs as we dont symlink files
    if (st.st_mode & S_IFLNK) {
        return 0;
    }
    // Return if filepath has the S_IFDIR bit set, marking it as a directory
    return ((st.st_mode & S_IFDIR) == S_IFDIR) != 0;
}


/* UTILITY FUNCTION - DETECT_IF_IS_SYMBOLIC_LINK
 * Returns TRUE (1) if exists and is a symlink, FALSE (0) IF NOT
 * NOTE: this is the opposite of the shell-script-oriented tests with system() etc
 * NOTE: lstat() is needed to detect symlinks, not stat() !!
 * NOTE: Must remove trailing slash or a symlink will detect as a directory
 */
int bool_detect_is_symlink(char * filepath)
{
    int rc;
    struct stat st;

    memset(&st, 0, sizeof(st));
    if ((rc=lstat(remove_trailing_slash(filepath), &st)) != 0) {
        DBGPRINT("lstat returned %d (want 0) -- doesn't exist?\n", rc);
        return 0;   // FALSE - doesn't exist
    }
    printf("st_mode(%s) = %08x\n", filepath, st.st_mode);
    printf("   (S_IFLNK=%x S_IFDIR=%x)\n", S_IFLNK, S_IFDIR);
    return ((st.st_mode & S_IFLNK) == S_IFLNK) != 0;
}


/* UTILITY function - Is config file a special no-equals format config file?
** Most config files are of format either 'key=value#comment' or 'key=value;min,max#comment'
** However, snmpd.conf is a different format, 'key value#comment' and this breaks all existing
** sed commands that pivot on the equals.  This test checks the config filename to see if
** it is a known key-value-noequals config file.  It is used in the various functions that
** call sed so they can adjust their expressions as needed.
*/
int bool_config_is_a_noequals_config_file(char * cfgfile)
{
    // Config file snmpd.conf needs to be handled differently as it doesn't use equals
    if (strlen(cfgfile) >= 10 && !strcmp(cfgfile + strlen(cfgfile) - 10, "snmpd.conf")) {
        return 1;   // TRUE
    }

    return 0;    // FALSE
}


/* UTILITY functions for interacting with shell script variables like a config file
 * GET: Gets value of what a variable is set to in script (startup.sh)
 * SET: Updates the value in the line where that values is set
 * These functions are called to update network_mode (dhcp or static), static ip, and netmask.
 * Returns 0 on success, nonzero on error
 */
int getScriptVariableValue(char *inputFile, char * setting, char *dest_value)
{
    FILE *fp;
    char *ptr, pcmd[400] = {0};
    char buffer[MAXBUFFER] = {0};

    DBGPRINT("Called to get %s from %s\n", setting, inputFile);

    /* confirm file exists */
    if (!bool_detect_file_exists(inputFile)) {
        DBGPRINT("Error: File %s doesnt exist, returning -1\n", inputFile);
        return -1;
    }

    // This command greps out the setting line with or without spaces between
    // setting and its equals.  The sed commands allow us to capture all the text
    // after the first equals sign, including other equals signs in the
    // comments, so we can handle a commented second equals sign without
    // losing the value (which we would if we didn't do the <EQUALS1> thing).
    // NOTE: We strip any leading spaces in the value to be consistent with the
    //       setScriptVariableValue, which strips all spaces between setting
    //       name and its equals.
    snprintf(pcmd, sizeof(pcmd),"grep '^%s *=' %s | sed -e 's/=/<EQUALS1>/' -e 's/.*<EQUALS1> *//'", setting, inputFile);
    DBGPRINT("The pcmd is: %s\n", pcmd);
    if (NULL != (fp = popen(pcmd, "r"))) {
        fgets(buffer, MAXBUFFER, fp);
        /* fgets includes the '\n' at the end, strip it */
        if (strlen(buffer) > 0) {
            ptr = buffer + strlen(buffer) - 1; 
            if (*ptr == '\n') {
                *ptr = 0;   /* NULL terminate there */
            }
        }
        DBGPRINT("The grep-sed retrieved %s's value as '%s'\n", setting, buffer);
        pclose(fp);
        /* Copy retrieved value back to caller */
        strcpy(dest_value, buffer);
    } else {
        DBGPRINT("system(grep) failed, returning -1\n");
        return -1;
    }

    return 0;
}
int setScriptVariableValue(char *inputFile, char * setting, char *newvalue)
{
    char cmd[400];
    char * badchar;
    int rc;

    DBGPRINT("setScriptVariableValue called with setting %s value '%s' in file %s\n", setting, newvalue, inputFile);

    /* confirm file exists */
    if (!bool_detect_file_exists(inputFile)) {
        DBGPRINT("Error: File %s doesnt exist, returning -1\n", inputFile);
        return -1;
    }

    // Our replacement edit method uses sed, so we cannot handle values
    //     with slashes and single quotes in them.  We must detect this
    //     and either sub them out (alternatively, fail the edit, or build
    //     a special sed string with backquotes for the slashes and quotes)
    if (index(newvalue, '/')) {
        DBGPRINT("WARNING: Setting %s new value \"%s\" includes a slash -- I cant handle that, removing it\n", setting, newvalue);
        while((badchar = index(newvalue, '/'))) {
            *badchar = ' ';
        }
    }
    if (index(newvalue, '\'')) {
        DBGPRINT("WARNING: Setting %s new value \"%s\" includes a single-quote -- I cant handle that, removing it\n", setting, newvalue);
        while((badchar = index(newvalue, '\''))) {
            *badchar = ' ';
        }
    }

    // Update setting to new value with sed
    // NOTE: This sed command updates the setting line with or without spaces
    // between setting and its equals.
    // NOTE: The "change-to" value has no spaces around the equals as this is
    // required for shell variables, such as in startup.sh.  It is also
    // compaible with the xyz.conf config files.
    snprintf(cmd,sizeof(cmd), "sed -i -e 's/^%s *=.*/%s=%s/' %s", setting, setting, newvalue, inputFile);
    DBGPRINT("Update system() cmd is: %s\n", cmd);
    rc = system(cmd);
    DBGPRINT("sed call to update %s returned %d (want 0)\n", setting, rc);
    if (rc != 0) {
        DBGPRINT("ERROR: Update to %s failed, sed call returned %d (want 0)\n", setting, rc);
        return -2;
    }

    /* All good! */
    return 0;
}

/* Copies a config value from one config file to another
 * Returns 0 for success or non-zero for errors
 * NOTE: This is works on both script variable and config file settings,
 * by tweaking the getSVV to allow optional spaces between setting and the
 * equals (config file format), and by writing the update in startup.sh
 * format (no spaces before or after equals).
 */
int copy_config_value(char * src_config_file, char * dest_config_file, char * setting)
{
    char buffer[MAXBUFFER];
    int rc;
    rc = getScriptVariableValue(src_config_file, setting, buffer);
    if (rc != 0) return rc;
    if (strlen(buffer) == 0) {
        DBGPRINT("ERROR: No value for setting %s to copy, skipping this setting value copy\n", setting);
        return -8;  // No value to copy
    }
    rc = setScriptVariableValue(dest_config_file, setting, buffer);
#ifdef WANT_VERIFY_AFTER_WRITE
    // Verify we did actually update that setting
    if (rc != 0) return rc;
    char buffer2[MAXBUFFER];
    rc = getScriptVariableValue(dest_config_file, setting, buffer2);
    if (rc != 0) return rc;
    if (strcmp(buffer, buffer2)) {
        DBGPRINT("ERROR: Wanted to set %s:%s to %s but verify retrieved %s instead!\n", dest_config_file, setting, buffer, buffer2);
        return -9;
    } else {
        DBGPRINT("Verified we set %s:%s to %s\n", dest_config_file, setting, buffer);
    }
#endif
    return rc;
}

/*
 * Detects if /mnt/rwflash is mounted.
 *   NOTE: There's an autofs systemd-1 mount of /rwflash that
 *         is *NOT* what we're looking for, so we have a second
 *         grep /dev to focus on the actual device mount of rwflash
 */
int bool_is_rwflash_mounted()
{
    int rc;

    rc = system("mount | grep rwflash | grep /dev >>/dev/null");
    if (rc == 0) {
        return 1; // TRUE
    } else {
        return 0; // FALSE
    }
}

/* Retrieve startup.sh's DHCP_ENABLE setting
 * Returns the value, with 1 (true) or 0 (false), -1 on failure
 */
int getDHCPEnableSetting(char *inputFile)
{
    char buffer[MAXBUFFER];
    int rc;
    rc = getScriptVariableValue(inputFile, "DHCP_ENABLE", buffer);
    if (rc != 0) return rc;
    // Validate retrieved value, but using strncmp for length as value may have a comment with it
    if (!strncmp(buffer,"TRUE", 4)) {
        return 1;
    } else if (!strncmp(buffer,"FALSE", 5)) {
        return 0;
    } else {
        DBGPRINT("WARNING: Retrieved invalid boolean \"%s\" for DHCP_ENABLE setting\n", buffer);
        return -1;
    }
}
/*
 * Get & Set functions for DHCP_ENABLE, StaticIP, and Netmask
 * Returns 0 on success, nonzero on error
 */
int setDHCPEnableSetting(char *inputFile, int newvalue)
{
    if (newvalue) {
        return setScriptVariableValue(inputFile, "DHCP_ENABLE", "TRUE");
    } else {
        return setScriptVariableValue(inputFile, "DHCP_ENABLE", "FALSE");
    }
}
int getCfgIP(char *inputFile, char *staticIP)
{
    return getScriptVariableValue(inputFile, "IP_FOR_STATIC_ADDRESS", staticIP);
}
int setCfgIP(char *inputFile, char * newvalue)
{
    return setScriptVariableValue(inputFile, "IP_FOR_STATIC_ADDRESS", newvalue);
}
int getNetmask(char *inputFile, char *dest_buffer)
{
    return getScriptVariableValue(inputFile, "NETMASK_FOR_STATIC_ADDRESS", dest_buffer);
}
int setNetmask(char *inputFile, char * newvalue)
{
    return setScriptVariableValue(inputFile, "NETMASK_FOR_STATIC_ADDRESS", newvalue);
}
int getVLANEnableSetting(char *inputFile)
{
    char buffer[MAXBUFFER];
    int rc;
    rc = getScriptVariableValue(inputFile, "VLAN_ENABLE", buffer);
    if (rc != 0) return rc;
    // Validate retrieved value, but using strncmp for length as value may have a comment with it
    if (!strncmp(buffer,"TRUE", 4)) {
        return 1;
    } else if (!strncmp(buffer,"FALSE", 5)) {
        return 0;
    } else {
        DBGPRINT("WARNING: Retrieved invalid boolean \"%s\" for VLAN_ENABLE setting\n", buffer);
        return -1;
    }
}
int setVLANEnableSetting(char *inputFile, int newvalue)
{
    if (newvalue) {
        return setScriptVariableValue(inputFile, "VLAN_ENABLE", "TRUE");
    } else {
        return setScriptVariableValue(inputFile, "VLAN_ENABLE", "FALSE");
    }
}





/*
 * cancelUpdate()
 *
 * Revert all changes; this is accomplished by deleting tmp blob
 *
 * returns 0 on success
 *
 */
static int cancelUpdate(void)
{
    /* check for mounted first */
    if (bool_detect_if_mounted(TMP_CFG_BLOB)) {
        if (!system("umount " DEFAULT_MNTPNT)) {
            remove(TMP_CFG_BLOB);    /* in theory should check return on this; may lock cfg updates */
            return 0;
        }
    }

    return -1;
}


/*
 * backupCustomerConfigs()
 *
 * create a backup of the current set of configs
 *
 * returns 0 on success
 *
 */
static int backupCustomerConfigs(void)
{
    int rc;
    char cmd[1000];
    sprintf(cmd, "cp " ACTV_CFG_BLOB " " CUST_CFG_BLOB);
    
    rc = system(cmd);
    if (0 == rc) {
        return 0;
    } else {
        DBGPRINT("Unable to backup customer configuration\n");    
        return -1;
    }
}


/*
 * resetCustomerConfigs()
 *
 * 
 * resets active configs using the customer configs backup
 *
 * steps: 
 * 1. Check that a customer backup exists
 * 2. Overwrite the current active blob with the customer backup
 *
 * returns 0 on success
 *
 */
static int resetCustomerConfigs(void)
{
    int rc;
    char cmd[1000];
    
    sprintf(cmd, "[[ -e " CUST_CFG_BLOB " ]]");
    rc = system(cmd);
    if (rc) {
        DBGPRINT("Error: No customer configuration backup found. \n");
        return -1;
    }
    
    sprintf(cmd, "cp " CUST_CFG_BLOB " " ACTV_CFG_BLOB);
    rc = system(cmd);
    if (0 == rc) {
        return 0;
    } else {
        DBGPRINT("Unable to restore customer configuration. \n");    
        return -1;
    }
}




/*
 * commit()
 *
 * Save all changes made to blob
 *
 * steps: unmount tmp; unmount in_use, copy tmp to actv, remount actv, delete tmp
 *
 * returns 0 on success
 *
 */
static int commit(void)
{
    int rc;
    /* First check that both TMP and ACTV are mounted; otherwise no changes to commit */
    if (bool_detect_if_mounted(TMP_CFG_BLOB) && bool_detect_if_mounted(ACTV_CFG_BLOB))
    {
        /*unmount, unmount tmp first; separate unmounts because 
          there may be corrective action to perform individually */
        rc = system("umount " DEFAULT_MNTPNT);
        DBGPRINT("system(\"umount " DEFAULT_MNTPNT " returned %d (want 0)\n", rc);
        if (rc) {
            /* unmount failed -- delete tmp cfg blob? */
            DBGPRINT("Deleting " TMP_CFG_BLOB " since umount failed\n");
            remove(TMP_CFG_BLOB);
            return -1;
        }
        rc = system("umount " IN_USE_MNTPNT);
        DBGPRINT("system(\"umount " IN_USE_MNTPNT " returned %d (want 0)\n", rc);
        if (!rc) {
            // Copy TMP to ACTV to commit to using it
            rc = system("cp " TMP_CFG_BLOB " " ACTV_CFG_BLOB);
            DBGPRINT("cp " TMP_CFG_BLOB " " ACTV_CFG_BLOB " returned %d (want 0)\n", rc);
            if (!rc) {
                rc = remove(TMP_CFG_BLOB);    /* in theory should check return on this; may lock cfg updates */
                DBGPRINT("remove(" TMP_CFG_BLOB ") returned %d (want 0)\n", rc);
                /* remount, read-only */
                rc = system("mount -r -tvfat " ACTV_CFG_BLOB " " IN_USE_MNTPNT);
                DBGPRINT("mount -r -tvfat " ACTV_CFG_BLOB " " IN_USE_MNTPNT " returned %d (want 0)\n", rc);
                if (!rc) {
#if 0
                    /* save the copy */
                    rc = system("cp -rf " RWFLCFGS "/* " EMERGNCYBAK);
                    DBGPRINT("cp -rf RWFLCFGS/* EMERGENCYBAK returned %d (want 0)\n", rc);
                    // Sync the changes to disk before we return.  This handles the case of
                    // a webgui user who updates a setting and powers down the RSU immediately
                    // after the page returns.  With no sync, or with the sync before the first
                    // unmount, the last webgui change is lost with the power-down.  The sync
                    // is only effective against abrupt power loss right here (likely anywhere
                    // after the cp would be good, but we can flush the EMERGENCYBAK copies
                    // as well down here).
                    rc = system("sync");
                    DBGPRINT("system(sync) returned %d (want 0)\n", rc);
                    if (rc) {
                        DBGPRINT("Commit sync failed! Update may not be written to disk!\n");
                        return -1;
                    }
#endif
                    return 0;
                }
            }
        }
    } else {
        DBGPRINT("WARNING: commit request ignored as not all blobs are mounted: TMP=%s ACTV=%s\n",
            bool_detect_if_mounted(TMP_CFG_BLOB) ? "mounted" : "NOT-mounted",
            bool_detect_if_mounted(ACTV_CFG_BLOB) ? "mounted" : "NOT-mounted");
    }

    return -1;
}

/*
 * get()
 *
 * get a value from the active configuration
 * active blob needs to be mounted
 * if not mounted; just get value from current cfg
 *
 * returns 0 on success
 *
 */
static int getConfigSettingValue(char *param, char *optionalFile)
{
    char cmd[400] = {0};   /* uneducated guess for max size */

    /* commands:

        mount | grep $TMP_CFG_BLOB
        ...if input optional file specified:
          cat $DEFAULT_MNTPNT/$optionalfile | grep $param | head -1 | sed 's/.*=\(.*\)/\1/' | sed 's/[[:space:]]//g' > $outfile
        ...if no input optional file, use listing files only:
          find $DEFAULT_MNTPNT -name '*.conf' | xargs grep $param | head -1 | sed 's/.*=\(.*\)/\1/' | sed 's/[[:space:]]//g' > $outfile

        then open $outfile for value
    */

    DBGPRINT("GET called on param %s\n", param);

    /* error checking; mandatory args */
    if (NULL == param) {
        DBGPRINT("a system error\n");
        return -1;
    }
    /* the following is only because I was burned some time ago by the compiler optimizing code to generate a null pointer; this could be merged below with no new lines */
    if (NULL != optionalFile) {
        if (!strlen(optionalFile)) optionalFile = NULL;
    }

    // If config is a key-value-noequals config, use a simple grep + awk
    if (bool_config_is_a_noequals_config_file(optionalFile)) {
        snprintf(cmd,sizeof(cmd), "grep ^%s " RWFLCFGS "/%s | awk '{print $2}'", param, optionalFile);
        DBGPRINT("conf file doesnt use equals. Going to use this cmd:\n");
        DBGPRINT("   %s\n", cmd);
        return system(cmd);
    }

// The output from system() is return to cfgmgrapi.c
// up to cfgmgrapi to error check beyond this.

    if (!system("mount | grep " TMP_CFG_BLOB_MNTD ">>/dev/null")) {
        if (NULL != optionalFile) {
            /* confirm file exists - this is the correct check on system output (opposite of expected) */
            snprintf(cmd,sizeof(cmd), "[[ -e " DEFAULT_MNTPNT "/%s ]]", optionalFile);
            if (system(cmd)) {
                /* file doesn't exist */
                return -1;
            }
            snprintf(cmd,sizeof(cmd), "cat " DEFAULT_MNTPNT "/%s| grep ^[[:space:]]*%s[[:space:]]|head -1|sed 's/[;#].*//'|sed 's/.*=\\(.*\\)/\\1/'|sed 's/[[:space:]]//g'", optionalFile, param);
            if (!system(cmd)) {
                return 0;
            }
        } else {
            snprintf(cmd,sizeof(cmd), "find " DEFAULT_MNTPNT " -name '*.conf' |grep -v " MERGE_DIR "| xargs grep ^[[:space:]]*%s[[:space:]]|head -1|sed 's/[;#].*//'|sed 's/.*=\\(.*\\)/\\1/'|sed 's/[[:space:]]//g'", param);
            if (!system(cmd)) {
                return 0;
            }
        }
    } else {
        /* actv cfg not mounted; just get value from current cfg */
        if (NULL != optionalFile) {
            /* confirm file exists */
            snprintf(cmd,sizeof(cmd), "[[ -e " RWFLCFGS "/%s ]]", optionalFile);
            if (system(cmd)) {
                /* file doesn't exist */
                return -1;
            }
            snprintf(cmd,sizeof(cmd), "cat " RWFLCFGS "/%s| grep ^[[:space:]]*%s[[:space:]]*=|head -1|sed 's/[;#].*//'|sed 's/.*=\\(.*\\)/\\1/'|sed 's/[[:space:]]//g'", optionalFile, param);
            if (!system(cmd)) {
                return 0;
            }
        } else {
            snprintf(cmd,sizeof(cmd), "find " RWFLCFGS "/ -name '*.conf' |grep -v " MERGE_DIR "| xargs grep ^[[:space:]]*%s[[:space:]]*=|head -1|sed 's/[;#].*//'|sed 's/.*=\\(.*\\)/\\1/'|sed 's/[[:space:]]//g'", param);
            if (!system(cmd)) {
                return 0;
            }
        }
    }

    return -1;
}

/*
 * processUpdatedCfg()
 *
 * Generates the blob file and merges
 * 
 * input isUpdate is 0 if generating from scratch; otherwise
 * update is treated as a merge of existing cfg blob
 *
 * returns 0 on success
 * returns negative # on failure, individual values do not have
 *     any meaning other than to point to places in the code.
 *
 */
static int processUpdatedCfg(int isUpdate)
{
    int i, rc, rc2;

    DBGPRINT("processUpdatedCfg() called with isUpdate=%d\n", isUpdate);

    /* copy the PERM_CFG_BLOB as the basis of the active blob */


    // Make sure we have a PERM blob or we have nothing to copy
    if (!bool_detect_file_exists(PERM_CFG_BLOB)) {
        DBGPRINT("system error for configuration\n");
        return -1;
    }
    // If in isUpdate mode, verify we have an ACTV_CFG_BLOB or we are not an update
    if (isUpdate && !bool_detect_file_exists(ACTV_CFG_BLOB)) {
        DBGPRINT("In processUpdatedCfg(upgrade=Yes), but ACTV_CFG_BLOB does not exist!  Turning off upgrade flag\n");
        isUpdate = 0;
    }
    // Update
    if (isUpdate) {
        // Copy ACTV to TMP, delete ACTV
        rc = system("cp " ACTV_CFG_BLOB " " TMP_CFG_BLOB);
        DBGPRINT("Copy of ACTV_CFG_BLOG to TMP_CFG_BLOB returned %d (want 0)\n", rc);
        if (rc) {
            DBGPRINT("ERROR: failure in copying ACTV_CFG_BLOB to TMP_CFG_BLOB\n");
            /* failure in creating blob */
            return -2;
        }
        // Mount the old-active-now-tmp-cfgblob to the IN_USE mntpnt
        if (system("mount -tvfat " TMP_CFG_BLOB " " IN_USE_MNTPNT)) {
            /* issue mounting; cleanup and fail */
            DBGPRINT("mount -tvfat " TMP_CFG_BLOB " " IN_USE_MNTPNT " failed, cleanup and fail\n");
            return -3;
        }
        /* IN_USE_MNTPNT already has a link */
    }
    // Copy the new firmware cfgblob to be the new actv
    if (system("cp " PERM_CFG_BLOB " " ACTV_CFG_BLOB)) {
        /* failure in creating blob */
        DBGPRINT("failure in copying PERM_CFG_BLOB to ACTV_CFG_BLOB\n");
        return -4;
    } else {
        DBGPRINT("Copy of PERM_CFG_BLOB to ACTV_CFG_BLOB successful\n");
    }
    /* mount ACTV_CFG_BLOB to initial mntpnt */
    rc = system("mount -tvfat " ACTV_CFG_BLOB " " INIT_MNTPNT);
    if (rc) {
        DBGPRINT("failure mounting ACTV_CFG_BLOB to INIT_MNTPNT\n");
        return -5;
    } else {
        DBGPRINT("Successfully mounted ACTV_CFG_BLOB to INIT_MNTPNT\n");
    }
    /* K: Just in case check */
    rc = bool_detect_if_mounted(ACTV_CFG_BLOB);
    DBGPRINT("Detect_if_mounted(ACTV_CFG_BLOB) said %s (want T)\n", rc ? "TRUE" : "FALSE");
    rc = bool_detect_if_mounted(INIT_MNTPNT);
    DBGPRINT("Detect_if_mounted(INIT_MNTPNT) said %s (want T)\n", rc ? "TRUE" : "FALSE");
    rc = bool_detect_if_mounted(TMP_CFG_BLOB);
    DBGPRINT("Detect_if_mounted(TMP_CFG_BLOB) said %s (want T)\n", rc ? "TRUE" : "FALSE");
    rc = bool_detect_if_mounted(IN_USE_MNTPNT);
    DBGPRINT("Detect_if_mounted(IN_USE_MNTPNT) said %s (want T)\n", rc ? "TRUE" : "FALSE");

    /* Copy over network-related settings */
    #define NUM_ALWAYSCOPY_SETTINGS 8
    char * always_copy_these_startupsh_setting_values[NUM_ALWAYSCOPY_SETTINGS] = {
        "DHCP_ENABLE",
        "IP_FOR_STATIC_ADDRESS",
        "NETMASK_FOR_STATIC_ADDRESS",
        "MTU",
        "VLAN_ENABLE",
        "VLAN_IP",
        "VLAN_MASK",
        "VLAN_ID"
    };
    for (i=0; i<NUM_ALWAYSCOPY_SETTINGS; i++) {
        DBGPRINT("DCU-Journal is copying over setting startup.sh:%s\n", always_copy_these_startupsh_setting_values[i]);
        rc = copy_config_value(IN_USE_MNTPNT "/startup.sh",
                               INIT_MNTPNT "/startup.sh",
                               always_copy_these_startupsh_setting_values[i]);
        if (rc != 0) {
            DBGPRINT("ERROR: Copy of startup.sh,%s failed! rc=%d (want 0)\n",
                always_copy_these_startupsh_setting_values[i], rc);
        } else {
            DBGPRINT("GOOD: Copy of startup.sh,%s successful\n",
                always_copy_these_startupsh_setting_values[i]);
        }
    }

    /* Copy over Journal's config settings */
    char srcpath[1000], dstpath[1000], cfg_file[1000], setting[1000];
    FILE * fin;
    int linecount = 0;
    if ((fin = fopen(DCU_JOURNAL_FILE, "r")) != NULL) {
        DBGPRINT("DCU-Journal: Successfully opened journal %s\n", DCU_JOURNAL_FILE);
        // NOTE: The space in format " %[^,],%s" is to strip the trailing
        // newline left over from the previous fscanf, otherwise every
        // newline will be included in the next fscanf'd strings.
        while (2 == fscanf(fin, " %[^,],%s", cfg_file, setting)) {
            ++linecount;
            DBGPRINT("DCU-Journal is copying over setting %s:%s\n", cfg_file, setting);
            // If cfgfile is the magic word, we have some journal magic to perform
            if (!strcmp(cfg_file, DCU_JOURNAL_MAGIC_WORD)) {
                // Magic+TSCBM_UPDATE -- copy over entire tscbm.conf file
                if (!strcmp(setting, TSCBM_UPDATE)) {
                    rc = system("cp " IN_USE_MNTPNT "/tscbm.conf " INIT_MNTPNT "/");
                    if (rc == 0) {
                        DBGPRINT("GOOD: Copy of tscbm.conf successful\n");
                    } else {
                        DBGPRINT("ERROR: Copy of tscbm.conf failed! rc=%d (want 0)\n", rc);
                    }
                } else {
                    DBGPRINT("ERROR: Unknown MAGIC setting %s - dont know what to do!\n", setting);
                }
            }
            // No magic, just a normal config setting copy
            else {
                snprintf(srcpath,sizeof(srcpath), "%s/%s", IN_USE_MNTPNT, cfg_file);
                snprintf(dstpath,sizeof(dstpath), "%s/%s", INIT_MNTPNT, cfg_file);
                rc = copy_config_value(srcpath, dstpath, setting);
                if (rc != 0) {
                    DBGPRINT("ERROR: Copy of %s,%s failed! rc=%d (want 0)\n", cfg_file, setting, rc);
                } else {
                    DBGPRINT("GOOD: Copy of %s,%s successful\n", cfg_file, setting);
                }
            }
        }
        fclose(fin);
        DBGPRINT("DCU-Journal: Closed journal, copied across %d settings' values\n", linecount);
    } else {
        DBGPRINT("DCU-Journal: Failed to open journal file %s, not copying over any more settings\n", DCU_JOURNAL_FILE);
    }
    DBGPRINT("DCU-Journal: Config Setting Copying done.\n");

    /* K: Safety check */
    rc = bool_detect_if_mounted(ACTV_CFG_BLOB);
    DBGPRINT("Before cleanup, detect_if_mounted(ACTV_CFG_BLOB) said %s (want T)\n", rc ? "TRUE" : "FALSE");
    rc = bool_detect_if_mounted(INIT_MNTPNT);
    DBGPRINT("Before cleanup, detect_if_mounted(INIT_MNTPNT) said %s (want T)\n", rc ? "TRUE" : "FALSE");
    /* cleanup */
    rc = system("umount " INIT_MNTPNT);
    DBGPRINT("cleanup: umount INIT_MNTPNT returned %d (want 0)\n", rc);
    /* K: Safety check */
    rc = bool_detect_if_mounted(ACTV_CFG_BLOB);
    DBGPRINT("After cleanup, detect_if_mounted(ACTV_CFG_BLOB) said %s (want F)\n", rc ? "TRUE" : "FALSE");
    rc = bool_detect_if_mounted(INIT_MNTPNT);
    DBGPRINT("After cleanup, detect_if_mounted(INIT_MNTPNT) said %s (want F)\n", rc ? "TRUE" : "FALSE");
    if (isUpdate) {
        DBGPRINT("Inside isUpdate Branch\n");
        rc = system("umount " IN_USE_MNTPNT);
        DBGPRINT("umount IN_USE_MNTPNT said %d (want 0)\n", rc);
        /* Unmount safety check */
        rc2 = bool_detect_if_mounted(TMP_CFG_BLOB);
        DBGPRINT("Detect_if_mounted(TMP_CFG_BLOB) said %s (want F)\n", rc2 ? "TRUE" : "FALSE");
        rc2 = bool_detect_if_mounted(IN_USE_MNTPNT);
        DBGPRINT("Detect_if_mounted(IN_USE_MNTPNT) said %s (want F)\n", rc2 ? "TRUE" : "FALSE");
        if (!rc) {
            /* delete old cfg - earlier renamed */
            remove(TMP_CFG_BLOB);   /* check return? */
            DBGPRINT("processUpdatedCfg(isUp=%d) finishing successfully !!\n", isUpdate);
            return 0;
        } else {
            DBGPRINT("umount IN_USE_MNTPNT failed, returned %d (want 0)\n", rc);
        }
    } else {
        DBGPRINT("Inside isNOTUpdate Branch\n");
        /* need to delete /rwflash/configs and create link to IN_USE_MNTPNT */
        /* rather than call remove c function, using rm -rf because remove needs
           directory to be empty; don't want to have to manage that here */
        rc = system("rm -rf " RWFLCFGS);
        if (!rc) {
            rc = system("ln -s " IN_USE_MNTPNT " " RWFLCFGS);
            if (!rc) {
                DBGPRINT("processUpdatedCfg(0) finishing successfully !\n");
                return 0;
            } else {
                DBGPRINT("ln -s " IN_USE_MNTPNT " " RWFLCFGS " failed, returned %d (want 0)\n", rc);
            }
        } else {
            DBGPRINT("rm -rf RWFLCFGS failed, returned %d (want 0)\n", rc);
        }
    }

    DBGPRINT("processUpdatedCfg() failed, exiting with failure code -6\n");
    return -6;
}

static void promoteBackup(void)
{
#if 0
    struct stat st;

    memset(&st, 0, sizeof(st));
    if (!stat(EMERGNCYBAK, &st)) {
        system("rm -f " RWFLCFGS);
        system("cp -rf " EMERGNCYBAK " " RWFLCFGS);
        /* the script below will ensure unix style files + permissions */
        system("/usr/scripts/runtime_check.sh");
    }
#endif
}
/*
 * initialize()
 *
 * The first thing that should be done to prepare the configs.
 * Initialize will prepare the rwflash/configs directory, including
 * merging if this is an upgrade (and first time conversion if needed)
 *
 * returns 0 on success
 *
 */
static int initialize(void)
{
    int fd, ret = 0;
#if 0
    int didupdate = 0;
#endif
    int rc;

    /* check if already mounted; if so, nothing to do */
    if (!system("mount | grep " ACTV_CFG_BLOB_MNTD ">>/dev/null")) {
        DBGPRINT("ACTV blob mounted, so initialize() done\n");
        /* nothing to do */
        return 0;
    }

    /* check if already in progress */
    if (-1 == (fd = open(PROGRESS_TMP_FILE, O_CREAT | O_EXCL, 644))) {
        /* another instance must have file open, do nothing */
        DBGPRINT("another instance must have file open, do nothing, so initialize() done\n");
        return 0;
    }

    /* delete any existing tmp blob */
    system("umount " DEFAULT_MNTPNT " >>/dev/null 2>&1");   /* just try it, don't care about return */
    remove(TMP_CFG_BLOB);

    /* check if ACTV_CFG_BLOB exists; if not it needs to be created */
    rc = bool_detect_file_exists(ACTV_CFG_BLOB);
    DBGPRINT("Detect ACTV_CFG_BLOB returned %s\n", rc ? "TRUE" : "FALSE");
    // Doesn't exist, so processUpdatedCfg .. whatever that does [20200113]
    if (! rc) {
        DBGPRINT("ACTV_CFG_BLOB doesnt exist, so calling processUpdatedCfg(0)\n");
#if 0
        didupdate = 1;
#endif
        rc = processUpdatedCfg(0);
        if (rc) {
            DBGPRINT("processUpdatedCfg(0) failed, returned %d (want 0)\n", rc);
            /* cleanup */
            close(fd);
            remove(PROGRESS_TMP_FILE);
            promoteBackup();
            return -1;
        } else {
            DBGPRINT("processUpdatedCfg(0) succeeded\n");
        }
    /* check to see if configs folder is a folder or link; if folder, proceed as if upgrade */
    } else {
        rc = bool_detect_is_directory(RWFLCFGS);
        DBGPRINT("Detect is Directory(RWFLCFGS) returned %s\n", rc ? "TRUE" : "FALSE");
        if (rc) {
            DBGPRINT("/rwflash/configs is a directory not a link; proceeding as if upgrade\n");
#if 0
            didupdate = 1;
#endif
            rc = processUpdatedCfg(0);
            if (rc) {
                DBGPRINT("processUpdatedCfg(0) failed, returned %d (want 0)\n", rc);
                /* cleanup */
                close(fd);
                remove(PROGRESS_TMP_FILE);
                promoteBackup();
                return -1;
            } else {
                DBGPRINT("processUpdatedCfg(0) succeeded\n");
            }
        }
        else {
            /* See if its a link, and if so, verify link is pointing correctly */
            if (bool_detect_is_symlink(RWFLCFGS)) {
                rc = system("ls -ld " RWFLCFGS " | grep " IN_USE_MNTPNT " 2> /dev/null");
                DBGPRINT("Detection if RWFLCFGS is pointed to IN_USE_MNTPT said %d (0 = correct)\n", rc);
                // Note: If link is correct, rc=0, exiting this block
            } else {
                rc = 1; // It's something else, or nothing, so set rc to blow it away and fix it
            }
            // FIX where link is pointing: RWFLCFGS -> IN_USE_MNTPT
            if (rc != 0) {
                DBGPRINT("FIXing where link is pointing\n", rc);
                system("rm -f " RWFLCFGS);  // Ignore ret code because if doesnt exist, will fail but is ok
                rc = system("ln -s " IN_USE_MNTPNT " " RWFLCFGS);
                DBGPRINT("ln -s " IN_USE_MNTPNT " " RWFLCFGS " returned %d (want 0)\n", rc);
                // Note: Now rc=0, exiting this block, if everything worked correctly
            }
            // If we are in an error, do recovery
            // TODO: This needs some rework ... case RWFLCFGS-doesnt-exist falls through this logic -- ask Jason
            if (rc != 0) {
                /* recovery? */
                rc = bool_detect_file_exists(RWFLCFGS);
                DBGPRINT("bool_detect_file_exists(RWFLCFGS) returned %s\n", rc ? "TRUE" : "FALSE");
                if (!rc) {
                    DBGPRINT("rwflash link doesn't exist; for now, no retry, promoting Backup\n");
                    /* rwflash link doesn't exist; for now, no retry */
                    close(fd);
                    promoteBackup();
                    return -1;
                } else { 
                    ; // TODO: also fail?  Ask Jason
                }
            }
            /* active blob exists; need to check if it needs to be updated */
            rc = system("mount -tvfat " ACTV_CFG_BLOB " " INIT_MNTPNT);
            DBGPRINT("mount -tvfat " ACTV_CFG_BLOB " " INIT_MNTPNT " returned %d (want 0)\n", rc);
            if (!rc) {
                /* Development safety check -- check both files exist */
                // rc = system("[[ -e " DEFAULT_VERSION_FILE " ]]");
                if (!bool_detect_file_exists(DEFAULT_VERSION_FILE)) {
                    DBGPRINT("Error: DEFAULT_VERSION_FILE %s doesnt exist!\n", DEFAULT_VERSION_FILE);
                }
                // rc = system("[[ -e " INIT_MNTPNT "/" BLOB_VERSION_FILE " ]]");
                if (!bool_detect_file_exists(INIT_MNTPNT "/" BLOB_VERSION_FILE)) {
                    DBGPRINT("Error: BLOB_VERSION_FILE %s/%s doesnt exist!\n", INIT_MNTPNT, BLOB_VERSION_FILE);
                }
                /* check fw match */
                ret = system("diff " DEFAULT_VERSION_FILE " " INIT_MNTPNT "/" BLOB_VERSION_FILE ">>/dev/null");
                DBGPRINT("Diff of firmware version and blob version said %d (0==same, triggers upgrade if not)\n", ret);
                // And ... save a copy of the diff for developer to peruse
                if (ret != 0) {
                    system("diff " DEFAULT_VERSION_FILE " " INIT_MNTPNT "/" BLOB_VERSION_FILE "> /tmp/dcu_diff_output.txt");
                    DBGPRINT("Diff of firmware version and blob version files in /tmp/dcu_diff_output.txt\n");
                }
                // Done with ACTV_CFG_BLOB for now
                rc = system("umount " INIT_MNTPNT);
                if (rc != 0) {
                    DBGPRINT("ERROR: Failed to unmount INIT_MNTPNT after version chk done with it\n");
                } else {
                    DBGPRINT("Successfully unmounted INIT_MNTPNT after version chk done with it\n");
                }
                if (ret) {    /* from diff command */
                    DBGPRINT("Since versions differ, calling  processUpdatedCfg(1)\n", rc);
#if 0
                    didupdate = 1;
#endif
                    rc = processUpdatedCfg(1);
                    DBGPRINT("processUpdatedCfg(1) said %d (want 0)\n", rc);
                    if (rc) {
                        DBGPRINT("Since processUpdatedCfg failed, promoting backup\n");
                        /* cleanup */
                        close(fd);
                        remove(PROGRESS_TMP_FILE);
                        promoteBackup();
                        return -1;
                    }
                }
            } else {
                DBGPRINT("ERROR: Can't diff config versions since ACTV_CFG_BLOB didnt mount .. now what?\n");
            }
        }
    }

    /* active blob is ready to mount */
    rc = system("mount -r -tvfat " ACTV_CFG_BLOB " " IN_USE_MNTPNT);
    DBGPRINT("mount -r -tvfat " ACTV_CFG_BLOB " " IN_USE_MNTPNT " said %d (want 0)\n", rc);
    if (!rc) {
        // All good!
        close(fd);   /* closing after last action */
        /* cleanup */
        remove(PROGRESS_TMP_FILE);
#if 0
        if (didupdate) {
            /* save the copy */
            system("mkdir -p " EMERGNCYBAK);
            system("cp -rf " RWFLCFGS "/* " EMERGNCYBAK);
        }
#endif
        return 0;
    } else {
        DBGPRINT("ERROR: ACTV_CFG_BLOB failed to mount!\n");
    }

    close(fd);
    /* cleanup */
    remove(PROGRESS_TMP_FILE);
    promoteBackup();
    return -1;
}

/*
 * management()
 *
 * management action decided by input value
 *
 * returns 0 on success
 *
 */
//TODO: Need to add support/test for restoring FWDMSG.
static int management(char *value)
{
    /* possible future extension */
    /* char cmd[400] = {0}; */   /* uneducated guess for max size */
    int rc;

    /* RESETDFLTS - Reset configs completely to factory defaults */
    /* RESETDFLTSBUTSAVENET - RESETDFLTS, but preserve the 
            always_copy_these_startupsh_setting_values settings */
    if (!strcmp(value, RESETDFLTS) || !strcmp(value,RESETDFLTSBUTSAVENET) || !strcmp(value, RESETCUSTOMER)) {
        // Unmount all the blobs; we'll throw an error if they stay mounted
        //  so no need to also test return from umounts
        system("umount " TMP_CFG_BLOB);
        system("umount " ACTV_CFG_BLOB);
        if (bool_detect_if_mounted(TMP_CFG_BLOB)) {
            DBGPRINT("ERROR: Failed to unmount TMP_CFG_BLOB!\n");
            return -1;
        }
        if (bool_detect_if_mounted(ACTV_CFG_BLOB)) {
            DBGPRINT("ERROR: Failed to unmount TMP_CFG_BLOB!\n");
            return -1;
        }
        DBGPRINT("Successfully unmounted all blobs\n");

        // Extra Debug
        DBGPRINT("bool_detect_file_exists(DCU_JOURNAL_FILE) says %d\n",
            bool_detect_file_exists(DCU_JOURNAL_FILE));

        // Remove DCU Journal
        if (bool_detect_file_exists(DCU_JOURNAL_FILE)) {
            rc = system("rm " DCU_JOURNAL_FILE);
            if (rc) {
                DBGPRINT("ERROR: Failed to remove journal file!  rc=%d (want 0)\n");
                return -1;
            } else {
                DBGPRINT("Successfully removed dcu journal\n");
            }
        } else {
            DBGPRINT("No DCU journal to remove, all good\n");
        }

        // If this reset is preserving networking settings ..
        if (!strcmp(value,RESETDFLTSBUTSAVENET)) {

            // We can leverage DCU's initialize process to perform our
            // ResetButSave process.  We can remove the DCU journal and then
            // call the initialize action as if we are in a firmware upgrade.
            // The point we call is the processUpdatedCfg(isUpdate=1), which
            // will copy a fresh perm config blob from the firmware as in a
            // normal firmware upgrade, and then preserve Network settings
            // also as in a normal firmware upgrade.  Since we just deleted
            // the journal we know we won't carry over any other settings.
            if (0 != (rc = processUpdatedCfg(1))) {
                DBGPRINT("ERROR: processUpdatedCfg() failed! rc=%d.\n",rc);
                return -1;
            }
            DBGPRINT("Returned successfully from processUpdatedCfg()\n");
        
        // Else if this reset is to revert to the customer backup configs...
        } else if (!strcmp(value,RESETCUSTOMER)) {
            if (0 != (rc = resetCustomerConfigs())) {
                DBGPRINT("ERROR: resetCustomerConfigs() failed! rc=%d. \n", rc);
                return -1;
            } 
        // No, it's a full facdef reset, simply wipe the active blob, and
        //   after reboot, DCU's initialize installs a copy of the perm blob.
        } else {

            // Remove active blob
            if (bool_detect_file_exists(ACTV_CFG_BLOB)) {
                DBGPRINT("ACTV_CFG_BLOB detected, need to remove it\n");
                rc = system("rm " ACTV_CFG_BLOB);
                if (rc) {
                    DBGPRINT("ERROR: Failed to remove active blob file! rc=%d (want 0)\n", rc);
                    return -1;
                } else {
                    DBGPRINT("Successfully removed active blob\n");
                }
            } else {
                DBGPRINT("No active blob to remove, all good\n");
            }
        }
#if 0
        // Remove emergency backup.  It might not exist, so dont test return
        if (0 != (rc=system("rm -rf " EMERGNCYBAK))) {
            DBGPRINT("INFO: Failed to remove EMERGNCYBAK. rc=%d (want 0)\n", rc);
        } else {
            DBGPRINT("Successfully removed EMERGNCYBAK.\n");
        }
#endif
        // Ready for reboot, which will install PERM_CFG_BLOB for factory defaults
        DBGPRINT("RESETDFLTS done, ready for reboot!\n");
        return 0;

#ifdef DIAG_OPS
    } else if (!strcmp(value, STOPI2VTERMHLTH) || !strcmp(value, HLTHACTVTYNIX)) {
        /* this should not be in cfgmgr - but temporarily support it due to time constraints and priorities of other work */
        /* terminate rsu health */
        system("kill -9 `pidin -f Aa | grep rsuh | grep -v grep | awk '{print $NF}'` 2>/dev/null");
        if (!strcmp(value, STOPI2VTERMHLTH)) {
            /* and stop i2v */
            system("slay i2v 2>/dev/null");
            /* just to be sure */
            system("kill -9 i2v 2>/dev/null");
        }
        return 0;
#endif /* DIAG_OPS */
    }

    return -1;
}

void append_to_dcu_journal(char * cfgfile, char * param)
{
    FILE *fp;
    char buff1[500], buff2[500];
    // If cfgfile is a fullpath, move pointer to just the base file
    if (index(cfgfile,'/')) {
        cfgfile = rindex(cfgfile,'/') + 1;
    }
    // See if combo already exists in journal
    int do_append = 1;
    if ((fp = fopen(DCU_JOURNAL_FILE, "r")) != NULL) {
        // NOTE: The space in format " %[^,],%s" is to strip the trailing
        // newline left over from the previous line's fscanf, otherwise
        // the newline will be included in the next fscanf'd strings.
        while (do_append && 2 == fscanf(fp, " %[^,],%s", buff1, buff2)) {
            if (!strcmp(buff1,cfgfile) && !strcmp(buff2,param)) {
                DBGPRINT("Found %s:%s already in journal, not going to append it again\n", cfgfile, param);
                do_append = 0;
            } else {
                DBGPRINT("Line with %s:%s does not match %s:%s\n", buff1,buff2,cfgfile,param);
            }
        }
        fclose(fp);
    } else {
        DBGPRINT("WARNING: Failed to open DCU_JOURNAL to test for duplicates!\n");
    }
    if (do_append) {
        if ((fp = fopen(DCU_JOURNAL_FILE, "a")) != NULL) {
            fprintf(fp, "%s,%s\n", cfgfile, param);
            fclose(fp);
            DBGPRINT("DCU-Journal: Appended %s:%s to journal\n", cfgfile, param);
        } else {
            DBGPRINT("ERROR: DCU-Journal: Failed to append %s:%s to journal\n", cfgfile, param);
        }
    }
}

// Tests if setting in a config file is "setting = value; min, max" or just "setting = value"
// Important because it affects which sed regex to use when replacing it
// Moved into its own function because of the duplicate code in updateCfg and hating to
//   debug two iterations of the same thing (but not quite) when porting to Herc/5912.
//   Also might be reusable elsewhere when importing values during a firmware upgrade
int test_if_setting_value_follow_by_semicolon(char * cfgfile, char * param)
{
    char cmd[400];
    int rc;

    // Test for a space OR an equals after the setting name, then a semicolon, before any comment
    snprintf(cmd,sizeof(cmd), "egrep '^[[:space:]]*%s( |=)' %s | grep ';' | grep -v '=[^;]*#' >/dev/null", param, cfgfile);
    DBGPRINT("Semicolon-Test: %s\n", cmd);
    rc = system(cmd);
    DBGPRINT("Semicolon-Test returned %d, applying not, returning %d\n", rc, !rc);

    return !rc;
}


/* 
    updateCfg -- called by set() to do the actual config file updating
    if hasFullPath = 1; cfgfile has a full path; otherwise it is just file name (like i2v.conf)
*/
static int updateCfg(char *param, char *value, char *cfgfile, char hasFullPath)
{
    char cmd[400];   /* uneducated guess for max size */
    char fullpath_cfgfile[400];

    if ((NULL == param) || (NULL == value) || (NULL == cfgfile)) {
        return -1;
    }

    DBGPRINT("updateCfg called with param %s value %s and cfgfile %s\n", param, value, cfgfile);

    /* If cfgfile isn't a fullpath, prepend DEFAULT_MNTPNT to it */
    if (!hasFullPath) {
        snprintf(fullpath_cfgfile,sizeof(fullpath_cfgfile), "%s/%s", DEFAULT_MNTPNT, cfgfile);
    } else {
        strncpy(fullpath_cfgfile, cfgfile,sizeof(fullpath_cfgfile));
    }

    /* confirm file exists */
    if (!bool_detect_file_exists(fullpath_cfgfile)) {
        /* file doesn't exist */
        DBGPRINT("  -> Abort: file %s doesn't exist!\n", fullpath_cfgfile);
        return -1;
    }

    /* If config file is a no-equals conf, handle it special */
    if (bool_config_is_a_noequals_config_file(cfgfile)) {
        snprintf(cmd,sizeof(cmd),"sed -i 's/\\(^%s \\).*/\\1%s/' %s", param, value, fullpath_cfgfile);
        DBGPRINT("conf file doesnt use equals. Going to use this cmd:\n");
        DBGPRINT("   %s\n", cmd);
    // Normal conf file, Figure out semicolon status, build the update sed cmd
    } else if (test_if_setting_value_follow_by_semicolon(fullpath_cfgfile, param)) {
        /* Setting has a ; after the value, edit to replace stuff after the equals and before semicolon */
        snprintf(cmd,sizeof(cmd), "sed -i 's/\\(^[[:space:]]*%s *= *\\)[^;]*\\(;.*\\)/\\1%s\\2/' %s", param, value, fullpath_cfgfile);
        DBGPRINT("param has a ; after the value.  Going to use this cmd:\n");
        DBGPRINT("   %s\n", cmd);
    } else {
        /* Setting has no ; after the value; edit to replace first word after the equals */
        snprintf(cmd,sizeof(cmd), "sed -i 's/\\(^[[:space:]]*%s *= *\\)[^\\ ]*\\(.*\\)/\\1%s\\2/' %s", param, value, fullpath_cfgfile);
        DBGPRINT("param does NOT have a ; after the value.  Going to use this cmd:\n");
        DBGPRINT("   %s\n", cmd);
    }
    // Run update sed command
    if (!system(cmd)) {
        DBGPRINT("   ==> system call succeeded!\n");
        append_to_dcu_journal(cfgfile, param);
        return 0;
    }

    DBGPRINT("   ==> system call FAILED\n");
    return -1;
}

/*
 * optionalParams()
 *
 * set/get an optional value in the staged configuration
 * temp blob needs to be mounted r/w (unlocked)
 * if file not specified, first matching param is updated
 *
 * returns -1 if failure; 0 on disabled param, 1 on enabled param
 *
 */
static int optionalParams(char *param, char *optionalFile, char *value)
{
    char *ptr, filename[MAX_I_FILENAME] = {0};
    char pcmd[400] = {0};   /* uneducated guess for max size */
    char newvalue[MAX_STRING] = {0};
    FILE *f;
    char opaction = 0;   /* follows the bitmask */
    /* bitmask */
    enum {
        OPENABLE = 0x1,
        OPDISABLE = 0x02,
        OPSET = 0x04,
        OPGET = 0x08,
    };

    /* error checking; mandatory args */
    if (NULL == param) {
        DBGPRINT("a system error\n");
        return -1;
    }
    /* the following is only because I was burned some time ago by the compiler optimizing code to generate a null pointer; this could be merged below with no new lines */
    if (NULL != optionalFile) {
        if (!strlen(optionalFile)) optionalFile = NULL;
    }

    /* value is passed in (checked before fxn called); assume good */
    ptr = strtok(value, ":");    /* normally, would call this tok, but reusing ptr since it is used later not as a tok */

    /* supported operations, using if-elseif-else for future expansion with possibly more than single characters */
    if (!strcmp(ptr, "e")) {
        /* enable the optional param */
        opaction = OPENABLE;
        if (NULL != (ptr = strtok(NULL, " "))) {
            /* set the value as well */
            opaction |= OPSET;
            strcpy(newvalue, ptr);
        }
    } else if (!strcmp(ptr, "d")) {
        /* disable the optional param */
        opaction = OPDISABLE;
    } else if (!strcmp(ptr, "s")) {
        /* set the optional param */
        if (NULL != (ptr = strtok(NULL, " "))) {
            opaction |= OPSET;
            strcpy(newvalue, ptr);
        } else {
            /* a value was not specified or error on strtok */
            return -1;
        }
    } else if (!strcmp(ptr, "g")) {
        /* get the optional param (and indicate if enabled) */
        opaction = OPGET;
    } else {
        /* unsupported operation */
        return -1;
    }

    /* for set, enable, disable, staged cfg must be mounted; future expansion needs to check logic below to make sure it applies */
    if ((opaction & ~(OPGET)) && system("mount | grep " TMP_CFG_BLOB_MNTD ">>/dev/null")) {
        /* not mounted, do no more */
        return -1;
    }

    /* need to find file if no input specified */
    if (NULL == optionalFile) {
        if (opaction & OPGET) {
            /* just use rwflash configs for get op */
            snprintf(pcmd,sizeof(pcmd), "find " RWFLCFGS "/ -name '*.conf' |grep -v " MERGE_DIR "|xargs grep ^[[:space:]]*#*%s|head -1|awk '{print $1}'|sed 's/:.*//'", param);
        } else {
            snprintf(pcmd,sizeof(pcmd), "find " DEFAULT_MNTPNT " -name '*.conf' |grep -v " MERGE_DIR "|xargs grep ^[[:space:]]*#*%s|head -1|awk '{print $1}'|sed 's/:.*//'", param);
        }
        if (NULL != (f = popen(pcmd,"r"))) {
            fgets(filename, MAX_I_FILENAME, f);
            pclose(f);
            /* fgets may put a '\n' at the end */
            ptr = filename + strlen(filename) - 1; 
            if (*ptr == '\n') {
                *ptr = 0;   /* NULL terminate there */
            }
        }
        if (!strlen(filename)) {
            /* no corresponding file found; param must not exist */
            return -1;
        }
    } else {
        /* put directory first */
        if (opaction & OPGET) {
            strcpy(filename, RWFLCFGS);
        } else {
            strcpy(filename, DEFAULT_MNTPNT);
        }
        strcat(filename, "/");
        strcat(filename, optionalFile);
    }

    /* confirm file exists */
    snprintf(pcmd,sizeof(pcmd), "[[ -e %s ]]", filename);
    if (system(pcmd)) {
        /* file doesn't exist */
        return -1;
    }
    /* now confirm filename is correct case; the QNX DOS does what DOS does which is case insensitve; code however is not */
    snprintf(pcmd,sizeof(pcmd), "ls `dirname %s` | grep `basename %s` >/dev/null 2>&1", filename, filename);
    if (system(pcmd)) {
        /* file input is wrong case; for set this means we could overwrite filename and break boot up behavior */
        return -1;
    }

    /* perform operation */
    /* mutually exclusive ops below */
    if (opaction & OPSET) {
        snprintf(pcmd,sizeof(pcmd),"cat %s | grep '^[[:space:]]*#*%s[[:space:]]*' | grep ';' >/dev/null", filename, param);
        if (!system(pcmd)) {
            /* param has a ; after the value, edit to save the stuff after the value */
            snprintf(pcmd,sizeof(pcmd), "sed -i 's/\\(^[[:space:]]*#*%s *= *\\).*\\(;.*\\)/\\1%s\\2/' %s >>/dev/null", param, newvalue, filename);
        } else {
            snprintf(pcmd,sizeof(pcmd), "sed -i 's/\\(^[[:space:]]*#*%s *= *\\)[^\\ ]*\\(.*\\)/\\1%s\\2/' %s >>/dev/null", param, newvalue, filename);
        }
        if (system(pcmd)) {
            return -1;
        }
        /* if no other action end here */
        if (!(opaction & ~OPSET)) {
            /* detect if enabled; the second grep sets the terminal return code */
            snprintf(pcmd,sizeof(pcmd), "cat %s| grep ^[[:space:]]*%s[[:space:]]|head -1|grep ^[[:space:]]*%s >>/dev/null", filename, param, param);
            return (!system(pcmd)) ? 1 : 0;    /* 1 = enabled; 0 = disabled */
        }
    } else if (opaction & OPGET) {
        /* get cannot be combined with other actions */
        snprintf(pcmd,sizeof(pcmd), "cat %s| grep ^[[:space:]]*#*%s[[:space:]]|head -1|sed 's/\\(^[[:space:]]*#*%s[^;]*\\)[;#].*/\\1/'|sed 's/.*=\\(.*\\)/\\1/'|sed 's/[[:space:]]//g'", filename, param, param);
        if (!system(pcmd)) {
            /* detect if enabled; the second grep sets the terminal return code */
            snprintf(pcmd,sizeof(pcmd), "cat %s| grep ^[[:space:]]*%s[[:space:]]|head -1|grep ^[[:space:]]*%s >>/dev/null", filename, param, param);
            return (!system(pcmd)) ? 1 : 0;    /* 1 = enabled; 0 = disabled */
        }
    }

    /* mutually exclusive ops below */
    if (opaction & OPENABLE) {
        snprintf(pcmd,sizeof(pcmd), "sed -i 's/^[[:space:]]*#*%s/%s/' %s >>/dev/null", param, param, filename);
        return (!system(pcmd)) ? 1 : -1;
    } else if (opaction & OPDISABLE) {
        snprintf(pcmd,sizeof(pcmd), "sed -i 's/^[[:space:]]*%s/#%s/' %s >>/dev/null", param, param, filename);
        return (!system(pcmd)) ? 0 : -1;
    }

    /* there should be no other way to reach here */
    return -1;
}

/*
 * set()
 *
 * set a value in the staged configuration
 * temp blob needs to be mounted r/w (unlocked)
 * if file not specified, first matching param is updated
 *
 * returns 0 on success
 *
 */
static int setConfigValue(char *param, char *optionalFile, char *value)
{
    char *ptr, filename[MAX_I_FILENAME] = {0};
    char pcmd[400] = {0};   /* uneducated guess for max size */
    FILE *f;

    /* commands:

        mount | grep $TMP_CFG_BLOB
        ...if input optional file specified, update cfg file and listing file:
          cat $DEFAULT_MNTPNT/$optionalfile | grep $param 
          then update cfg...
        ...if no input optional file; take first matching param from listing files; then update both listing + corresponding cfg file
          find $DEFAULT_MNTPNT -name '*.conf' | xargs grep $param | head -1 | awk 'print $1}' | sed 's/:.* //' (no space in sed command)
          then update cfg...

        then open $outfile for value
    */

    DBGPRINT("SET called on param %s with value %s\n", param, value);

    /* error checking; mandatory args */
    if (NULL == param) {
        DBGPRINT("a system error\n");
        return -1;
    }
    /* the following is only because I was burned some time ago by the compiler optimizing code to generate a null pointer; this could be merged below with no new lines */
    if (NULL != optionalFile) {
        if (!strlen(optionalFile)) optionalFile = NULL;
    }

    if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
        DBGPRINT("Detected " TMP_CFG_BLOB_MNTD " is mounted (want mounted)\n");
        if (NULL != optionalFile) {
            /* have file name; check for and update param */
            snprintf(pcmd,sizeof(pcmd), "grep %s %s/%s >>/dev/null",param, DEFAULT_MNTPNT, optionalFile);
            DBGPRINT("Attempting to system: %s\n", pcmd);
            if (system(pcmd) == 0) {    // 0 == grep found a match
                DBGPRINT("  --> successfully verified %s is in %s, proceeding to update\n", param, optionalFile);
                // Update param
                return updateCfg(param, value, optionalFile, 0);
            } else {
                DBGPRINT("Aborting set: failed to detect %s in %s\n", param, optionalFile);
            }
        } else {
            /* need to find param in listing files to determine file */
            snprintf(pcmd,sizeof(pcmd), "find " DEFAULT_MNTPNT " -name '*.conf' |grep -v " MERGE_DIR "|xargs grep ^[[:space:]]*%s|head -1|awk '{print $1}'|sed 's/:.*//'", param);
            DBGPRINT("Attempting to find config that contains param %s\n", param);
            if (NULL != (f = popen(pcmd,"r"))) {
                fgets(filename, MAX_I_FILENAME, f);
                pclose(f);
                /* fgets may put a '\n' at the end */
                if (strlen(filename) > 0) {
                    ptr = filename + strlen(filename) - 1; 
                    if (*ptr == '\n') {
                        *ptr = 0;   /* NULL terminate there */
                    }
                }
                if (strlen(filename) > 0) {
                    DBGPRINT("  --> Successfully found file %s contains param %s\n", filename, param);
                    return updateCfg(param, value, filename, 1);
                } else {
                    DBGPRINT("  --> Abort: no config file contains param %s\n", filename, param);
                    return -1;
                }
            } else {
                DBGPRINT("  --> Failed to popen the search system() command!\n");
            }
        }
    } else {
        DBGPRINT("  -> Abort: " TMP_CFG_BLOB_MNTD " is not mounted, cant make changes!\n");
    }

    return -1;
}


/*
 * networkCfg()
 *
 * get or set network settings; only one file matters: startup.sh
 * temp blob needs to be mounted r/w (unlocked) if writing
 *
 * returns 0 on success
 *
 * NOTE: no error checking on input value format; caller must assure proper IP
 *
 */
static int networkCfg(char *param, char *value)
{
    char tmpstr[MAXBUFFER] = {0};   /* uneducated guess for max size */

    /* param defines action:
        DHCP_ENABLE,   (no value)
        GET_MODE,      (no value)
        SET_STATIC_IP,
        STATIC_ENABLE, (optional value)
        MTU,
        VLAN_ENABLE,
        VLAN_IP,
        VLAN_MASK,
        VLAN_ID
    */

    if (NULL == param) {
        return -1;
    }

    if (NULL != value) {
        if (!strlen(value)) {
            value = NULL;
        }
    }

    // NETWORK CMD = Enable DHCP
    if (!strcmp(param, RSU_DHCP_ENABLE)) {
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            return setDHCPEnableSetting(DEFAULT_MNTPNT "/startup.sh", 1);  // 1 == True
        }
    // NETWORK CMD = Mode -- say if in DHCP or STATIC
    } else if (!strcmp(param, RSU_GET_MODE) || !strcmp(param, OBU_GET_MODE)) {
        switch (getDHCPEnableSetting(RWFLCFGS "/startup.sh")) {
            case 0:    /* disabled */
                printf("%s\n", CFG_STATIC);
                break;
            case 1:    /* enabled */
                printf("%s\n", CFG_DHCP);
                break;
            default:
                return -1;
        }
        return 0;
    // NETWORK CMD = GET STATIC IP
    } else if (!strcmp(param, RSU_GET_IP) || !strcmp(param, OBU_GET_IP)) {
        /* just use /rwflash directory */
        if (!getCfgIP(RWFLCFGS "/startup.sh", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Get Netmask
    } else if (!strcmp(param, RSU_GET_NETMASK) || !strcmp(param, OBU_GET_NETMASK)) {
        /* just use /rwflash directory */
        if (!getNetmask(RWFLCFGS "/startup.sh", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Set netmask
    } else if (!strcmp(param, RSU_SET_NETMASK) || !strcmp(param, OBU_SET_NETMASK)) {
        /* need to have staged configs mounted */
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setNetmask(DEFAULT_MNTPNT "/startup.sh", value);
            }
        }
    // NETWORK CMD - Set static IP
    } else if (!strcmp(param, RSU_SET_STATIC_IP) || !strcmp(param, OBU_SET_STATIC_IP)) {
        /* need to have staged configs mounted */
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setCfgIP(DEFAULT_MNTPNT "/startup.sh", value);
            }
        }
    // NETWORK CMD - Enable Static (really means disable DHCP)
    } else if (!strcmp(param, RSU_STATIC_ENABLE) || !strcmp(param, OBU_STATIC_ENABLE)) {
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            return setDHCPEnableSetting(DEFAULT_MNTPNT "/startup.sh", 0);   // 0 = False
        }
    // NETWORK CMD - Replace tscbm.conf contents with passed file
    } else if (!strcmp(param, TSCBM_UPDATE)) {
        DBGPRINT("Command TSCBM_UPDATE entered\n");
        // If updating is enabled
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            // Note user has updated tscbm.conf so dcu carries it across future firmware upgrades
            append_to_dcu_journal(DCU_JOURNAL_MAGIC_WORD, TSCBM_UPDATE);
            // Do the copy
            DBGPRINT("Command TSCBM_UPDATE is copying %s to configs/tscbm.conf\n", value);
            snprintf(tmpstr,sizeof(tmpstr),"cp %s %s/tscbm.conf", value, DEFAULT_MNTPNT);
            return system(tmpstr);
        } else {
            DBGPRINT("Command TSCBM_UPDATE aborted, read-only mode is active\n");
        }
    // NETWORK CMD - Get bozohttpd's http port
    } else if (!strcmp(param, RSU_GET_HTTP_PORT)) {
        DBGPRINT("Command RSU_GET_HTTP_PORT entered\n");
        if (!getScriptVariableValue(RWFLCFGS "/startup.sh", "WEBGUI_HTTP_SERVICE_PORT", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Set bozohttpd's http port
    } else if (!strcmp(param, RSU_SET_HTTP_PORT)) {
        DBGPRINT("Command RSU_SET_HTTP_PORT entered\n");
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setScriptVariableValue(DEFAULT_MNTPNT "/startup.sh", "WEBGUI_HTTP_SERVICE_PORT", value);
            }
        }
    // NETWORK CMD - Get bozohttpd's http<s> port
    } else if (!strcmp(param, RSU_GET_HTTPS_PORT)) {
        DBGPRINT("Command RSU_GET_HTTP_PORT entered\n");
        if (!getScriptVariableValue(RWFLCFGS "/startup.sh", "WEBGUI_HTTPS_SERVICE_PORT", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Set bozohttpd's http<s> port
    } else if (!strcmp(param, RSU_SET_HTTPS_PORT)) {
        DBGPRINT("Command RSU_SET_HTTPS_PORT entered\n");
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setScriptVariableValue(DEFAULT_MNTPNT "/startup.sh", "WEBGUI_HTTPS_SERVICE_PORT", value);
            }
        }
    // NETWORK CMD - Get MTU
    } else if (!strcmp(param, RSU_GET_MTU)) {
        DBGPRINT("Command RSU_GET_MTU entered\n");
        if (!getScriptVariableValue(RWFLCFGS "/startup.sh", "MTU", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Set MTU
    } else if (!strcmp(param, RSU_SET_MTU)) {
        DBGPRINT("Command RSU_SET_MTU entered\n");
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setScriptVariableValue(DEFAULT_MNTPNT "/startup.sh", "MTU", value);
            }
        }
    // NETWORK CMD = get VLAN Mode
    } else if (!strcmp(param, RSU_GET_VLAN_EN)) {
        switch (getVLANEnableSetting(RWFLCFGS "/startup.sh")) {
            case 0:    /* disabled */
                printf("%s\n", CFG_OFF);
                break;
            case 1:    /* enabled */
                printf("%s\n", CFG_ON);
                break;
            default:
                return -1;
        }
        return 0;
    // NETWORK CMD = Set VLAN_ENABLE
    } else if (!strcmp(param, RSU_SET_VLAN_EN)) {
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if ((value != NULL) && (1 == strtol(value, NULL, 10))) {
                return setVLANEnableSetting(DEFAULT_MNTPNT "/startup.sh", 1);  // 1 == True
            } else if (value != NULL) {
                return setVLANEnableSetting(DEFAULT_MNTPNT "/startup.sh", 0);  // 0 == False
            }
        }
    // NETWORK CMD - Get VLAN_IP
    } else if (!strcmp(param, RSU_GET_VLAN_IP)) {
        DBGPRINT("Command RSU_GET_VLAN_IP entered\n");
        if (!getScriptVariableValue(RWFLCFGS "/startup.sh", "VLAN_IP", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Set VLAN_IP
    } else if (!strcmp(param, RSU_SET_VLAN_IP)) {
        DBGPRINT("Command RSU_SET_VLAN_IP entered\n");
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setScriptVariableValue(DEFAULT_MNTPNT "/startup.sh", "VLAN_IP", value);
            }
        }
    // NETWORK CMD - Get VLAN_MASK
    } else if (!strcmp(param, RSU_GET_VLAN_MASK)) {
        DBGPRINT("Command RSU_GET_VLAN_MASK entered\n");
        if (!getScriptVariableValue(RWFLCFGS "/startup.sh", "VLAN_MASK", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Set VLAN_MASK
    } else if (!strcmp(param, RSU_SET_VLAN_MASK)) {
        DBGPRINT("Command RSU_SET_VLAN_MASK entered\n");
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setScriptVariableValue(DEFAULT_MNTPNT "/startup.sh", "VLAN_MASK", value);
            }
        }
    // NETWORK CMD - Get VLAN_ID
    } else if (!strcmp(param, RSU_GET_VLAN_ID)) {
        DBGPRINT("Command RSU_GET_VLAN_ID entered\n");
        if (!getScriptVariableValue(RWFLCFGS "/startup.sh", "VLAN_ID", tmpstr)) {
            printf("%s\n", tmpstr);
            return 0;
        }
    // NETWORK CMD - Set VLAN_ID
    } else if (!strcmp(param, RSU_SET_VLAN_ID)) {
        DBGPRINT("Command RSU_SET_VLAN_ID entered\n");
        if (bool_detect_if_mounted(TMP_CFG_BLOB_MNTD)) {
            if (value != NULL) {
                return setScriptVariableValue(DEFAULT_MNTPNT "/startup.sh", "VLAN_ID", value);
            }
        }
    // COMMAND NOT PORTED - RSU_SET_I2V
    // COMMAND NOT PORTED - RSU_SET_HEALTH
    // COMMAND NOT PORTED - RSU_SET_SYSTEM_MON
    }

    return -1;
}

/*
 * unlock()
 *
 * Unmount mounted cfg directory;
 * copy active cfg blob to temporary file
 * mount cfg directory r/w
 *
 * returns 0 on success
 *
 */
static int unlock(void)
{
    /* check if expected blob is mounted first; (indicating an action in
       progress don't unmount in that case) */
    /* commands:
        mount | grep $TMP_CFG_BLOB  (if found, done)
        cp $ACTV_CFG_BLOB $TMP_CFG_BLOB
        mount -tvfat $ACTV_CFG_BLOB $DEFAULT_MNTPNT
    */
    // Detect if the TMP blob is mounted, and if not, copy ACTV blob to TMP blob and then mount it
    int rc;
    rc = bool_detect_if_mounted(TMP_CFG_BLOB);
    DBGPRINT("Unlock: detect_mounted(TMP_CFG_BLOG) said %s\n", rc ? "True" : "False");
    if (!rc) {
        DBGPRINT("Unlock: temp blob not mounted; need to mount\n");
        /* temp blob not mounted; need to mount */
        remove(TMP_CFG_BLOB);   /* just in case; ignore any errors */
        /* create a copy of the unaltered config */
        rc = system("cp " ACTV_CFG_BLOB " " TMP_CFG_BLOB);
        DBGPRINT("cp " ACTV_CFG_BLOB " " TMP_CFG_BLOB " returned %d (want 0)\n", rc);
        if (!rc) {
            /* continue */
            rc = system("mount -tvfat " TMP_CFG_BLOB " " DEFAULT_MNTPNT);
            DBGPRINT("system(\"mount -tvfat " TMP_CFG_BLOB " " DEFAULT_MNTPNT ") returned %d (want 0)\n", rc);
            if (!rc) {
                return 0;
            } else {
                DBGPRINT("error on unlock\n");
            }
        }
    }
    return -1;
}

/*
 * main()
 *
 * Execution entry
 *
 */
int main(int argc, char *argv[])
{
    /* a gross overestimate of max string length */
    char param[MAX_STRING] = {0};
    char value[MAX_STRING] = {0};
    char file[MAX_I_FILENAME] = {0};   /* file names won't be longer (or else developer will need to try again) */
    /* in theory, only one variable is needed, but to be clear */
    int result = 0, action = NOACTION, haveValue = 0;
    signed char c_arg;
    int mount_wait_count=0;

// Enable this to log parameters (helpful debugging conf_manager/cfgmgrapi.c issues)
#ifdef WANT_PARAMS_LOGGED
    char xdebug[1000];
    int i;
    xdebug[0] = 0;
    for (i=1; i<argc; i++) {
        if (strlen(argv[i]) < (sizeof(xdebug) - strlen(xdebug) - 2)) {
            strcat(xdebug, " ");
            strcat(xdebug, argv[i]);
        }
    }
    DBGPRINT("Called with %d params:%s\n", argc-1, xdebug);
#endif

#ifdef WANT_BASIC_TESTING_OF_DCU_UTIL_FUNCTIONS
    /* Some basic testing */
    printf("remove_trailing_slash(/var/spool/mail/ivi) said %s (want unchanged)\n",
        remove_trailing_slash("/var/spool/mail/ivi"));
    printf("remove_trailing_slash(/var/spool/mail/) said %s (want /var/spool/mail)\n",
        remove_trailing_slash("/var/spool/mail/"));
    printf("bool_detect_is_directory(/etc) said %s (want T)\n",
        bool_detect_is_directory("/etc") ? "TRUE" : "FALSE");
    printf("bool_detect_file_exists(/tmp/notexist) said %s (want F)\n",
        bool_detect_file_exists("/tmp/notexist") ? "TRUE" : "FALSE");
    printf("bool_detect_is_directory(/etc/notexist) said %s (want F)\n",
        bool_detect_is_directory("/etc/notexist") ? "TRUE" : "FALSE");
    printf("bool_detect_is_directory(/etc/passwd) said %s (want F)\n",
        bool_detect_is_directory("/etc/passwd") ? "TRUE" : "FALSE");
    printf("bool_detect_file_exists(/etc/passwd) said %s (want T)\n",
        bool_detect_file_exists("/etc/passwd") ? "TRUE" : "FALSE");
    system("rm -rf /tmp/ln-s-etc");
    system("ln -s /etc /tmp/ln-s-etc");
    printf("bool_detect_file_exists(/tmp/ln-s-etc) said %s (want T)\n",
        bool_detect_file_exists("/tmp/ln-s-etc") ? "TRUE" : "FALSE");
    printf("bool_detect_is_directory(/tmp/ln-s-etc) said %s (want F)\n",
        bool_detect_is_directory("/tmp/ln-s-etc") ? "TRUE" : "FALSE");
    printf("bool_detect_is_directory(/tmp/ln-s-etc/) said %s (want F)\n",
        bool_detect_is_directory("/tmp/ln-s-etc/") ? "TRUE" : "FALSE");
    printf("bool_detect_is_symlink(/tmp/ln-s-etc) said %s (want T)\n",
        bool_detect_is_symlink("/tmp/ln-s-etc") ? "TRUE" : "FALSE");
    printf("bool_detect_is_symlink(/tmp/ln-s-etc/) said %s (want T)\n",
        bool_detect_is_symlink("/tmp/ln-s-etc/") ? "TRUE" : "FALSE");
    printf("bool_detect_is_symlink(/etc/passwd) said %s (want F)\n",
        bool_detect_is_symlink("/etc/passwd") ? "TRUE" : "FALSE");
    printf("bool_detect_is_symlink(/tmp) said %s (want F)\n",
        bool_detect_is_symlink("/tmp") ? "TRUE" : "FALSE");
    printf("bool_detect_is_symlink(/tmp/) said %s (want F)\n",
        bool_detect_is_symlink("/tmp/") ? "TRUE" : "FALSE");
#endif

    /* Create mount points - TEMP - until we create mount points into the NOR filesystem */
    mkdir(DEFAULT_MNTPNT, 0777);
    mkdir(IN_USE_MNTPNT, 0777);
    mkdir(INIT_MNTPNT, 0777);

//TODO: do not get stuck in loop
//error states
//i2v_util include?

    /* only allow a single action */
    while ((c_arg = getopt(argc, argv, "bc:df:g:i:m:n:o:rs:u:v:h")) != -1) {
        switch (c_arg) {
            case 'b':
            if (action == NOACTION) action = ACTION_BACKUP;
            break;
            
            case 'c':
            /* commit requires key */
            if (!strcmp(optarg, MASTERKEY)) {
                if (action == NOACTION) action = ACTION_COMMIT;
            } else {
                action = ACTION_EXIT;
            }
            break;

            case 'd':
            debugmode = 1;  /* non zero value will work */
            mylog_output_selector = 1;
            break;

            case 'f':    /* optional specifier for file */
            if (strlen(optarg) < sizeof(file) - 1) {
                strcpy(file, optarg);
            } else {
                DBGPRINT("ignoring invalid file name\n");
            }
            break;

            case 'g':
            if ((NULL != optarg) && (MAX_STRING > strlen(optarg))) {
                strcpy(param, optarg);
                if (action == NOACTION) action = ACTION_GET;  /* non zero value will work */
            } else {
                action = ACTION_EXIT;
            }
            break;

            case 'i':
            if (!strcmp(optarg, MASTERKEY)) {
                if (action == NOACTION) action = ACTION_INITIALIZE;
            } else {
                action = ACTION_EXIT;
            }
            break;

            case 'm':
            /* management requires key */
            if (!strcmp(optarg, MASTERKEY)) {
                if (action == NOACTION) action = ACTION_MANAGEMENT;  /* non zero value will work */
            } else {
                action = ACTION_EXIT;
            }
            break;

            case 'n':
            if ((NULL != optarg) && (MAX_STRING > strlen(optarg))) {
                strcpy(param, optarg);
                if (action == NOACTION) action = ACTION_NETWORK;  /* non zero value will work */
            } else {
                action = ACTION_EXIT;
            }
            break;

            case 'o':
            if ((NULL != optarg) && (MAX_STRING > strlen(optarg))) {
                strcpy(param, optarg);
                if (action == NOACTION) action = ACTION_OPTIONAL;  /* non zero value will work */
            } else {
                action = ACTION_EXIT;
            }
            break;

            /* cancel; this may be called be something that didn't initiate 
               cfg update (i.e. SNMP cancels, though GUI initiated) */
            case 'r':
            if (action == NOACTION) action = ACTION_REVERT;
            break;

            case 's':
            if ((NULL != optarg) && (MAX_STRING > strlen(optarg))) {
                strcpy(param, optarg);
                if (action == NOACTION) action = ACTION_SET;  /* non zero value will work */
            } else {
                action = ACTION_EXIT;
            }
            break;

            case 'u':
            if (NULL != optarg) {
                if (!strcmp(optarg, MASTERKEY)) {
                    DBGPRINT("Correct Masterkey '%s'\n", optarg);
                    if (action == NOACTION) { 
                        DBGPRINT("Set ACTION_UNLOCK\n");
                        action = ACTION_UNLOCK;
                    }
                } else {
                    DBGPRINT("Incorrect Masterkey '%s'\n", optarg);
                    action = ACTION_EXIT;
                }
            } else {
                DBGPRINT("NULL optarg instead of valid Masterkey\n");
                action = ACTION_EXIT;
            }
            break;

            case 'v':
            if ((NULL != optarg) && (MAX_STRING > strlen(optarg))) {
                strcpy(value, optarg);
                haveValue = 1;
            }
            break;

            default:
            break;
        }
        if (action == ACTION_EXIT) {
            DBGPRINT("Input arguments are insufficient\n");
            return -1;
        }
    }

    // Log results of our parsing
    if (mylog_output_selector > 0) {
        DBGPRINT("Parsed Args, Action=%s param=%s value=%s\n",
            getActionString(action), param[0] ? param : "", value[0] ? value : "");
    }

    /* perform intended action */
    switch (action) {
    
        case ACTION_BACKUP:
        result = backupCustomerConfigs();
        
        case ACTION_COMMIT:
        result = commit();
        break;

        case ACTION_GET:
        result = getConfigSettingValue(param, file);
        break;

        case ACTION_INITIALIZE:
        // During startup, /rwflash will still be in the process of mounting
        // when we are called to initialize().  We need to wait for /rwflash
        // to be fully mounted, and we'll do that right here.
        mount_wait_count=0;

//TODO: do not get stuck in loop

        while (!bool_is_rwflash_mounted()) {
            DBGPRINT("dcu:action:initialize() WAITING for rwflash to mount...\n");
            ++mount_wait_count;
            sleep(1);
        }
        if (mount_wait_count > 0) {
            DBGPRINT("dcu:action:initialize() After waiting %d secs now rwflash is mounted!\n", mount_wait_count);
        }
        result = initialize();
        break;

        case ACTION_MANAGEMENT:
        if (haveValue) {
            result = management(value);
        }
        break;

        case ACTION_NETWORK:
        /* network cfg may not require value, so no haveValue check needed */
        result = networkCfg(param, value);
        break;

        case ACTION_OPTIONAL:
        if (haveValue) {
            result = optionalParams(param, file, value);
        }
        break;

        case ACTION_REVERT:
        result = cancelUpdate();
        break;

        case ACTION_SET:
        if (haveValue) {
            result = setConfigValue(param, file, value);
        }
        break;

        case ACTION_UNLOCK:
        result = unlock();
        break;

        default:
        break;
    }

    /* the return(s) could be in the switch; but separating this out */
    DBGPRINT("EXIT: ret(%d)\n", result);
    return result;
}

