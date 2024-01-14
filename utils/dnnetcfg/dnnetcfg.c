#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "dn_types.h"
#include "i2v_util.h"
#include "ipcsock.h"
#include "rs.h"

#include "rsuhealth.h"


#define RULEFILE   "/rwflash/customer/routing.rules"
#define SETTINGS   "/rwflash/configs/startup.sh"

#define MY_ERR_LEVEL   LEVEL_INFO
#define MY_NAME        "dnnet"

/* 20230726: new functionality for MTU, VLAN support
   this is separated out for possible future support where
   only these items are called independently
*/
int16_t setMTU(const char *interface)
{
    int16_t mtu = -1, ret = 0;
    FILE *fp;
    char pcmd[100], buf[20];

    if (NULL == interface) {
        I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "setMTU called with null interface; internal error\n");
        return ret;
    }

    snprintf(pcmd, sizeof(pcmd), "cat " SETTINGS " | grep MTU | sed 's/.*=\\(.*\\)/\\1/'");
    if (NULL != (fp = popen(pcmd, "r"))) {
        fgets(buf, sizeof(buf), fp);
        mtu = (int16_t)strtol(buf, NULL, 10);
        pclose(fp);
    } else {
        I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "setMTU unable to read settings file; internal error (%s)\n", interface);
    }

    if (mtu > 0) {
        /* garbage in, garbage out */
        snprintf(pcmd, sizeof(pcmd), "ip link set %s mtu %d", interface, mtu);
        if (system(pcmd)) {
            I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "setMTU unable to apply system MTU; internal error (%s)\n", interface);
        } else {
            I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "MTU applied to interface (%s): %d\n", interface, mtu);
            ret = 1;
        }
    } else {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "System MTU is invalid; no change to default applied (%s)\n", interface);
    }

    return ret;
}

int16_t doVLAN(void)
{
#define PROGRESS_VIFC 0x01
#define PROGRESS_VIP  0x02

    int16_t vlanid = -1, ret = 0, vlanmask = -1;
    FILE *fp;
    char pcmd[100], buf[50];
    unsigned char idx;
    static unsigned char singleton = 0;

    if (singleton == (PROGRESS_VIFC | PROGRESS_VIP)) {
        return 1;
    }

    if (!system("cat " SETTINGS " | grep VLAN_ENABLE | grep FALSE")) {
        /* no work to do */
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "No VLAN to activate\n");
        singleton = PROGRESS_VIFC | PROGRESS_VIP;
        return 1;
    }

    snprintf(pcmd, sizeof(pcmd), "cat " SETTINGS " | grep VLAN_ID | sed 's/.*=\\(.*\\)/\\1/'");
    if (NULL != (fp = popen(pcmd, "r"))) {
        fgets(buf, sizeof(buf), fp);
        vlanid = (int16_t)strtol(buf, NULL, 10);
        pclose(fp);
    } else {
        I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "doVLAN unable to read settings file for VLAN_ID; internal error\n");
        return ret;
    }
    snprintf(pcmd, sizeof(pcmd), "cat " SETTINGS " | grep VLAN_MASK | sed 's/.*=\\(.*\\)/\\1/'");
    if (NULL != (fp = popen(pcmd, "r"))) {
        fgets(buf, sizeof(buf), fp);
        vlanmask = (int16_t)strtol(buf, NULL, 10);
        pclose(fp);
    } else {
        I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "doVLAN unable to read settings file for VLAN_MASK; internal error\n");
        return ret;
    }

    if (vlanid > 0 && vlanid < 4096 && vlanmask > 0 && vlanmask < 31) {
        /* garbage in, garbage out */
        snprintf(pcmd, sizeof(pcmd), "ip link add link eth0 name eth0.%d type vlan id %d >>/dev/null", vlanid, vlanid);
        idx = system(pcmd);    /* for some dumb reason; the return is 512; but the status can only go up to 127; use idx to mask return */
        if (idx && (idx <= 127)) {    /* if nonzero; ignore any status over 127 which is invalid status return */
            /* stop and go no further */
            I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "doVLAN unable to create VLAN interface; internal error (%d)\n", idx);
            I2V_DBG_LOG(LEVEL_DBG, MY_NAME, "doVLAN: %s\n", pcmd);
            return ret;
        }
        singleton = PROGRESS_VIFC;

        /* collect IP address and apply to vlan */
        if (NULL != (fp = popen("cat " SETTINGS " | grep VLAN_IP | sed 's/.*=\\(.*\\)/\\1/'", "r"))) {
            fgets(buf, sizeof(buf), fp);
            idx = strlen(buf);
            if (idx < 9) {
                /* minimum check for invalid data, i.e. 1.1.1.1/1 is 9 chars */
                I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "doVLAN detected invalid IP; internal error\n");
                return ret;
            }
            if (buf[idx - 1] == '\n') {
                buf[idx - 1] = 0;    /* remove any newline character */
            }
            snprintf(pcmd, sizeof(pcmd), "ip addr add %s/%d dev eth0.%d", buf, vlanmask, vlanid);   /* buf has IP */
            idx = system(pcmd);
            if (idx && (idx <= 127)) {
                /* stop */
                I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "doVLAN unable to set IP to VLAN interface; internal error (%s)\n", buf);
                return ret;
            }
            snprintf(pcmd, sizeof(pcmd), "ifconfig eth0.%d up", vlanid);
            idx = system(pcmd);
            if (idx && (idx <= 127)) {
                /* stop */
                I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "doVLAN unable to activate IP on VLAN interface; internal error (%s)\n", buf);
                return ret;
            }
            I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "System VLAN activated\n");
            singleton |= PROGRESS_VIP;
            ret = 1;
        } else {
            I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "doVLAN unable to collect VLAN IP from settings; internal error\n");
            return ret;
        }

        /* update mtu for vlan */
        snprintf(buf, sizeof(buf), "eth0.%d", vlanid);
        setMTU(buf);    /* will have it's own error logging */
    } else {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "System VLAN settings are invalid (%d %d); no VLAN activated\n", vlanid, vlanmask);
    }
    I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Done VLAN activate logic\n");

    return ret;
}

void checkResetGateway(void)
{
    /* if the rules file has a default gateway, check if applied on ifc; if not apply it */
    if (!system("[[ -e " RULEFILE " ]] && cat " RULEFILE " | grep default >> /dev/null")) {
        if (system("ip -4 route | grep default >> /dev/null")) {
            /* default has been lost; rerun ourself - will apply all rules again */
            I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Network gateway was lost (disconnect); attempting to restore\n");
            system("dnnetcfg");
        }
    }
}

/* just read in the routing.rules and only invoke route commands
   route commands are either with the command 'route' OR 'ip'
   i.e. route add default gw 192.168.2.254 eth1 
   OR  ip route add 192.168.1.0/24 dev eth1
*/

/**
* main()
*
* Main function
*
* returns 0 (success); -1 (failure); 1 (no rule file)
*
*/
int main(int argc, char *argv[])
{
    char rulefile[200] = {0};    /* waste of space */
    char cmd[200] = {0};
    char *ptr, rule[400] = {0};   /* really? - a rule this long shouldn't exist */
    FILE *f;
    struct stat st;

    i2vUtilEnableDebug(MY_NAME);
//TODO: sometimes too early for syslogs, serial only at that point.
//    :[   15.636088] rsuhealth.sh[2689]: dnnet|i2vUtilEnableSyslog: mq_open failed: errno(No such file or directory)
    i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME);

    if (argc > 1) {
        /* 20231115 JJG: this is absolutely LAZY, but so be it - just a quick hack of the input for a special check */
        if (!strcmp(argv[1], GWRST)) {    /* does not support a custom rule file */
            checkResetGateway();
            /* terminous operation */
            return 0;
        }
        /* cppcheck fix from strcpy to strncpy; good catch */
        strncpy(rulefile, argv[1], sizeof(rulefile));
    } else {
        /* cppcheck fix from sprintf to snprintf; good catch */
        snprintf(rulefile, sizeof(rulefile), RULEFILE);
    }
    /* future standalone option with a different input arg; for now always run; these generate their own log messages */
    setMTU("eth0");
    doVLAN();

    memset(&st, 0, sizeof(st));
    if (stat(RULEFILE, &st)) {
        /* rule file does not exist; done (don't mark as error) */
        return 0;    
    }

    if (NULL == (f = fopen(rulefile, "r"))) {
        /* error opening file */
        I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "%s: ERROR: unable to open rule file (%s)\n", argv[0], rulefile);
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
        } else {
            /* also test if the route is from the ip command */
            sprintf(cmd, "echo '%s' | sed 's/^[[:space:]]*//' | grep \"^ip \" | grep route >/dev/null", rule);
            if (!system(cmd)) {
                system(rule);
            } else {
                I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "%s: ERROR: invalid rule: (%s)\n", argv[0], rule);
            }
        }
    }
    fclose(f);
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(LEVEL_DBG, MY_NAME,"Exit.\n",MY_NAME);
#endif
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    /* 20211013: finally fixing idiot mistake i made months ago and then forgot to update */
    return 0;
}

