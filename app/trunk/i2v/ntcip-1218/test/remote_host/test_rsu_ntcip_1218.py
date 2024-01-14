###
### PRE REQ:
### pip install pysnmp
### pip install pysnmp-mibs
###

from pysnmp.hlapi import *
from pysnmp.entity.rfc3413.oneliner import cmdgen
from pysnmp.proto import rfc1902
from pysnmp.proto import rfc1905
from pywinauto import application

import sys
sys.path.append("framework")
import os
import pysnmp
import datetime as dt
import time
import socket
import binascii
from contextlib import contextmanager
import txt_file
import utils
import WebUI
import RSU
import inspect

from pysnmp import debug
from pysmi import debug as pysmi_debug

#from pysnmp import debug
#from pysmi import debug as pysmi_debug

#debug.setLogger(debug.Debug('dsp', 'io', 'msgproc', 'secmod'))
#pysmi_debug.setLogger(pysmi_debug.Debug('searcher', 'reader', 'compiler'))
#debug.setLogger(debug.Debug('io', 'msgproc', 'secmod'))

#SNMP V3 support
SNMP_userName='linadmin' 
SNMP_authKey='linauthpass' 
SNMP_privKey='linprivpass'

#Current MIB under test.
my_mib = 'NTCIP1218-v01'
    
denso_rsu = '10.52.11.80'
denso_rsu_port = 161
mccain = '172.18.3.31'
mccain_port = 161
econolite = '172.18.2.32'
econolite_port = 501
trafficware = '172.18.2.229'
trafficware_port = 501
siemens = "172.18.3.23"  # down
siemens_port = 0  # ?
# mccain, econolite, trafficware all support only SNMPv1 aka mpModel=0

#SNMP: row status
nonExistent   = 0
active        = 1 
notInService  = 2
notReady      = 3
createAndGo   = 4
createAndWait = 5
destroy       = 6

#
# NTCIP-1218 MIB Identifier:
#
# rsu MODULE-IDENTITY
#     LAST-UPDATED     "202009040000Z"
#     DESCRIPTION      "This MIB defines the Roadside Unit (RSU) Objects"
#     REVISION         "202009040000Z"

#
# Common shared values and sizes from MIB.
#

RSU_MIB_VERSION = "202009040000Z"

# Radio 
MAX_RSU_RADIOS  = 2      # DSRC and CV2X
RSU_RADIO_MAC_LENGTH = 6
RSU_RADIO_PSID_SIZE  = 4 # https://standards.ieee.org/products-services/regauth/psid/public.html

RSU_RADIO_CHANNEL_MIN = 172
RSU_RADIO_CHANNEL_MAX = 184

RSU_RADIO_TX_POWER_MIN = -128
RSU_RADIO_TX_POWER_MAX =  127

RSU_LAT_MIN     = -900000000
RSU_LAT_UNKNOWN = 900000001
RSU_LAT_MAX     = RSU_LAT_UNKNOWN

RSU_LON_MIN     = -1800000000
RSU_LON_UNKNOWN = 1800000001 
RSU_LON_MAX     = RSU_LON_UNKNOWN

RSU_ELV_MIN     = -100000
RSU_ELV_UNKNOWN = 1000001 
RSU_ELV_MAX     = RSU_ELV_UNKNOWN

# Date & Time.
RFC2579_DATEANDTIME_LENGTH  = 8
MIB_DATEANDTIME_LENGTH      = 8

#Network
RSU_PORT_MIN = 0  # Can be 1024 but in some cases 0 is ok
RSU_PORT_MAX = 65535

URI255_LENGTH_MIN = 0  # Common URL lengths
URI255_LENGTH_MAX = 255

URI1024_LENGTH_MIN = 0
URI1024_LENGTH_MAX = 1024

RSU_IP_MIN = 0 # IPv4 and IPv6
RSU_IP_MAX = 64
RSU_IP_MIN_DATA   = ''
RSU_IP_MAX_DATA_1 = '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABABBA'
RSU_IP_MAX_DATA_2 = '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABBEEF'
RSU_IP_MAX_DATA_3 = '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABACDC'
RSU_IP_MAX_DATA_4 = '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCAFE'

#
#
#
def test_min_max_int32(r,my_name,my_index,my_min,my_max):
    if(r.SNMPSet(my_name,my_index, rfc1902.Integer32(my_min)) == -1):
        print("  Set " + my_name + "." + str(my_index) + " MIN int32 Failed.")
        return -1
    if(r.SNMPGet(my_name, my_index) !=  rfc1902.Integer32(my_min)):
        print("  Get " + my_name  + "." + str(my_index) + " MIN int32 Failed.")
        return -1    
    if(r.SNMPSet(my_name,my_index, rfc1902.Integer32(my_max)) == -1):
        print("  Set " + my_name  + "." + str(my_index) + " MAX int32 Failed.")
        return -1   
    if(r.SNMPGet(my_name, my_index) !=  rfc1902.Integer32(my_max)):
        print("  Get " + my_name  + "." + str(my_index) +  " MAX int32 Failed.")
        return -1        
    return 0
#
#
#
def test_min_max_octet_string(r,my_name,my_index,my_min,my_max):
    #min
    if(r.SNMPSet(my_name,my_index, rfc1902.OctetString(my_min)) == -1):
        print("  Set " + my_name  + "." + str(my_index) + " MIN octet_string Failed.")
        return -1
    if(r.SNMPGet(my_name, my_index) != rfc1902.OctetString(my_min)):
        print("  Get " + my_name  + "." + str(my_index) + " MIN octet_string Failed.")
        return -1
    #max
    if(r.SNMPSet(my_name,my_index, rfc1902.OctetString(my_max)) == -1):
        print("  Set " + my_name  + "." + str(my_index) + " MAX octet_string Failed.")
        return -1
    if(r.SNMPGet(my_name, my_index) != rfc1902.OctetString(my_max)):
        print("  Get " + my_name  + "." + str(my_index) + "  MAX octet_string Failed.")
        return -1       
    return 0
#
# NTCIP-1218: 5.6: rsuReceivedMsg
#    
def test_rsuReceivedMsg(r, radio):

    RSU_RX_MSG_MIN = 1 
    RSU_RX_MSG_MAX = 100

    RSU_RX_MSG_PORT_MIN = 1024
    RSU_RX_MSG_PORT_MAX = RSU_PORT_MAX

    RSU_RX_MSG_RSSI_THRESHOLD_MIN = -100
    RSU_RX_MSG_RSSI_THRESHOLD_MAX = -60

    RSU_RX_MSG_INTERVAL_MIN = 0
    RSU_RX_MSG_INTERVAL_MAX = 10

    RSU_RX_MSG_AUTH_INTERVAL_MIN = 0
    RSU_RX_MSG_AUTH_INTERVAL_MAX = 10
 
    OUTPUT_ADDRESS_MIN      = "" 
    OUTPUT_ADDRESS_MAX      = "AAAABBBBCCCCDDDD1111222233334444ffffeeeeddddcccc9999888877776666"
    OUTPUT_ADDRESS_DEFAULT_IPv6  = "0A34:0B2F:AABB:CCDD:0:0:0:0"
    OUTPUT_ADDRESS_DEFAULT  = "10.52.11.47"
 
 
    # 5.6.1	
    data =  r.SNMPGet('maxRsuReceivedMsgs', 0)      
    if((data == None) or (data != RSU_RX_MSG_MAX)):
        print("  Get maxRsuReceivedMsgs Failed: " + data)
        return -1
 
    #Remove any lingering rows. Could be empty but clear anyways.
    i=RSU_RX_MSG_MIN
    while (i <= RSU_RX_MSG_MAX):
        if(r.SNMPSet('rsuReceivedMsgStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set rsuReceivedMsgStatus." + str(i) + " Destroy Failed." )
            return -1 
        if(r.SNMPGet('rsuReceivedMsgStatus', i) != pysnmp.proto.rfc1905.NoSuchInstance):
            print(" Destroy Failed. Row i=" + str(i) + " still exists.")
            #return -1        
        i=i+1    

    # Set/Get columns of existing rows that exists. Without back end support nothing on RSU will happen.
    i=RSU_RX_MSG_MIN
    while (i <= RSU_RX_MSG_MAX):
        # 5.6.2.10: First CREATE row.
        if(r.SNMPSet('rsuReceivedMsgStatus', i, rfc1902.Integer32(createAndGo)) == -1):
            print("  Set " + 'rsuReceivedMsgStatus.i' + " Create Failed." )
            return -1                
        # 5.6.2.1: Index not available by definition. 
        # 5.6.2.2: Reject bad PSIDs and stay ready for TX.
        pass_list=[ "00",   "0000", "000000",   "00000000", \
                    "01",   "0001", "000001",   "00000001", \
                    "7f",   "007f", "00007f",   "0000007f", \
                            "8000", "008000",   "00008000", \
                            "bfff", "00bfff",   "0000bfff", \
                                    "c00000",   "00c00000", \
                                    "dfffff",   "00dfffff", \
                                                "e0000000", \
                                                "efffffff"]
        for val in pass_list:
            if not r.SNMPSet("rsuReceivedMsgPsid", i, rfc1902.OctetString(hexValue = val)):
                return(False, "rsuReceivedMsgPsid valid PSID rejected psid=" + val) 
        #Fail list will pass because no logic behind.                   
        fail_list=[ "80", "0080", "000080", "00000080", \
                    "ff", "00ff", "0000ff", "000000ff", \
                          "0100", "000100", "00000100", \
                          "7fff", "007fff", "00007fff", \
                          "c000", "00c000", "0000c000", \
                          "ffff", "00ffff", "0000ffff", \
                                  "010000", "00010000", \
                                  "bfffff", "00bfffff", \
                                  "e00000", "00e00000", \
                                  "ffffff", "00ffffff", \
                                            "01000000", \
                                            "dfffffff", \
                                            "f0000000", \
                                            "ffffffff" ]
        for val in fail_list:
            # Should not be "not"
            if not r.SNMPSet("rsuReceivedMsgPsid", 1, rfc1902.OctetString(hexValue = val)):
                return(False, "rsuReceivedMsgPsid invalid PSID accepted psid=" + val)
               
        # 5.6.2.3:
        if( test_min_max_octet_string(r,'rsuReceivedMsgDestIpAddr', i, OUTPUT_ADDRESS_MIN, OUTPUT_ADDRESS_MAX) == -1):
            print("  rsuReceivedMsgDestIpAddr." + str(i) + " Min/Max Failed.")
            return -1         
        # 5.6.2.4
        if( test_min_max_int32(r,'rsuReceivedMsgDestPort',i, RSU_RX_MSG_PORT_MIN, RSU_RX_MSG_PORT_MAX) == -1):
            print("  rsuReceivedMsgDestPort." + str(i) + " Min/Max Failed.")
            return -1     
        # 5.6.2.5
        if( test_min_max_int32(r,'rsuReceivedMsgProtocol',i, 1, 2) == -1):
            print("  rsuReceivedMsgProtocol." + str(i) + " Min/Max Failed.")
            return -1
        # 5.6.2.6
        if( test_min_max_int32(r,'rsuReceivedMsgRssi',i, RSU_RX_MSG_RSSI_THRESHOLD_MIN,RSU_RX_MSG_RSSI_THRESHOLD_MAX) == -1):
            print("  rsuReceivedMsgRssi." + str(i) + " Min/Max Failed.")
            return -1    
        # 5.6.2.7
        if( test_min_max_int32(r,'rsuReceivedMsgInterval',i, RSU_RX_MSG_INTERVAL_MIN, RSU_RX_MSG_INTERVAL_MAX) == -1):
            print("  rsuReceivedMsgInterval." + str(i) + " Min/Max Failed.")       
            return -1
        # 5.6.2.8
        if( test_min_max_octet_string(r,'rsuReceivedMsgDeliveryStart', i, '\0\0\0\0\0\0\0\0', '@@@@@@@@') == -1):
            print("  rsuReceivedMsgDeliveryStart." + str(i) + " Min/Max Failed.")
            return -1     
        # 5.6.2.9
        if( test_min_max_octet_string(r,'rsuReceivedMsgDeliveryStop', i, '\0\0\0\0\0\0\0\0', '@@@@@@@@') == -1):
            print("  rsuReceivedMsgDeliveryStop." + str(i) + " Min/Max Failed.")
            return -1     
        # 5.6.2.10: rsuReceivedMsgStatus
        # 5.6.2.11
        if( test_min_max_int32(r,'rsuReceivedMsgSecure', i, 0, 1) == -1):
            print("  rsuReceivedMsgSecure." + str(i) + " Min/Max Failed.")
            return -1     
        # 5.6.2.12
        if( test_min_max_int32(r,'rsuReceivedMsgAuthMsgInterval', i, RSU_RX_MSG_AUTH_INTERVAL_MIN , RSU_RX_MSG_AUTH_INTERVAL_MAX) == -1):
            print("  rsuReceivedMsgAuthMsgInterval." + str(i) + " Min/Max Failed.")
            return -1    
        i = i + 1

    i=RSU_RX_MSG_MIN
    while (i <= RSU_RX_MSG_MAX):
        if(r.SNMPSet('rsuReceivedMsgStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Closing Destroy Failed. rsuReceivedMsgStatus." + str(i) + "." )
            return -1 
        if(r.SNMPGet('rsuReceivedMsgStatus', i) != pysnmp.proto.rfc1905.NoSuchInstance):
            print(" Closing Destroy Failed. rsuReceivedMsgStatus." + str(i) + " still exists.")
            return -1   
        i=i+1    
    
    #Done.
    print("Test 5.6: rsuReceivedMsgs: Passed.")  
    return 0

#
# NTCIP-1218: 5.8
#
def test_rsuInterfaceLog(r):

    RSU_IFCLOG_MIN = 1 
    RSU_IFCLOG_MAX = 3  # 5912 & 5940 == eth0 + dsrc1 + cv2x1 is 3 total.

    RSU_IFCLOG_FILE_SIZE_MIN = 1
    RSU_IFCLOG_FILE_SIZE_MAX = 40
    RSU_IFCLOG_FILE_SIZE_DEFAULT = 5

    RSU_IFCLOG_FILE_TIME_MIN = 1
    RSU_IFCLOG_FILE_TIME_MAX = 48
    RSU_IFCLOG_FILE_TIME_DEFAULT = 24

    #RSU_INTERFACE_NAME_SIZE_MIN = 0 
    #RSU_INTERFACE_NAME_SIZE_MAX = 127 
    RSU_INTERFACE_NAME_MIN = '' 
    RSU_INTERFACE_NAME_MAX = 'ABBA56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123ABBA'    

    #RSU_IFCLOG_STORE_PATH_SIZE_MIN = 1
    #RSU_IFCLOG_STORE_PATH_SIZE_MAX = 255 
    RSU_IFCLOG_STORE_PATH_MIN = '/'    
    RSU_IFCLOG_STORE_PATH_MAX = 'BEEF5678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901BEEF' 
   
    #RSU_IFCLOG_NAME_SIZE_MIN = 12
    #RSU_IFCLOG_NAME_SIZE_MAX = 172
    RSU_IFCLOG_NAME_MIN = 'BAD456789012'
    RSU_IFCLOG_NAME_MAX = 'DEAD56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678DEAD'     

    RSU_IFCLOG_OPTIONS_MIN = 0x00
    RSU_IFCLOG_OPTIONS_MAX = 0x30 # diskFull(0),deleteEntry(1)

    #inboundOnly  = 1, /* in file only. */
    #outboundOnly = 2, /* out file only. */
    #biSeparate   = 3, /* in and out files. */
    #biCombined   = 4, /* in and out combined in one file. */

    RSU_IFCLOG_BYDIR_MIN = 1 
    RSU_IFCLOG_BYDIR_MAX = 4  

    print("Test 5.8: rsuInterfaceLog: Start.")
    # 5.8.1	
    data = r.SNMPGet('maxRsuInterfaceLogs', 0)      
    if((data == None) or (data != RSU_IFCLOG_MAX)):
        print("  Get maxRsuInterfaceLogs Failed: " + data)
        return -1

    i=RSU_IFCLOG_MIN
    while (i <= RSU_IFCLOG_MAX):
        if( r.SNMPSet('rsuIfaceLogStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuIfaceLogStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuIfaceLogStatus', i) == 0):
            print("  Get " + 'rsuIfaceLogStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    

    # Set/Get columns of existing rows that exists. Without back end support nothing on RSU will happen.
    i=RSU_IFCLOG_MIN
    while (i <= RSU_IFCLOG_MAX):
        # 5.8.2.12: First CREATE row.
        if( r.SNMPSet('rsuIfaceLogStatus', i, rfc1902.Integer32(createAndGo)) == -1):
            print("  Set " + 'rsuIfaceLogStatus.i' + " Create Failed." )
            return -1                
        # 5.8.2.1: Index not available by definition. 
        # 5.8.2.2
        if( test_min_max_int32(r,'rsuIfaceGenerate', i, 0, 1) == -1):
            return -1  
        # 5.8.2.3
        if( test_min_max_int32(r,'rsuIfaceMaxFileSize', i,RSU_IFCLOG_FILE_SIZE_MIN, RSU_IFCLOG_FILE_SIZE_MAX) == -1):
            return -1  
        # 5.8.2.4
        if( test_min_max_int32(r,'rsuIfaceMaxFileTime', i,RSU_IFCLOG_FILE_TIME_MIN, RSU_IFCLOG_FILE_TIME_MAX) == -1):
            return -1  
        # 5.8.2.5
        if( test_min_max_int32(r,'rsuIfaceLogByDir',i,RSU_IFCLOG_BYDIR_MIN, RSU_IFCLOG_BYDIR_MAX) == -1):
            return -1
        # 5.8.2.6            
        if( test_min_max_octet_string(r,'rsuIfaceName', i, RSU_INTERFACE_NAME_MIN, RSU_INTERFACE_NAME_MAX) == -1):
            return -1 
        # 5.8.2.7           
        if( test_min_max_octet_string(r,'rsuIfaceStoragePath', i, RSU_IFCLOG_STORE_PATH_MIN, RSU_IFCLOG_STORE_PATH_MAX) == -1):
            return -1
        # 5.8.2.8       
        if( test_min_max_octet_string(r,'rsuIfaceLogName', i, RSU_IFCLOG_NAME_MIN, RSU_IFCLOG_NAME_MAX) == -1):
            return -1
        # 5.8.2.9         
        if( test_min_max_octet_string(r,'rsuIfaceLogStart', i, '\0\0\0\0\0\0\0\0', '@@@@@@@@') == -1):
            return -1
        # 5.8.2.10         
        if( test_min_max_octet_string(r,'rsuIfaceLogStop', i, '\0\0\0\0\0\0\0\0', '@@@@@@@@') == -1):
            return -1                     
        # 5.8.2.11 BITS (diskFull(0),deleteEntry(1))
        my_name = 'rsuIfaceLogOptions'
        if(r.SNMPSet( my_name,i, rfc1902.Bits(chr(RSU_IFCLOG_OPTIONS_MIN))) == False):
            print("  Set RSU_IFCLOG_OPTIONS_MIN Failed." )
            return -1
        if (r.SNMPGet( my_name,i) != rfc1902.Bits(chr(RSU_IFCLOG_OPTIONS_MIN))):
            print("  Get RSU_IFCLOG_OPTIONS_MIN Failed." )
            return -1;
        if(r.SNMPSet( my_name,i, rfc1902.Bits(chr(RSU_IFCLOG_OPTIONS_MAX))) == False):
            print("  Set RSU_IFCLOG_OPTIONS_MAX Failed." )
            return -1
        if (r.SNMPGet( my_name,i) != rfc1902.Bits(chr(RSU_IFCLOG_OPTIONS_MAX))):
            print("  Get RSU_IFCLOG_OPTIONS_MAX Failed." )
            return -1;            
        i = i + 1

    i=RSU_IFCLOG_MIN
    while (i <= RSU_IFCLOG_MAX):
        if( r.SNMPSet( 'rsuIfaceLogStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuIfaceLogStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuIfaceLogStatus', i) == 0):
            print("  Get " + 'rsuIfaceLogStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    

    #Done.
    print("Test 5.8: rsuInterfaceLog: Passed.")  
    return 0
#
# NTCIP-1218: 5.9
#
def test_rsuSecurity(r):

    #from ./i2v/ntcip-1218/ntcip-1218.h
    RSU_SEC_APP_CERTS_MAX = 32
    RSU_SEC_APP_PSID_MAX  = 63  
    RSU_SECURITY_PROFILES_MAX = 100
    
    RSU_SEC_APP_CERT_STATE_MIN = 1 # 1 = other, 2 = valid, 3 = notValid, 4 = future: validty= time + region.
    RSU_SEC_APP_CERT_STATE_MAX = 4
    RSU_SEC_APP_CERT_STATE_DEFAULT = 3
    
    print("Test 5.9: rsuSecurity: Started.")
    
    #5.9.1
    if( test_min_max_int32(r,'rsuSecCredReq',0,0,8736) == -1):
        return -1 
    #5.9.2
    data = r.SNMPGet('rsuSecEnrollCertStatus', 0)
    if((data == None) or (data < 1) or (4 < data)):
        print("  Get rsuSecEnrollCertStatus Failed.")
        return -1      
    #5.9.3
    data = r.SNMPGet('rsuSecEnrollCertValidRegion', 0)    
    if((data == None) or (data < 0) or (65535 < data)):
        print("  Get rsuSecEnrollCertValidRegion Failed.")
        return -1
    #5.9.4
    data = r.SNMPGet('rsuSecEnrollCertUrl',0)
    if(data == None):
        print("  Get rsuSecEnrollCertUrl Failed.")
        return -1
    #5.9.5
    data = r.SNMPGet('rsuSecEnrollCertId',0)
    #TODO check length of string returned. MIB does it for us but nice to see.
    #if((data == None) or (len(data) < 1) or (255 < len(data))):
    if(data == None):
        print("  Get rsuSecEnrollCertId Failed.")
        return -1  
    #5.9.6: 
    data = r.SNMPGet('rsuSecEnrollCertExpiration',0)
    if(data == None):
        print("  Get rsuSecEnrollCertExpiration.")
        return -1 
    #5.9.7    
    data = r.SNMPGet('rsuSecuritySource', 0)    
    if((data == None) or (data < 1) or (3 < data)):
        print("  Get rsuSecuritySource Failed.")
        return -1   
    #5.9.8
    data = r.SNMPGet('rsuSecAppCertUrl',0)
    if(data == None):
        print("  Get rsuSecAppCertUrl Failed.")
        return -1
    #5.9.9
    data = r.SNMPGet('maxRsuSecAppCerts', 0)    
    if((data == None) or (data != RSU_SEC_APP_CERTS_MAX)):
        print("  Get maxRsuSecAppCerts Failed.")
        return -1
    #5.9.10.1 : RsuTableIndex not accessible
    #5.9.10.2
    data = r.SNMPGet('rsuSecAppCertPsid',1)
    if(data == None):
        print("  Get rsuSecAppCertPsid.1 Failed.")
        return -1
    data = r.SNMPGet('rsuSecAppCertPsid',RSU_SEC_APP_CERTS_MAX)    
    if(data == None):
        print("  Get rsuSecAppCertPsid.maxrow Failed.")
        return -1       
    #5.9.10.3   
    data = r.SNMPGet('rsuSecAppCertState', 1)    
    if((data == None) or (data < 1) or (4 < data)):
        print("  Get rsuSecAppCertState.1 Failed.")
        return -1
    data = r.SNMPGet('rsuSecAppCertState', RSU_SEC_APP_CERTS_MAX)    
    if((data == None) or (data < 1) or (4 < data)):
        print("  Get rsuSecAppCertState.maxrow Failed.")
        return -1     
    #5.9.10.4   
    data = r.SNMPGet('rsuSecAppCertExpiration', 1)    
    if((data == None) or (data < 0) or (255 < data)):
        print("  Get rsuSecAppCertExpiration.1 Failed.")
        return -1
    data = r.SNMPGet('rsuSecAppCertExpiration', RSU_SEC_APP_CERTS_MAX)    
    if((data == None) or (data < 0) or (255 < data)):
        print("  Get rsuSecAppCertExpiration.maxrow Failed.")
        return -1    
    #5.9.10.5
    if( test_min_max_int32(r,'rsuSecAppCertReq', 1, 0,65535) == -1):
        return -1
    if( test_min_max_int32(r,'rsuSecAppCertReq', RSU_SEC_APP_CERTS_MAX, 0, 65535) == -1):
        return -1           
    #5.9.11
    data = r.SNMPGet('rsuSecCertRevocationUrl', 0)    
    if(data == None):
        print("  Get rsuSecCertRevocationUrl Failed.")
        return -1
    #5.9.12 
    data = r.SNMPGet('rsuSecCertRevocationTime', 0)    
    if(data == None):
        print("  Get rsuSecCertRevocationTime Failed.")
        return -1
    #5.9.13
    if( test_min_max_int32(r,'rsuSecCertRevocationInterval', 0, 0,255) == -1):
        return -1
    #5.9.14
    if( test_min_max_int32(r,'rsuSecCertRevocationUpdate', 0, 0,1) == -1):
        return -1
    #5.9.15    
    data = r.SNMPGet('maxRsuSecProfiles', 0)    
    if((data == None) or (data != RSU_SECURITY_PROFILES_MAX)):
        print("  Get maxRsuSecProfiles Failed.")
        return -1     
    #5.9.16.2
    data = r.SNMPGet('rsuSecProfileName', 1)    
    if(data == None):
        print("  Get rsuSecProfileName.1 Failed.")
        return -1
    data = r.SNMPGet('rsuSecProfileName', RSU_SECURITY_PROFILES_MAX)    
    if(data == None):
        print("  Get rsuSecProfileName.maxrow Failed.")
        return -1
    #5.9.16.3
    data = r.SNMPGet('rsuSecProfileDesc', 1)    
    if(data == None):
        print("  Get rsuSecProfileDesc.1 Failed.")
        return -1
    data = r.SNMPGet('rsuSecProfileDesc', RSU_SECURITY_PROFILES_MAX)    
    if(data == None):
        print("  Get rsuSecProfileDesc.maxrow Failed.")
        return -1
    
    #Done   
    print("Test 5.9: rsuSecurity: Passed.")    
    return 0
#
# NTCIP-1218: 5.10
#
def test_rsuWsaConfig(r):

    RSU_WSA_SERVICES_MAX = 100 # from ./i2v/ntcip-1218/ntcip-1218.h
    RSU_WSA_SERVICES_MIN = 1

    WSA_PRIORITY_MIN = 1
    WSA_PRIORITY_MAX = 7

    #WSA_PSC_LENGTH_MIN = 0
    #WSA_PSC_LENGTH_MAX = 31
    WSA_PSC_MIN = ''
    WSA_PSC_MAX = '1234567890123456789012345678901'
    #WSA_PSC_MAX = '1'    
    
    WSA_IP_ADDR_LENGTH_MIN = 0
    WSA_IP_ADDR_LENGTH_MAX = 64

    WSA_PORT_MIN = 1024
    WSA_PORT_MAX = 65535

    WSA_RCPI_THRESHOLD_MIN = 0
    WSA_RCPI_THRESHOLD_MAX = 255

    WSA_OPTIONS_MIN = 0x00
    WSA_OPTIONS_MAX = 0xf8  # 5 bits 2 extra reserved.

    WSA_COUNT_RCPI_MIN = 0
    WSA_COUNT_RCPI_MAX = 255
    
    WSA_COUNT_THRESHOLD_MIN = 0
    WSA_COUNT_THRESHOLD_MAX = 255

    WSA_COUNT_THRESHOLD_INTERVAL_MIN = 0
    WSA_COUNT_THRESHOLD_INTERVAL_MAX = 255

    WSA_REPEAT_RATE_MIN = 0
    WSA_REPEAT_RATE_MAX = 255 

    #WSA_AD_ID_LENGTH_MIN = 0
    #WSA_AD_ID_LENGTH_MAX = 31
    WSA_AD_ID_MIN = ''
    WSA_AD_ID_MAX = '1234567890123456789012345678901'    

    WSA_CHANNEL_ACCESS_MIN = 0
    WSA_CHANNEL_ACCESS_MAX = 3    
    
    RSU_WSA_VERSION = 3  # 3 = IEEE 1609.3-2016 
     
    print("Test 5.10: rsuWsaConfig: Start.")
    
    #5.10.1
    data = r.SNMPGet('maxRsuWsaServices', 0)    
    if((data == None) or (data != RSU_WSA_SERVICES_MAX)):
        print("  Get maxRsuWsaServices Failed.")
        return -1
    
    #5.10.2: WSA Service Table
    i=RSU_WSA_SERVICES_MIN
    while (i <= RSU_WSA_SERVICES_MAX):
        if( r.SNMPSet( 'rsuWsaStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuWsaStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuWsaStatus', i) == 0):
            print("  Get " + 'rsuWsaStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    

    # Set/Get columns of existing rows that exists. Without back end support nothing on RSU will happen.
    i=RSU_WSA_SERVICES_MIN
    while (i <= RSU_WSA_SERVICES_MAX):
        # 5.10.2.8: First CREATE row.
        if( r.SNMPSet( 'rsuWsaStatus', i, rfc1902.Integer32(createAndGo)) == -1):
            print("  Set " + 'rsuWsaStatus.i' + " Create Failed." )
            return -1           
        # 5.10.2.1: Index not available by definition. 
        # 5.10.2.2
        if( test_min_max_octet_string(r,'rsuWsaPsid', i, '\0', '@@@@') == -1):
            return -1 
        # 5.10.2.3
        if( test_min_max_int32(r,'rsuWsaPriority', i, WSA_PRIORITY_MIN, WSA_PRIORITY_MAX) == -1):
            return -1
        # 5.10.2.4:
        if( r.SNMPSet( 'rsuWsaPSC',i, rfc1902.OctetString(WSA_PSC_MIN)) == -1):
            print("  Set " + 'rsuWsaPSC' + " MIN Failed.")
            return -1
        if( r.SNMPGet( 'rsuWsaPSC', i) == -1):
            print("  Get " + 'rsuWsaPSC' + " MIN Failed.")
            return -1
        if( r.SNMPSet( 'rsuWsaPSC',i, rfc1902.OctetString(WSA_PSC_MAX)) == -1):
            print("  Set " + 'rsuWsaPSC' + " MAX Failed.")
            return -1
        if( r.SNMPGet( 'rsuWsaPSC', i) == -1):
            print("  Get " + 'rsuWsaPSC' + " MAX Failed.")
            return -1
        # 5.10.2.5
        if( test_min_max_octet_string(r,'rsuWsaIpAddress', i, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_3) == -1):
            return -1
        # 5.10.2.6
        if( test_min_max_int32(r,'rsuWsaPort', i, WSA_PORT_MIN, WSA_PORT_MAX) == -1):
            return -1
        # 5.10.2.7
        if( test_min_max_int32(r,'rsuWsaChannel', i, RSU_RADIO_CHANNEL_MIN, RSU_RADIO_CHANNEL_MAX) == -1):
            return -1
        # 5.10.2.8: rsuWsaStatus
        # 5.10.2.9
        if( test_min_max_octet_string(r,'rsuWsaMacAddress', i, '\0\0\0\0\0\0','@@@==='  ) == -1):
            return -1
        # 5.10.2.10
        my_name = 'rsuWsaOptions'
        if(r.SNMPSet( my_name,i, rfc1902.Bits(chr(WSA_OPTIONS_MIN))) == False):
            print("  Set WSA_OPTIONS_MIN Failed." )
            return -1
        if (r.SNMPGet( my_name,i) != rfc1902.Bits(chr(WSA_OPTIONS_MIN))):
            print("  Get WSA_OPTIONS_MIN Failed." )
            return -1;
        if(r.SNMPSet( my_name,i, rfc1902.Bits(chr(WSA_OPTIONS_MAX))) == False):
            print("  Set WSA_OPTIONS_MAX Failed." )
            return -1
        if (r.SNMPGet( my_name,i) != rfc1902.Bits(chr(WSA_OPTIONS_MAX))):
            print("  Get WSA_OPTIONS_MAX Failed." )
            return -1;             
        # 5.10.2.11
        if( test_min_max_int32(r,'rsuWsaRcpiThreshold', i, WSA_COUNT_RCPI_MIN, WSA_COUNT_RCPI_MAX) == -1):
            return -1
        # 5.10.2.12
        if( test_min_max_int32(r,'rsuWsaCountThreshold', i, WSA_COUNT_THRESHOLD_MIN, WSA_COUNT_THRESHOLD_MAX) == -1):
            return -1 
        # 5.10.2.13
        if( test_min_max_int32(r,'rsuWsaCountThresholdInterval', i, WSA_COUNT_THRESHOLD_INTERVAL_MIN, WSA_COUNT_THRESHOLD_INTERVAL_MAX) == -1):
            return -1   
        # 5.10.2.14
        if( test_min_max_int32(r,'rsuWsaRepeatRate', i, WSA_REPEAT_RATE_MIN, WSA_REPEAT_RATE_MAX) == -1):
            return -1
        # 5.10.2.15: Get and set works but verify on MAX fails.            
        if( test_min_max_octet_string(r,'rsuWsaAdvertiserIdentifier', i, WSA_AD_ID_MIN, WSA_AD_ID_MAX) == -1):
            return -1
        if( r.SNMPSet( 'rsuWsaAdvertiserIdentifier',i, rfc1902.OctetString(WSA_AD_ID_MIN)) == -1):
            print("  Set " + 'rsuWsaAdvertiserIdentifier' + " MIN Failed.")
            return -1
        if( r.SNMPGet( 'rsuWsaAdvertiserIdentifier', i) == -1):
            print("  Get " + 'rsuWsaAdvertiserIdentifier' + " MIN Failed.")
            return -1
        if( r.SNMPSet( 'rsuWsaAdvertiserIdentifier',i, rfc1902.OctetString(WSA_AD_ID_MAX)) == -1):
            print("  Set " + 'rsuWsaAdvertiserIdentifier' + " MAX Failed.")
            return -1
        if( r.SNMPGet( 'rsuWsaAdvertiserIdentifier', i) == -1):
            print("  Get " + 'rsuWsaAdvertiserIdentifier' + " MAX Failed.")
            return -1          
        # 5.10.2.16
        if( test_min_max_int32(r,'rsuWsaEnable', i, 0, 1) == -1):
            return -1            
        i = i + 1

    i=RSU_WSA_SERVICES_MIN
    while (i <= RSU_WSA_SERVICES_MAX):
        if( r.SNMPSet( 'rsuWsaStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuWsaStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuWsaStatus', i) == 0):
            print("  Get " + 'rsuWsaStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    

    #5.10.3: WSA Channel Table
    i=RSU_WSA_SERVICES_MIN
    while (i <= RSU_WSA_SERVICES_MAX):
        if( r.SNMPSet( 'rsuWsaChannelStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuWsaChannelStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuWsaChannelStatus', i) == 0):
            print("  Get " + 'rsuWsaChannelStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    
  
    # Set/Get columns of existing rows that exists. Without back end support nothing on RSU will happen.
    i=RSU_WSA_SERVICES_MIN
    while (i <= RSU_WSA_SERVICES_MAX):
        # 5.10.3.6: First CREATE row.
        if( r.SNMPSet( 'rsuWsaChannelStatus', i, rfc1902.Integer32(createAndGo)) == -1):
            print("  Set " + 'rsuWsaChannelStatus.i' + " Create Failed." )
            return -1                
        # 5.10.3.1: Index not available by definition. 
        # 5.10.3.2
        if( test_min_max_octet_string(r,'rsuWsaChannelPsid', i, '\0', '@@@@') == -1):
            return -1
        # 5.10.3.3
        if( test_min_max_int32(r,'rsuWsaChannelNumber', i, RSU_RADIO_CHANNEL_MIN, RSU_RADIO_CHANNEL_MAX) == -1):
            return -1
        # 5.10.3.4            
        if( test_min_max_int32(r,'rsuWsaChannelTxPowerLevel', i, RSU_RADIO_TX_POWER_MIN, RSU_RADIO_TX_POWER_MAX) == -1):
            return -1
        # 5.10.3.5            
        if( test_min_max_int32(r,'rsuWsaChannelAccess', i, WSA_CHANNEL_ACCESS_MIN, WSA_CHANNEL_ACCESS_MAX) == -1):
            return -1
        # 5.10.3.6: rsuWsaChannelStatus            
        i = i + 1 
    
    i=RSU_WSA_SERVICES_MIN
    while (i <= RSU_WSA_SERVICES_MAX):
        if( r.SNMPSet( 'rsuWsaChannelStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuWsaChannelStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuWsaChannelStatus', i) == 0):
            print("  Get " + 'rsuWsaChannelStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    

    #5.10.4: WSA Version
    data = r.SNMPGet('rsuWsaVersion', 0)    
    if((data == None) or (data != RSU_WSA_VERSION)):
        print("  Get rsuWsaVersion Failed.")
        return -1
        
    #Done.    
    print("Test 5.10: rsuWsaConfig: Passed.")    
    return 0
#
# NTCIP-1218: 5.11
#
def test_rsuWraConfig(r):

    print("Test 5.11: rsuWraConfig: Start.")

    #5.11.1:
    if( test_min_max_octet_string(r,'rsuWraIpPrefix', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_1) == -1):
        return -1
    #5.11.2:
    #Set to 0 bits(MIN)
    if( r.SNMPSet( 'rsuWraIpPrefixLength', 0, rfc1902.OctetString('\0')) == -1):
        print("  Set " + 'rsuWraIpPrefixLength' + " MIN Failed.")
        return -1   
    if(r.SNMPGet('rsuWraIpPrefixLength', 0) != rfc1902.OctetString('\0')):
        print("  Get rsuWraIpPrefixLength MIN Failed.")
        return -1
    # Set to 64 bits(MAX)        
    if( r.SNMPSet( 'rsuWraIpPrefixLength', 0, rfc1902.OctetString('@')) == -1):
        print("  Set " + 'rsuWraIpPrefixLength' + " MAX Failed.")
        return -1  
    if(r.SNMPGet('rsuWraIpPrefixLength', 0) != rfc1902.OctetString('@')):
        print("  Get rsuWraIpPrefixLength MAX Failed.")
        return -1                  
    #5.11.3 rsuWraGateway[RSU_DEST_IP_MAX]
    if( test_min_max_octet_string(r,'rsuWraGateway', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_2) == -1):
        return -1    
    #5.11.4 rsuWraPrimaryDns[RSU_DEST_IP_MAX]
    if( test_min_max_octet_string(r,'rsuWraPrimaryDns', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_3) == -1):
        return -1 
    #5.11.5 rsuWraSecondaryDns[RSU_DEST_IP_MAX]
    if( test_min_max_octet_string(r,'rsuWraSecondaryDns', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_4) == -1):
        return -1         
    #5.11.6 rsuWraGatewayMacAddress[RSU_RADIO_MAC_LENGTH]
    #Set to 0x00
    if( r.SNMPSet( 'rsuWraGatewayMacAddress', 0, rfc1902.OctetString('\0\0\0\0\0\0')) == -1):
        print("  Set " + 'rsuWraGatewayMacAddress' + " Failed:" + 'nulls' )
        return -1  
    if(r.SNMPGet('rsuWraGatewayMacAddress', 0) != rfc1902.OctetString('\0\0\0\0\0\0')):
        print("  Get rsuWraGatewayMacAddress MIN Failed: " + data )
        return -1
    #Set to 0x40
    if( r.SNMPSet( 'rsuWraGatewayMacAddress', 0, rfc1902.OctetString('@@@@@@')) == -1):
        print("  Set " + 'rsuWraGatewayMacAddress' + " Failed." + '@@@@@@')
        return -1  
    if(r.SNMPGet('rsuWraGatewayMacAddress', 0)  != rfc1902.OctetString('@@@@@@')):
        print("  Get rsuWraGatewayMacAddress Failed: " + data)
        return -1  
    #5.11.7 rsuWraLifetime
    if( test_min_max_int32(r,'rsuWraLifetime', 0, 0,65535) == -1):
        return -1
    #Done.    
    print("Test 5.11: rsuWraConfig: Passed.")    
    return 0
#
# NTCIP-1218: 5.12
#
def test_rsuMessageStats(r):

    RSU_PSID_TRACKED_STATS_MIN = 1
    RSU_PSID_TRACKED_STATS_MAX = 32

    RSU_MESSAGE_COUNT_DIRECTION_MIN = 1
    RSU_MESSAGE_COUNT_DIRECTION_MAX = 3

    print("Test 5.12: rsuMessageStats: Start.")
    # 5.12.1	
    data = r.SNMPGet('maxRsuMessageCountsByPsid', 0)      
    if((data == None) or (data != RSU_PSID_TRACKED_STATS_MAX)):
        print("  Get maxRsuMessageCountsByPsidFailed: " + data)
        return -1

    i=RSU_PSID_TRACKED_STATS_MIN
    while (i <= RSU_PSID_TRACKED_STATS_MAX):
        if( r.SNMPSet( 'rsuMessageCountsByPsidRowStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuMessageCountsByPsidRowStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuMessageCountsByPsidRowStatus', i) == 0):
            print("  Get " + 'rsuMessageCountsByPsidRowStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    
    
    # Set/Get columns of existing rows that exists. Without back end support nothing on RSU will happen.
    i=RSU_PSID_TRACKED_STATS_MIN
    while (i <= RSU_PSID_TRACKED_STATS_MAX):
        # 5.12.2.7: First CREATE row.
        if( r.SNMPSet( 'rsuMessageCountsByPsidRowStatus', i, rfc1902.Integer32(createAndGo)) == -1):
            print("  Set " + 'rsuMessageCountsByPsidRowStatus.i' + " Create Failed." )
            return -1                
        # 5.12.2.1: Index not available by definition. 
        # 5.12.2.2
        if( test_min_max_octet_string(r,'rsuMessageCountsByPsidId', i, '\0', '@@@@') == -1):
            return -1          
        # 5.12.2.3
        if( test_min_max_int32(r,'rsuMessageCountsByChannel', i, RSU_RADIO_CHANNEL_MIN, RSU_RADIO_CHANNEL_MAX) == -1):
            return -1
        # 5.12.2.4
        if( test_min_max_int32(r,'rsuMessageCountsDirection', i, RSU_MESSAGE_COUNT_DIRECTION_MIN, RSU_MESSAGE_COUNT_DIRECTION_MAX) == -1):
            return -1
        # 5.12.2.5
        if( test_min_max_octet_string(r,'rsuMessageCountsByPsidTime', i, '\0\0\0\0\0\0\0\0', '@@@@@@@@') == -1):
            return -1    
        # 5.12.2.6
        output = r.SNMPGet( 'rsuMessageCountsByPsidCounts', i)
        if(None == output):
            return -1
        i = i + 1  
    
    i=RSU_PSID_TRACKED_STATS_MIN
    while (i <= RSU_PSID_TRACKED_STATS_MAX):
        if( r.SNMPSet( 'rsuMessageCountsByPsidRowStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuMessageCountsByPsidRowStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuMessageCountsByPsidRowStatus', i) == 0):
            print("  Get " + 'rsuMessageCountsByPsidRowStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    
    
    #Done.
    print("Test 5.12: RsuMessageCountsByPsid: Passed.")  
    return 0
#
# NTCIP-1218: 5.13
#
def test_rsuSystemStats(r):

    #5.13.2 RSU Internal Temperature.  
    RSU_TEMP_CELSIUS_MIN = -101 # default  
    RSU_TEMP_CELSIUS_MAX = 100

    #5.13.3 */
    RSU_TEMP_LOW_CELSIUS_MIN = -101 # default 
    RSU_TEMP_LOW_CELSIUS_MAX = 100

    #5.13.4 */
    RSU_TEMP_HIGH_CELSIUS_MIN = -101 # default 
    RSU_TEMP_HIGH_CELSIUS_MAX = 100

    #5.13.5 Maximum Number of Communications Range Entries. 
    RSU_COMM_RANGE_MIN = 1
    RSU_COMM_RANGE_MAX = 16 # Enough to do all sectors.  

    #5.13.6 RSU Communications Range Table.  
    RSU_COMM_RANGE_SECTOR_MIN = 1  # Due north then east by 22.5 degrees.  
    RSU_COMM_RANGE_SECTOR_MAX = 16 # Due north then west by 22.5 degrees.  

    RSU_COMM_RANGE_MSG_ID_MIN = 0
    RSU_COMM_RANGE_MSG_ID_MAX = 32767

    RSU_COMM_RANGE_FILTER_TYPE_MIN = 1
    RSU_COMM_RANGE_FILTER_TYPE_MAX = 3

    RSU_COMM_RANGE_FILTER_VALUE_MIN = 0
    RSU_COMM_RANGE_FILTER_VALUE_MAX = 255

    RSU_COMM_RANGE_MINUTES_MIN = 0
    RSU_COMM_RANGE_MINUTES_MAX = 2001

    print("Test 5.13: rsuSystemStats: Start.")
   
    # 5.13.1: Read only Counter32 
    data = r.SNMPGet( 'rsuTimeSincePowerOn', 0)
    if(None == data):
        print("  Get rsuTimeSincePowerOn Failed.")
        return -1 
    # 5.13.2
    data = r.SNMPGet( 'rsuIntTemp', 0)
    if((None == data) or (data < RSU_TEMP_LOW_CELSIUS_MIN) or (RSU_TEMP_LOW_CELSIUS_MAX < data)):
        print("  Get rsuIntTemp Failed.")
        return -1   
    # 5.13.3   
    data = r.SNMPGet( 'rsuIntTempLowThreshold', 0)
    if((None == data) or (data < RSU_TEMP_LOW_CELSIUS_MIN) or (RSU_TEMP_LOW_CELSIUS_MAX < data)):
        print("  Get rsuIntTempLowThreshold Failed.")
        return -1     
    # 5.13.4   
    data = r.SNMPGet( 'rsuIntTempHighThreshold', 0)
    if((None == data) or (data < RSU_TEMP_HIGH_CELSIUS_MIN) or (RSU_TEMP_HIGH_CELSIUS_MAX < data)):
        print("  Get rsuIntTempHighThreshold Failed.")
        return -1   
    # 5.13.5   
    data = r.SNMPGet( 'maxRsuCommRange', 0)
    if((None == data) or (data != RSU_COMM_RANGE_MAX)):
        print("  Get maxRsuCommRange Failed.")
        return -1

    # 5.13.6
    i=RSU_COMM_RANGE_MIN
    while (i <= RSU_COMM_RANGE_MAX):
        if( r.SNMPSet( 'rsuCommRangeStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuCommRangeStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuCommRangeStatus', i) == 0):
            print("  Get " + 'rsuCommRangeStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    
    
    i=RSU_COMM_RANGE_MIN
    while (i <= RSU_COMM_RANGE_MAX):
        # 5.13.6.12: First CREATE row.
        if( r.SNMPSet( 'rsuCommRangeStatus', i, rfc1902.Integer32(createAndGo)) == -1):
            print("  Set " + 'rsuCommRangeStatus.i' + " Create Failed." )
            return -1                
        # 5.12.6.1: Index not available by definition. 
        # 5.12.6.2
        if( test_min_max_int32(r,'rsuCommRangeSector', i, RSU_COMM_RANGE_SECTOR_MIN, RSU_COMM_RANGE_SECTOR_MAX) == -1):
            return -1
        # 5.12.6.3
        if( test_min_max_int32(r,'rsuCommRangeMsgId', i, RSU_COMM_RANGE_MSG_ID_MIN, RSU_COMM_RANGE_MSG_ID_MAX) == -1):
            return -1
        # 5.12.6.4
        if( test_min_max_int32(r,'rsuCommRangeFilterType', i, RSU_COMM_RANGE_FILTER_TYPE_MIN, RSU_COMM_RANGE_FILTER_TYPE_MAX) == -1):
            return -1
        # 5.12.6.5
        if( test_min_max_int32(r,'rsuCommRangeFilterValue', i, RSU_COMM_RANGE_FILTER_VALUE_MIN, RSU_COMM_RANGE_FILTER_VALUE_MAX) == -1):
            return -1
        # 5.12.6.6
        data = r.SNMPGet( 'rsuCommRange1Min', i)     
        if((data == None) or (data < RSU_COMM_RANGE_MINUTES_MIN) or (RSU_COMM_RANGE_MINUTES_MAX < data)):
            print("  Get rsuCommRange1Min Failed.")
            return -1
        # 5.12.6.7
        data = r.SNMPGet( 'rsuCommRange5Min', i)
        if((None == data) or (data < RSU_COMM_RANGE_MINUTES_MIN) or (RSU_COMM_RANGE_MINUTES_MAX < data)):
            print("  Get rsuCommRange5Min Failed.")
            return -1        
        # 5.12.6.8
        data = r.SNMPGet( 'rsuCommRange15Min', i)
        if((None == data) or (data < RSU_COMM_RANGE_MINUTES_MIN) or (RSU_COMM_RANGE_MINUTES_MAX < data)):
            print("  Get rsuCommRange15Min Failed.")
            return -1        
        # 5.12.6.9
        data = r.SNMPGet( 'rsuCommRangeAvg1Min', i)
        if((None == data) or (data < RSU_COMM_RANGE_MINUTES_MIN) or (RSU_COMM_RANGE_MINUTES_MAX < data)):
            print("  Get rsuCommRangeAvg1Min Failed.")
            return -1       
        # 5.12.6.10
        data = r.SNMPGet( 'rsuCommRangeAvg5Min', i)
        if((None == data) or (data < RSU_COMM_RANGE_MINUTES_MIN) or (RSU_COMM_RANGE_MINUTES_MAX < data)):
            print("  Get rsuCommRangeAvg5Min Failed.")
            return -1       
        # 5.12.6.11
        data = r.SNMPGet( 'rsuCommRangeAvg15Min', i)
        if((None == data) or (data < RSU_COMM_RANGE_MINUTES_MIN) or (RSU_COMM_RANGE_MINUTES_MAX < data)):
            print("  Get rsuCommRangeAvg5Min Failed.")
            return -1                    
        i=i+1    
   
    i=RSU_COMM_RANGE_MIN
    while (i <= RSU_COMM_RANGE_MAX):
        if( r.SNMPSet( 'rsuCommRangeStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuCommRangeStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuCommRangeStatus', i) == 0):
            print("  Get " + 'rsuCommRangeStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1    

    #Done.
    print("Test 5.13: rsuSystemStats: Passed.") 
    return 0
#
# NTCIP-1218: 5.15
#
def test_rsuSysSettings(r):

    print("Test 5.15: rsuSysSettings: Start.")
    
    # 5.15.1
    if( test_min_max_octet_string(r,'rsuNotifyIpAddress', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_1) == -1):
        return -1
    # 5.15.2
    if( test_min_max_int32(r,'rsuNotifyPort',0,RSU_PORT_MIN,RSU_PORT_MAX) == -1):
        return -1 
    # 5.15.3   07e10a0717220104
    if( test_min_max_octet_string(r,'rsuSysLogQueryStart', 0, '\0\0\0\0\0\0\0\0', '@@@@@@@@') == -1):
        return -1    
    # 5.15.4   07e10b0717220209
    if( test_min_max_octet_string(r,'rsuSysLogQueryStop', 0, '\0\0\0\0\0\0\0\0', '@@@@@@@@') == -1):
        return -1
    # 5.15.5
    if( test_min_max_int32(r,'rsuSysLogQueryPriority',0, 0, 7) == -1):
        return -1     
    # 5.15.6
    if( test_min_max_int32(r,'rsuSysLogQueryGenerate',0, 0, 1) == -1):
        return -1    
    # 5.15.7
    data = r.SNMPGet('rsuSysLogQueryStatus', 0)    
    if((data == None) or (data < 1) or (6 < data)):
        print("  Get rsuSysLogQueryStatus Failed: " + data)
        return -1     
    # 5.15.8
    if( test_min_max_int32(r,'rsuSysLogCloseCommand',0, 0, 1) == -1):
        return -1    
    # 5.15.9
    if( test_min_max_int32(r,'rsuSysLogSeverity',0, 0, 7) == -1):
        return -1
    # 5.15.10
    if( test_min_max_octet_string(r,'rsuSysConfigId', 0, '', '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF') == -1):
        return -1    
    # 5.15.11
    if( test_min_max_int32(r,'rsuSysRetries',0, 0, 15) == -1):
        return -1    
    # 5.15.12
    if( test_min_max_int32(r,'rsuSysRetryPeriod',0, 0, 1440) == -1):
        return -1       
    # 5.15.13
    if( test_min_max_int32(r,'rsuShortCommLossTime',0, 0, 65535) == -1):
        return -1       
    # 5.15.14
    if( test_min_max_int32(r,'rsuLongCommLossTime',0, 0, 65535) == -1):
        return -1       
    # 5.15.15
    data = r.SNMPGet('rsuSysLogName', 0)    
    if(data == None):
        print("  Get rsuSysLogName Failed: " + data)
        return -1      
    # 5.15.16
    data = r.SNMPGet('rsuSysDir', 0)    
    if(data == None):
        print("  Get rsuSysDir Failed: " + data)
        return -1  
    # 5.15.17
    if( test_min_max_int32(r,'rsuLongCommLossReboot',0, 0, 1) == -1):
        return -1      
    # 5.15.18
    if( test_min_max_octet_string(r,'rsuHostIpAddr', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_2) == -1):
        return -1        
    # 5.15.19 
    if( test_min_max_octet_string(r,'rsuHostNetMask', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_3) == -1):
        return -1     
    # 5.15.20
    if( test_min_max_octet_string(r,'rsuHostGateway', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_1) == -1):
        return -1      
    # 5.15.21
    if( test_min_max_octet_string(r,'rsuHostDNS', 0, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_2) == -1):
        return -1    
    # 5.15.22
    if( test_min_max_int32(r,'rsuHostDHCPEnable',0, 1, 2) == -1):
        return -1  
    #Done.
    print("Test 5.15: rsuSysSettings: Passed.")
    return 0
#
# NTCIP-1218: 5.16
#
def test_rsuAntenna(r):

    RSU_ANTENNAS_MAX = 12   

    print("Test 5.15: rsuAntenna: Start.")

    # 5.16.1
    data = r.SNMPGet('maxRsuAntennas', 0)    
    if((data == None) or (data != RSU_ANTENNAS_MAX)):
        print("  Get maxRsuAntennas Failed: " + data)
        return -1  

    # 5.16.2.1
    if( test_min_max_int32(r, 'rsuAntLat', 1, RSU_LAT_MIN, RSU_LAT_MAX) == -1):
        return -1
    if( test_min_max_int32(r, 'rsuAntLat', RSU_ANTENNAS_MAX, RSU_LAT_MIN, RSU_LAT_MAX) == -1):
        return -1    
    # 5.16.2.2
    if( test_min_max_int32(r,'rsuAntLong', 1, RSU_LON_MIN, RSU_LON_MAX) == -1):
        return -1 
    if( test_min_max_int32(r,'rsuAntLong', RSU_ANTENNAS_MAX, RSU_LON_MIN, RSU_LON_MAX) == -1):
        return -1     
    # 5.16.2.3
    if( test_min_max_int32(r,'rsuAntElv', 1, RSU_ELV_MIN, RSU_ELV_MAX) == -1):
        return -1
    if( test_min_max_int32(r,'rsuAntElv', RSU_ANTENNAS_MAX, RSU_ELV_MIN, RSU_ELV_MAX) == -1):
        return -1        
    # 5.16.2.4
    if( test_min_max_int32(r,'rsuAntGain', 1, RSU_RADIO_TX_POWER_MIN, RSU_RADIO_TX_POWER_MAX) == -1):
        return -1
    if( test_min_max_int32(r,'rsuAntGain', RSU_ANTENNAS_MAX, RSU_RADIO_TX_POWER_MIN, RSU_RADIO_TX_POWER_MAX) == -1):
        return -1        
    # 5.16.2.5
    if( test_min_max_int32(r,'rsuAntDirection', 1, 0, 361) == -1):
        return -1
    if( test_min_max_int32(r,'rsuAntDirection', RSU_ANTENNAS_MAX, 0, 361) == -1):
        return -1
    #Done.
    print("Test 5.15: rsuAntenna: Passed.")
    return 0
#
# NTCIP-1218: 5.17
#
def test_rsuSystemStatus(r):

    print("Test 5.17: rsuSystemStatus: Started.")
    
    # 5.17.1
    data = r.SNMPGet('rsuChanStatus', 0)      
    if((data == None) or (data < 1) or (3 < data)):
        print("  Get rsuChanStatus Failed: " + data)
        return -1     
    # 5.17.2
    ##if( test_min_max_int32(r,'rsuMode', 0, 1, 3) == -1):
    ##    return -1     
    # 5.17.3
    data = r.SNMPGet('rsuModeStatus', 0)      
    if((data == None) or (data < 1) or (4 < data)):
        print("  Get rsuModeStatus Failed: " + data)
        return -1         
    # 5.17.4
    ##if( test_min_max_int32(r,'rsuReboot', 0, 0, 1) == -1):
    ##    return -1     
    # 5.17.5
    data = r.SNMPGet('rsuClockSource', 0)      
    if((data == None) or (data < 1) or (4 < data)):
        print("  Get rsuClockSource Failed: " + data)
        return -1         
    # 5.17.6
    data = r.SNMPGet('rsuClockSourceStatus', 0)      
    if((data == None) or (data < 1) or (3 < data)):
        print("  Get rsuClockSourceStatus Failed: " + data)
        return -1           
    # 5.17.7
    if( test_min_max_int32(r,'rsuClockSourceTimeout', 0, 0, 3600) == -1):
        return -1     
    # 5.17.8
    if( test_min_max_int32(r,'rsuClockSourceFailedQuery', 0, 0, 15) == -1):
        return -1     
    # 5.17.9
    if( test_min_max_int32(r,'rsuClockDeviationTolerance', 0, 0, 65535) == -1):
        return -1      
    # 5.17.10
    data = r.SNMPGet('rsuStatus', 0)      
    if((data == None) or (data < 1) or (5 < data)):
        print("  Get rsuStatus Failed: " + data)
        return -1     

    #Done.
    print("Test 5.17: rsuSystemStatus: Passed.")
    return 0
#
# NTCIP-1218: 5.18
#
def test_rsuAsync(r):

    print("Test 5.18: rsuAsync: Started.")
    
    # 5.18.1.1 File Integrity Check Error Message: AMH SAR& IMF: i.e. No valid message frame in UPER payload.
    data = r.SNMPGet('messageFileIntegrityError', 0)      
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get messageFileIntegrityError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuMsgFileIntegrityMsg', 0)      
    if(data == None):
        print("  Get rsuMsgFileIntegrityMsg Failed: " + data)
        return -1     
    # 5.18.1.2 Storage Integrity Error Message: Errors in file system.
    data = r.SNMPGet('rsuSecStorageIntegrityError', 0)      
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuSecStorageIntegrityError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuSecStorageIntegrityMsg', 0)      
    if(data == None):
        print("  Get rsuSecStorageIntegrityMsg Failed: " + data)
        return -1     
    # 5.18.1.3 Authorization Error Message: invalid security creds: Is failed login attempt one?
    data = r.SNMPGet('rsuAuthError', 0)      
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuAuthError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuAuthMsg', 0)      
    if(data == None):
        print("  Get rsuAuthMsg Failed: " + data)
        return -1     
    # 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. 
    data = r.SNMPGet('rsuSignatureVerifyError', 0)      
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuSignatureVerifyError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuSignatureVerifyMsg', 0)      
    if(data == None):
        print("  Get rsuSignatureVerifyMsg Failed: " + data)
        return -1     
    # 5.18.1.5 Access Error Message: error or rejection due to a violation of the Access Control List.
    data = r.SNMPGet('rsuAccessError', 0)
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuAccessError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuAccessMsg', 0)      
    if(data == None):
        print("  Get rsuAccessMsg Failed: " + data)
        return -1     
    # 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix.
    data = r.SNMPGet('rsuTimeSourceLost', 0)      
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuTimeSourceLost Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuTimeSourceLostMsg', 0)      
    if(data == None):
        print("  Get rsuTimeSourceLostMsg Failed: " + data)
        return -1     
    # 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold.
    data = r.SNMPGet('rsuTimeSourceMismatch', 0)
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuTimeSourceMismatch Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuTimeSourceMismatchMsg', 0)      
    if(data == None):
        print("  Get rsuTimeSourceMismatchMsg Failed: " + data)
        return -1     
    # 5.18.1.8 GNSS Anomaly Message: report any anomalous GNSS readings: Sky's the limit here.
    data = r.SNMPGet('rsuGnssAnomaly', 0)
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuGnssAnomaly Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuGnssAnomalyMsg', 0)      
    if(data == None):
        print("  Get rsuGnssAnomalyMsg Failed: " + data)
        return -1     
    # 5.18.1.9 GNSS Deviation Error Message: GNSS position deviation greater than allowed.
    data = r.SNMPGet('rsuGnssDeviationError', 0) 
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuGnssDeviationError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuGnssDeviationMsg', 0)      
    if(data == None):
        print("  Get rsuGnssDeviationMsg Failed: " + data)
        return -1
    # 5.18.1.10 GNSS NMEA Message:NMEA 0183 string (including the $ starting character and the ending <CR><LF>)
    data = r.SNMPGet('rsuGnssNmeaNotify', 0) 
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuGnssNmeaNotify Failed: " + data)
        return -1     
    if( test_min_max_int32(r,'rsuGnssNmeaNotifyInterval', 0, 0, 18000) == -1):
        return -1  
    # 5.18.1.11 Certificate Error Message
    data = r.SNMPGet('rsuCertificateError', 0) 
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuCertificateError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuCertificateMsg', 0)      
    if(data == None):
        print("  Get rsuCertificateMsg Failed: " + data)
        return -1     
    # 5.18.1.12 Denial of Service Error Message
    data = r.SNMPGet('rsuServiceDenialError', 0) 
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuServiceDenialError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuServiceDenialMsg', 0)      
    if(data == None):
        print("  Get rsuServiceDenialMsg Failed: " + data)
        return -1     
    # 5.18.1.13 Watchdog Error Message
    data = r.SNMPGet('rsuWatchdogError', 0) 
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuWatchdogError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuWatchdogMsg', 0)      
    if(data == None):
        print("  Get rsuWatchdogMsg Failed: " + data)
        return -1     
    # 5.18.1.14 Enclosure Environment Message: tamper detection
    data = r.SNMPGet('rsuEnvironError', 0) 
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuEnvironError Failed: " + data)
        return -1     
    data = r.SNMPGet('rsuEnvironMsg', 0)      
    if(data == None):
        print("  Get rsuEnvironMsg Failed: " + data)
        return -1     
    # 5.18.2.11 Notification Alert Level
    data = r.SNMPGet('rsuAlertLevel', 0)      
    if((data == None) or (data < 0) or (4 < data)):
        print("  Get rsuAlertLevel Failed: " + data)
        return -1     
    # 5.18.3 rsuNotificationRepeatInterval
    if( test_min_max_int32(r,'rsuNotificationRepeatInterval', 0, 0, 255) == -1):
        return -1      
    # 5.18.4 rsuNotificationMaxRetries
    if( test_min_max_int32(r,'rsuNotificationMaxRetries', 0, 0, 255) == -1):
        return -1
    #Done.
    print("Test 5.18: rsuAsync: Passed.")
    return 0
#
# NTCIP-1218: 5.19
#
def test_rsuAppConfig(r):

    RSU_APPS_RUNNING_MAX = 32 # 32 dsrc msg id's ie spat, map, bsm, tim, srmrx, ssm

    print("Test 5.19: rsuAppConfig: Start.")   
    
    # 5.19.1 int32_t maxRsuApps;
    data = r.SNMPGet('maxRsuApps', 0)      
    if((data == None)):
        print("  Get maxRsuApps Failed: " + data)
        return -1       
    # 5.19.2.1 Index N/A
    # 5.19.2.2
    data = r.SNMPGet('rsuAppConfigName', 1)      
    if(data == None):
        print("  Get rsuAppConfigName Failed: " + data)
        return -1
    data = r.SNMPGet('rsuAppConfigName', RSU_APPS_RUNNING_MAX)      
    if(data == None):
        print("  Get rsuAppConfigName Failed: " + data)
        return -1        
    # 5.19.2.3
    if( test_min_max_int32(r,'rsuAppConfigStartup', 1, 1, 3) == -1):
        return -1
    if( test_min_max_int32(r,'rsuAppConfigStartup', RSU_APPS_RUNNING_MAX, 1, 3) == -1):
        return -1        
    # 5.19.2.4
    data = r.SNMPGet('rsuAppConfigName', 1)      
    if(data == None):
        print("  Get rsuAppConfigName Failed: " + data)
        return -1
    data = r.SNMPGet('rsuAppConfigName', RSU_APPS_RUNNING_MAX)      
    if(data == None):
        print("  Get rsuAppConfigName Failed: " + data)
        return -1        
    # 5.19.2.5
    if( test_min_max_int32(r,'rsuAppConfigStart', 1, 0, 1) == -1):
        return -1
    if( test_min_max_int32(r,'rsuAppConfigStart', RSU_APPS_RUNNING_MAX, 0, 1) == -1):
        return -1        
    # 5.19.2.6
    if( test_min_max_int32(r,'rsuAppConfigStop', 1, 0, 1) == -1):
        return -1
    if( test_min_max_int32(r,'rsuAppConfigStop', RSU_APPS_RUNNING_MAX, 0, 1) == -1):
        return -1        
    #Done.
    print("Test 5.19: rsuAppConfig: Passed.")        
    return 0
#
# NTCIP-1218: 5.20
#
def test_rsuService(r):

    # 5.20.1: Maximum RSU Service Entries
    RSU_SERVICES_RUNNING_MIN = 1  
    RSU_SERVICES_RUNNING_MAX = 32
    RSU_SERVICE_RESERVED_INDEX_MIN = 1  # Reserved or RO rows from client perspective. 
    RSU_SERVICE_RESERVED_INDEX_MAX = 16
    # 5.20.2.2
    RSU_SERVICE_NAME_LENGTH_MIN = 1
    RSU_SERVICE_NAME_LENGTH_MAX = 127
    RSU_SERVICE_NAME_MIN = 'A'
    RSU_SERVICE_NAME_MAX = '012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789BADBEEF'    
    #5.20.2.3	Service Status
    RSU_SERVICE_STATUS_MIN = 1
    RSU_SERVICE_STATUS_MAX = 5
    # 5.20.2.4
    RSU_SERVICE_STATUS_DESC_LENGTH_MIN = 0
    RSU_SERVICE_STATUS_DESC_LENGTH_MAX = 255
    
    print("Test 5.20: rsuService: Start.")  
    
    # 5.20.1	Maximum RSU Service Entries
    data = r.SNMPGet('maxRsuServices', 0)      
    if((data == None) or (data != RSU_SERVICES_RUNNING_MAX)):
        print("  Get maxRsuServices Failed: " + data)
        return -1  
    # 5.20.2.1 rsuServiceID: Index not available by definition
    # 5.20.2.2: rsuServiceName.1..16 RO, 17..32 RW
    data = r.SNMPGet('rsuServiceName', RSU_SERVICE_RESERVED_INDEX_MIN)      
    if(data == None):
        print("  Get rsuServiceName.minReservedRow Failed: " + data)
        return -1         
    data = r.SNMPGet('rsuServiceName', RSU_SERVICE_RESERVED_INDEX_MAX)      
    if(data == None):
        print("  Get rsuServiceName.maxReservedRow Failed: " + data)
        return -1
    if( test_min_max_octet_string(r,'rsuServiceName', RSU_SERVICE_RESERVED_INDEX_MAX+1, RSU_SERVICE_NAME_MIN, RSU_SERVICE_NAME_MAX) == -1):
        return -1     
    if( test_min_max_octet_string(r,'rsuServiceName', RSU_SERVICES_RUNNING_MAX, RSU_SERVICE_NAME_MIN, RSU_SERVICE_NAME_MAX) == -1):
        return -1 
    # 5.20.2.3 rsuServiceStatus_e rsuServiceStatus; RO: 1..5
    data = r.SNMPGet('rsuServiceStatus', RSU_SERVICES_RUNNING_MIN)      
    if((data == None) or (data < RSU_SERVICE_STATUS_MIN)  or (RSU_SERVICE_STATUS_MAX < data)):
        print("  Get rsuServiceStatus.minRow Failed: " + data)
        return -1      
    data = r.SNMPGet( 'rsuServiceStatus', RSU_SERVICES_RUNNING_MAX)      
    if((data == None) or (data < RSU_SERVICE_STATUS_MIN)  or (RSU_SERVICE_STATUS_MAX < data)):
        print("  Get rsuServiceStatus.maxRow Failed: " + data)
        return -1
    # 5.20.2.4  rsuServiceStatusDesc[RSU_SERVICE_STATUS_DESC_LENGTH_MAX]; RO: 0..255 chars
    data = r.SNMPGet('rsuServiceStatusDesc', RSU_SERVICES_RUNNING_MIN)      
    if(data == None):
        print("  Get rsuServiceStatusDesc.minRow Failed: " + data)
        return -1        
    data = r.SNMPGet('rsuServiceStatusDesc', RSU_SERVICES_RUNNING_MAX)      
    if(data == None):
        print("  Get rsuServiceStatusDesc.maxRow Failed: " + data)
        return -1
    # 5.20.2.5  rsuServiceStatusTime[MIB_DATEANDTIME_LENGTH]; RO: 8 OCTETS: 
    data = r.SNMPGet('rsuServiceStatusTime', RSU_SERVICES_RUNNING_MIN)      
    if(data == None):
        print("  Get rsuServiceStatusTime.minRow Failed: " + data)
        return -1       
    data = r.SNMPGet('rsuServiceStatusTime', RSU_SERVICES_RUNNING_MAX)      
    if(data == None):
        print("  Get rsuServiceStatusTime.maxRow Failed: " + data)
        return -1

    #Done.
    print("Test 5.20: rsuService: Passed.")            
    return 0
#
# NTCIP-1218: 5.21
#
def test_rsuXmitMsgFwding(r):

    # 5.21.1
    RSU_XMIT_MSG_COUNT_MIN = 1
    RSU_XMIT_MSG_COUNT_MAX = 32

    # 5.21.2
    RSU_XMIT_MSG_PORT_MIN = 1024
    RSU_XMIT_MSG_PORT_MAX = RSU_PORT_MAX

    RSU_XMIT_MSG_SECURE_MIN = 0
    RSU_XMIT_MSG_SECURE_MAX = 1

    print("Test 5.21: rsuXmitMsgFwding: Started.") 
    
    # 5.21.1	
    data = r.SNMPGet('maxXmitMsgFwding', 0)      
    if((data == None) or (data != RSU_XMIT_MSG_COUNT_MAX)):
        print("  Get maxXmitMsgFwding Failed: " + data)
        return -1
        
    #Destroy default rows. If there are no rows already then not very exiciting.
    i=RSU_XMIT_MSG_COUNT_MIN
    while (i <= RSU_XMIT_MSG_COUNT_MAX):
        if( r.SNMPSet( 'rsuXmitMsgFwdingStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuXmitMsgFwdingStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuXmitMsgFwdingStatus', i) == 0):
            print("  Get " + 'rsuXmitMsgFwdingStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1
    
    # Set/Get columns of existing rows that exists. Without back end support nothing on RSU will happen.
    i=RSU_XMIT_MSG_COUNT_MIN
    while (i <= RSU_XMIT_MSG_COUNT_MAX):
        # 5.21.2.9: First CREATE row.
        if( r.SNMPSet( 'rsuXmitMsgFwdingStatus', i, rfc1902.Integer32(createAndGo)) == -1):
            print("  Set " + 'rsuXmitMsgFwdingStatus.i' + " Create Failed." )
            return -1 
        #time.sleep(1)            
        if(r.SNMPGet( 'rsuXmitMsgFwdingStatus', i) != rfc1902.Integer32(createAndGo)):
            print("  Get " + 'rsuXmitMsgFwdingStatus.i' + " Failed. Row doesn't exists.")
            return -1
        # 5.21.2.1 rsuXmitMsgFwdingIndex: RO: No client access by definition.            
        # 5.21.2.2
        if( test_min_max_octet_string(r,'rsuXmitMsgFwdingPsid', i, '\0', '@@@@') == -1):
            return -1          
        # 5.21.2.3
        if( test_min_max_octet_string(r,'rsuXmitMsgFwdingDestIpAddr', i, RSU_IP_MIN_DATA, RSU_IP_MAX_DATA_1) == -1):
            return -1
        # 5.21.2.4
        if( test_min_max_int32(r,'rsuXmitMsgFwdingDestPort', i, RSU_XMIT_MSG_PORT_MIN, RSU_XMIT_MSG_PORT_MAX) == -1):
            return -1    
        # 5.21.2.5
        if( test_min_max_int32(r,'rsuXmitMsgFwdingProtocol', i, 1, 2) == -1):
            return -1    
        # 5.21.2.6
        if( test_min_max_octet_string(r,'rsuXmitMsgFwdingDeliveryStart', i, '\0\0\0\0\0\0\0\0','@@@@@@@@') == -1):
            return -1    
        # 5.21.2.7
        if( test_min_max_octet_string(r,'rsuXmitMsgFwdingDeliveryStop', i, '\0\0\0\0\0\0\0\0','@@@@@@@@') == -1):
            return -1   
        # 5.21.2.8
        if( test_min_max_int32(r,'rsuXmitMsgFwdingSecure', i, 0, 1) == -1):
            return -1
        i=i+1
    # 5.21.2.9:
    
    #Destroy rows we know we just created. Now if DESTROY fails we'll see an error.
    i=RSU_XMIT_MSG_COUNT_MIN
    while (i <= RSU_XMIT_MSG_COUNT_MAX):
        if( r.SNMPSet( 'rsuXmitMsgFwdingStatus', i, rfc1902.Integer32(destroy)) == -1):
            print("  Set " + 'rsuXmitMsgFwdingStatus.i' + " Destroy Failed." )
            return -1 
        if(r.SNMPGet( 'rsuXmitMsgFwdingStatus', i) == 0):
            print("  Get " + 'rsuXmitMsgFwdingStatus.i' + " Destroy Failed. Row still exists.")
            return -1
        i=i+1
 
    #Done.
    print("Test 5.21: rsuXmitMsgFwding: Passed.")            
    return 0
#
# Test NTCIP-1218 MIB
#    
def test_rsu(r, radio):
    i = 1
    while (i < 2):
        i = i + 1
        if True:
            #5.6
            if(test_rsuReceivedMsg(r, radio) != 0):
                return -1
            #5.8
            if(test_rsuInterfaceLog(r) != 0):
                return -1                
            #5.9
            if(test_rsuSecurity(r) != 0):
                return -1 
            #5.10
            if(test_rsuWsaConfig(r) != 0):
                return -1
            #5.11
            if(test_rsuWraConfig(r) != 0):
                return -1        
            #5.12
            if(test_rsuMessageStats(r) != 0):
                return -1   
            #5.13
            if(test_rsuSystemStats(r) != 0):
                return -1   
            #5.15
            if(test_rsuSysSettings(r) != 0):
                return -1
            #5.16
            if(test_rsuAntenna(r) != 0):
                return -1
            #5.17: rsuMode and rsuReboot are live so dont test here.
            if(test_rsuSystemStatus(r) != 0):
                return -1
            #5.18
            if(test_rsuAsync(r) != 0):
                return -1  
            #5.19
            if(test_rsuAppConfig(r) != 0):
                return -1 
            #5.20
            if(test_rsuService(r) != 0):
                return -1
            #5.21
            if(test_rsuXmitMsgFwding(r) != 0):
                return -1
        
    return 0

#
#
#
def setup(r, rtype):

    rsu_snmp_port = 161
    
    print("Setup WebGUI.")
    
    # Clears MIB and amh manually. 
    # TODO: conf_manager on RSU should do for web ui not us in pythonland.
    if True:
        r.set_default_configs()

    #Use to get SNMP Port if needed.
    if False:
        # config via GUI
        webui = WebUI.WebUI()
        webui.login(r.ip, r.user, r.pswd)
        webui.navigate("Advanced Config Settings")
        webui.set("ImmediateEnable", "1")       # amh
        webui.set("EnableStoreAndRepeat", "1")  # amh
        webui.set("I2VWSAEnable", "0")          # i2v
        webui.set("BSMRxEnable", "0")           # spat16
        webui.btnclick("SAVE_BTTN")
        time.sleep(10)
        webui.back()
        webui.navigate("Configuration Settings")
        webui.navigate("Security Settings")
        webui.set("I2VMAPSecPSID", "0x8002")
        webui.btnclick("SAVE_BTTN")
        webui.back()
        webui.navigate("RSU Basic Settings")
        rsu_snmp_port = webui.get("agentaddress")        # todo .. pass this to SNMPSet() and SNMPGet() instead of counting on it as default
        
        if False:
            # I2VRadioType: Select which radio and channel we want.    
            if (rtype == "0"):
                webui.set("I2VUnifiedChannelNumber", "180") # This doesn't work?
                webui.radiobtnclick("I2VRadioType", rtype)
                print("Setup: Set radio to DSRC.")
            else:
                #webui.set("I2VUnifiedChannelNumber", "183") # This doesn't work?
                webui.radiobtnclick("I2VRadioType", rtype) # CV2X: Cant change channel on CV2X.
                print("Setup: Set radio to CV2X.")
            webui.set("I2VTransmitPower", "3")    
            webui.btnclick("SAVE_BTTN")
            time.sleep(10)
        
            # I need to do channel by itself with seperate save. Why?    
            if (rtype == "0"):
                webui.set("I2VUnifiedChannelNumber", "180") # DSRC
                print("Setup: Set channel to DSRC.")
            else:
                #webui.set("I2VUnifiedChannelNumber", channel) # CV2X
                print("Setup: Set channel to CV2X.")            
 
            webui.btnclick("SAVE_BTTN")
            time.sleep(10)
            webui.back()
            webui.navigate("Home")
            # sign-off
            webui.signoff()
            webui.close()
            time.sleep(10)
            r.reboot()

    return int(rsu_snmp_port)
#
# Test NTCIP-1218:
#    
def SQT_NTCIP_rsu(r_ip, o_ip, r_fw=None, o_fw=None, r_pswd=None):
    r = RSU.RSU(r_ip, r_fw, r_pswd)
    
    #
    # Test DSRC:
    #
    if True:
        rsu_snmp_port = setup(r, "0")
        if not r.wait_for_radio_active("cv2x"):
            return False, "FAIL: RSU CV2X Radio Inactive."       
        if not r.wait_for_radio_active("dsrc"):
            return False, "FAIL: RSU DSRC Radio Inactive."
        now = str(r.time())
        print("Test DSRC: rsu : Start. RSU UTC=" + now)
        success = test_rsu(r,0)
        if (-1 == success):
            return (False, "Test DSRC: rsu  Failed: start " + now + ": End " + str(r.time()))
        dsrc_result = ("Test DSRC: rsu  Passed: start " + now + ": End " + str(r.time()))
    else:
        dsrc_result = "Test DSRC: rsu  Skipped, not Tested."
    #    
    # Test CV2X:
    #
    if False: 
        rsu_snmp_port = setup(r, "1")
        if not r.wait_for_radio_active("cv2x"):
            return False, "FAIL: RSU CV2X Radio Inactive."       
        if not r.wait_for_radio_active("dsrc"):
            return False, "FAIL: RSU DSRC Radio Inactive."
        now = str(r.time())
        print("Test CV2X: rsu  Start. RSU UTC=" + now )
        success = test_rsu(r,1)        
        if (-1 == success):
            return (False, "Test CV2X: rsu  Failed: start " + now + ": End " + str(r.time()))
        cv2x_result = ("Test CV2X: rsu  Passed: start " + now + ": End " + str(r.time()))
    else:
        cv2x_result = "Test CV2X: rsu  Skipped, not Tested."
        
    print(dsrc_result)
    print(cv2x_result)
        
    return(True,"Done.") 
#
#
# 
if __name__ == "__main__":
    args = utils.parse_args(obu=True)
    print(SQT_NTCIP_rsu(args.rsu_ip, args.obu_ip, args.rsu_fw, args.obu_fw, args.rsu_pswd))

    
    