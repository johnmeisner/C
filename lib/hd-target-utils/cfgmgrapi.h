#ifndef CFGMGRAPI_H
#define CFGMGRAPI_H

//TODO this is actually MAX_TOKEN_SIZE
//#define MAX_RESULT_LENGTH 50

/* temporary */
#define DIAG_OPS 1

#define I2V_GRP      "i2v" 
#define AMH_GRP      "amh"
#define IPB_GRP      "ipb"
#define RSUINFO_GRP  "RSU_INFO"
#define SPAT_GRP     "spat16"
#define SCS_GRP      "scs"
#define SRMRX_GRP    "srm_rx"
#define FWDMSG_GRP   "fwdmsg"
#define SNMPD_GRP    "snmpd"
#define NSCONFIG_GRP "nsconfig"

/* function protos */

/**
 * cfgGetParam()
 *
 * Required for obtaining values, does not require unlocked config
 *
 * returns 0 on success
 *
 * Input:
 *   @param:  Desired config value to retrieve
 *   @file:   Desired group ie I2V_GRP; if unspecified, first matching param is retrieved
//TODO if you don't have a group then there are dupes in confs so....

 *   @result: String result of the parameter value
 *
 * NOTE: calling function must ensure adequate length for input strings
 *
 */
int cfgGetParam(char *param, char *group, char *result);

/**
 * cfgCheckParam()
 *
 * Range Check and validate conf value.
 * Required for changing values, requires unlocked config.
 *
 * returns 0 on success
 *
 * Input:
 *   @group:  Desired config group ie I2V_GRP defined above.
 *   @param:  Desired config value to update.
 *   @value:  String value to set for param.
 *
 */
int32_t cfgCheckParam(char_t * group, char_t * param, char_t * value);

/**
 * cfgSetParam()
 *
 * Required for changing values, requires unlocked config.
 * Call cfgCheckParam() first to verify value range before unlock and setting.
 *
 * returns 0 on success
 *
 * Input:
 *   @param:  Desired config value to update
 *   @group:  Desired group ie I2V_GRP; if unspecified, first matching param is updated
//TODO if you don't have  agroup then there are dupes in confs so....
 *            if specifying file, do not put /rwflash/configs in name; just file name like i2v.conf
 *   @value:  String value to set for param; be sure it is appropriate format 
 *            (i.e. don't put alpha characters for numeric param; the config file will get the invalid value)
 *
 */
int cfgSetParam(char *param, char *group, char *value);

/**
 * cfgGetOptionalParam()
 *
 * Required for obtaining optional values, does not require unlocked config
 * Optional params may or may not be enabled or exist (not mandatory for operation)
 *
 * returns -1 (failure or no param), 0 (param exists, but disabled), 1 (param enabled)
 *
 * Input:
 *   @param:  Desired config value to retrieve
 *   @file:   OPTIONAL config file; if unspecified, first matching param is retrieved
 *            if specifying file, do not put /rwflash/configs in name; just file name like i2v.conf
 *   @result: String result of the parameter value
 *
 * NOTE: calling function must ensure adequate length for input strings
 *
 */
int cfgGetOptionalParam(char *param, char *file, char *result);

/**
 * cfgSetOptionalParam()
 *
 * Required for changing optional values, requires unlocked config
 * Optional params may or may not be enabled or exist (not mandatory for operation)
 *
 * returns -1 (failure or no param), 0 (param exists, but disabled), 1 (param enabled)
 *
 * Input:
 *   @param:     Desired config value to update
 *   @file:      OPTIONAL config file; if unspecified, first matching param is updated
 *               if specifying file, do not put /rwflash/configs in name; just file name like i2v.conf
 *   @value:     String value to set for param; be sure it is appropriate format 
 *               (i.e. don't put alpha characters for numeric param; the config file will get the invalid value)
 *   @doEnable:  Set to 1 to enable the param if not enabled; set to 0 to leave params enabled state unchanged
 *
 */
int cfgSetOptionalParam(char *param, char *file, char *value, int doEnable);

/**
 * cfgOptionalParamCtl()
 *
 * Required for changing optional values enabled state, requires unlocked config
 * Optional params may or may not be enabled or exist (not mandatory for operation)
 *
 * returns -1 (failure or no param), 0 (param disabled), 1 (param enabled)
 *
 * Input:
 *   @param:        Desired config value to update
 *   @file:         OPTIONAL config file; if unspecified, first matching param is updated
 *                  if specifying file, do not put /rwflash/configs in name; just file name like i2v.conf
 *   @enableState:  Set to 1 to enable the param if not enabled; set to 0 to disable if not disabled
 *
 */
int cfgOptionalParamCtl(char *param, char *file, int enableState);

/**
 * cfgCommit()
 *
 * Required to save the config changes
 *
 * returns 0 on success
 *
 */
int cfgCommit(void);

/**
 * cfgRevert()
 *
 * Cancels any config changes in progress; locks the config so no further editing
 *
 * returns 0 on success
 *
 */
int cfgRevert(void);

/**
 * cfgUnlock()
 *
 * Required for editing files; first step to edit
 *
 * returns 0 on success
 *
 */
int cfgUnlock(void);


/**
 *
 * Network Operations
 *
**/


/**
 * cfgNetworkDHCPEnable()
 *
 * Enables DHCP mode; requires unlocked configuration
 *
 * returns 0 on success
 *
 * Input:
 *   @isOBU:  Flag to indicate if system is an OBU; set to a nonzero value for LD OBU, set to zero for RSU or OBU-5910
 *
 */
int cfgNetworkSetModeDHCPEnabled(int isOBU);

/**
 * cfgNetworkMode()
 *
 * Retrieves ONLY current mode active on system
 *
 * returns 0 on success
 *
 * Input:
 *   @isOBU:  Flag to indicate if system is an OBU; set to a nonzero value for LD OBU, set to zero for RSU or OBU-5910
 *
 */
int cfgNetworkGetMode(int isOBU);

/**
 * cfgNetworkStaticEnable()
 *
 * Enables static IP setting, and optional updates IP; disables DHCP; requires unlocked configuration
 *
 * returns 0 on success
 *
 * Input:
 *   @staticIP:  OPTIONAL IP address; if unspecified previous static IP used; caller must ensure format of IP
 *   @isOBU:     Flag to indicate if system is an OBU; set to a nonzero value for LD OBU, set to zero for RSU or OBU-5910
 *
 */
int cfgNetworkSetModeStaticEnabled(char *staticIP, int isOBU);

/**
 * cfgNetworkStaticIP()
 *
 * Sets static IP but does not change current mode; requires unlocked configuration; static needs to be enabled
 *
 * returns 0 on success
 *
 * Input:
 *   @staticIP:  IP address; caller must ensure format of IP
 *   @isOBU:     Flag to indicate if system is an OBU; set to a nonzero value for LD OBU, set to zero for RSU or OBU-5910
 *
 */
int cfgNetworkSetStaticIP(char *staticIP, int isOBU);

/**
 * cfgNetworkGetIP()
 *
 * Retrieves static IP set in configuration; this IP may not match the active IP (if DHCP is enabled)
 *
 * returns 0 on success
 *
 * Input:
 *   @result: String result of the parameter value
 *   @isFECIP:  Flag to indicate if IP retrieved is FEC IP; if zero, IP retrieved is HD IP (SC0)
 *
 * NOTE: calling function must ensure adequate length for input string
 *
 */
int cfgNetworkGetIP(char *result, int isFECIP);

/**
 * cfgNetworkGetMask()
 *
 * Retrieves static IP mask set in configuration; this mask may not match the active mask (if DHCP is enabled)
 *
 * returns mask value on success or -1 on failure
 *
 * Input:
 *   @isFECIP:  Flag to indicate if mask retrieved is FEC mask; if zero, mask retrieved is HD mask (SC0) [if 2 masks]
 *
 */
int cfgNetworkGetMask(int isFECIP);

/**
 * cfgNetworkSetMask()
 *
 * Sets static IP mask set in configuration; this mask may not match the active mask (if DHCP is enabled)
 * Requires unlocked config
 *
 * returns 0 on success
 *
 * Input:
 *   @mask:     Integer input mask to set (valid range: 1 - 31)
 *   @isFECIP:  Flag to indicate if mask set is FEC mask; if zero, mask set is HD mask (SC0) [if 2 masks]
 *
 */
int cfgNetworkSetMask(int mask, int isFECIP);

/**
 * cfgStartupSet()
 *
 * Sets i2v app and rsuhealth in startup.sh.
 *
 * returns the value of system call or -1 on failure
 *
 * Inputs:
 *   @value: To enable (1) / disable (0) i2v app or rsuhealth.
 *   @conf: The type of conf: I2V or HEALTH_ENABLE.
 *
 */
int cfgStartupSet(int value, char *conf);

/*
 * cfgNetworkGetMtu()
 *
 * Retrieves ONLY current mtu active on system
 *
 * returns mtu on success, negative on error
 *
 * JJG: type mismatch; header declares 'int' but function defined as int32_t; these are technically not the same;
 *      following the convention already established but this could be trouble down the road (applies to next few as well)
 *
 */
int cfgNetworkGetMtu(void);

/*
 * cfgNetworkSetMtu()
 *
 * Sets MTU; requires unlocked configuration
 *
 * returns 0 on success
 *
 * Input:
 *   @mtu: New MTU value in range of 68 - 1500
 *
 */
int cfgNetworkSetMtu(int mtu);

/*
 * cfgNetworkGetVLANEnable()
 *
 * Retrieves ONLY current VLAN mode active on system
 *
 * MODE means ON (1) or and OFF (0)
 *
 * returns mode (0 or 1) on success, negative on error
 *
 */
int cfgNetworkGetVLANEnable(void);

/*
 * cfgNetworkSetVLANEnable()
 *
 * Sets VLAN mode; requires unlocked configuration
 *
 * returns 0 on success
 *
 * Input:
 *   @enable: Setting to enable or disable (0 or 1)
 *
 */
int cfgNetworkSetVLANEnable(int enable);

/*
 * cfgNetworkGetVLANID()
 *
 * Retrieves ONLY current VLAN ID active on system
 *
 * returns VLAN ID on success, negative on error
 *
 */
int cfgNetworkGetVLANID(void);

/**
 * cfgNetworkGetVLANIP()
 *
 * Retrieves VLAN IP set in configuration
 *
 * returns 0 on success
 *
 * Input:
 *   @result: IP address
 *
 */
int cfgNetworkGetVLANIP(char_t *result);

/*
 * cfgNetworkSetVLANIP()
 *
 * Sets VLAN IP; requires unlocked configuration
 *
 * returns 0 on success
 *
 * Input:
 *   @staticIP: new IP address
 *
 */
int cfgNetworkSetVLANIP(char_t *staticIP);

/**
 * cfgNetworkGetVLANMask()
 *
 * Retrieves VLAN IP mask set in configuration
 *
 * returns mask on success
 *
 */
int cfgNetworkGetVLANMask(void);

/**
 * cfgNetworkSetVLANMask()
 *
 * Sets VLAN IP mask set in configuration
 * Requires unlocked config
 *
 * returns 0 on success
 *
 * Input:
 *   @mask: new VLAN mask
 *
 */
int cfgNetworkSetVLANMask(int mask);

/*
 * cfgNetworkSetVLANID()
 *
 * Sets VLAN ID; requires unlocked configuration
 *
 * returns 0 on success
 *
 * Input:
 *   @vlanid: New VLAN ID; allowed range 0 - 65535
 *
 */
int32_t cfgNetworkSetVLANID(int32_t vlanid);

#endif /* CFGMGRAPI_H */


/**
 *
 * Management Operations
 *
**/


/**
 * cfgResetToDefault(int bool_preserve_networking)
 *
 * Resets configuration to default deployed in firmware;
 * Two modes: everything reverts (including IPs), or
 *            everything but networking reverts.
 * This will interrupt any in progress config operations
 *
 * returns 0 on success
 *
 */
int cfgResetToDefault(int bool_preserve_networking);


/**
 * cfgReplaceTscbm()
 *
 * Copies content of passed file into the tscbm.conf config file
 *
 * returns 0 on success
 */
int cfgReplaceTscbm(char * filepath);

/**
 * customerBackup()
 *
 * Creates a backup blob of the current configs. 
 * Doesn't do anything to AMH files, etc, outside of the blob. 
 *
 * returns 0 on success
 *
 */
int32_t customerBackup(void);

/**
 * cfgResetToCustomer()
 *
 * Resets configuration to the customer-created backup point. 
 * Doesn't do anything to AMH files, etc, outside of the blob. 
 * If no customer blob exists, does nothing. 
 *
 * returns 0 on success
 *
 */
int32_t cfgResetToCustomer(void);

#ifdef DIAG_OPS

/**
 * cfgTermRSUHealth()
 *
 * Terminates RSU health script
 *
 * returns 0 on success
 *
 */
int cfgTermRSUHealth(void);

/**
 * cfgTermRSUSvcs()
 *
 * Terminates RSU health script AND i2v
 *
 * returns 0 on success
 *
 */
int cfgTermRSUSvcs(void);

/**
 * cfgNetworkGetHttpPort()
 * cfgNetworkGetHttpsPort()
 *
 * Returns the port number for the webgui http and https services.
 */
int cfgNetworkGetHttpPort();
int cfgNetworkGetHttpsPort();

/**
 * cfgNetworkSetHttpPort()
 * cfgNetworkSetHttpsPort()
 *
 * Sets the port number for the webgui http and https services.
 *
 * returns 0 on success
 */
int cfgNetworkSetHttpPort(int port);
int cfgNetworkSetHttpsPort(int port);
#endif /* DIAG_OPS */

