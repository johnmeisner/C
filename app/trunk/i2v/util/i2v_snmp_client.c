/**************************************************************************
 *                                                                        *
 *     File Name:  i2v_snmp_client.c                                      *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/*
 * This library exists solely because the author believes the snmpfunc implementation
 * has an error in its implementation.  The snmpfunc implementation makes use of
 * pointers for returning the values retrieved from an snmpget but the pointers
 * reference memory that is dynamically allocated and deallocated before the function
 * exits - so any successful call is only sheer luck since the memory referenced
 * hasn't been reused and the values are retained.  The primary focus of this library
 * is to provide a reliable client call to get snmp objects without introducing
 * blind luck.
 */
/* FIXME: maybe consider adding an i2v function for setting snmp vals instead
   of using snmpfunc - this would eliminate need for having both */

/* duplicates some code from snmpfunc.c which has a CMU copyright - see 
   snmpfunc.c for full license information - not sure where snmpfunc
   originated */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/utilities.h>
#include <net-snmp/net-snmp-includes.h>
#include "dn_types.h"
#include "i2v_snmp_client.h"

#if defined(MY_UNIT_TEST)
#define DN_SOCK_STARTUP
#define DN_SOCK_CLEANUP
#else
#define DN_SOCK_STARTUP  SOCK_STARTUP 
#define DN_SOCK_CLEANUP  SOCK_CLEANUP
#endif
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif
/* 20180216: QNX SNMP memory leak - leak improved when the session was held open
   instead of closed and reopened with each snmp command; however leak still
   persists; still under investigation; possibly temporary declaration for
   session
*/
STATIC    netsnmp_session        session, *ss = NULL;

STATIC void i2vSnmpParseDefault(int __attribute__((unused)) numParams, 
            char __attribute__((unused)) *const *paramBuff, int __attribute__((unused)) opt)
{
#ifdef EXTRA_DEBUG
    printf("received random opt: %c\n", opt);
#endif /* EXTRA_DEBUG */
    /* may want to add support in the future for flag defined in snmpfunc */
    /* nop for now */
    ;
}

int i2vSnmpGet(int numParams, char *paramBuff[], unsigned char *type, i2vSnmpData *val)
{
    netsnmp_pdu           *pdu;
    netsnmp_pdu           *response;
    netsnmp_variable_list *vars;
    int                    arg;
    int                    count;
    int                    current_name = 0;
    char                  *names[SNMP_MAX_CMDLINE_OIDS];
    oid                    name[MAX_OID_LEN];
    size_t                 name_length;
    int                    status;
    int                    failures = 0;
    int                    exitval = 0;
    
    switch (arg = snmp_parse_args(numParams, paramBuff, &session, NULL, i2vSnmpParseDefault)) {
        case -2:
        case -1:
            return I2V_SNMP_WRONG_USAGE;
        default:
            break;
    }

    if (arg >= numParams) {
        return I2V_SNMP_WRONG_USAGE;
    }
    if ((numParams - arg) > SNMP_MAX_CMDLINE_OIDS) {
        return I2V_SNMP_WRONG_USAGE;
    }

    /* get the object names */
    for (; arg < numParams; arg++) {
        names[current_name++] = paramBuff[arg];
    }

    DN_SOCK_STARTUP;

    /* Open an SNMP session. */
    if (NULL == ss) {
        ss = snmp_open(&session);
    }
    if (ss == NULL) {
        /* diagnose snmp_open errors with the input netsnmp_session pointer */
        snmp_sess_perror("snmpget", &session);
        DN_SOCK_CLEANUP;
        return I2V_SNMP_SOCK_ERROR;
    }


    /* Create PDU for GET request and add object names to request. */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    for (count = 0; count < current_name; count++) {
        name_length = MAX_OID_LEN;
        if (!snmp_parse_oid(names[count], name, &name_length)) {
            snmp_perror(names[count]);
            failures++;
        } else {
//TM TODO: last one is always OID. why? If not then unit test will fail.
            snmp_add_null_var(pdu, name, name_length);
        }
    }
    if (failures) {
        /* snmp_close(ss); */  /* memory leak test point */
        DN_SOCK_CLEANUP;
        return I2V_SNMP_INVALID_OID;
    }

    /* Perform the request. If the Get Request fails, note the OID 
       that caused the error, "fix" the PDU (removing the error-prone 
       OID) and retry. */
#if 0  /* see comment below about code not to use */
retry:
#endif /* if 0 */
    status = snmp_synch_response(ss, pdu, &response);
    if (status == STAT_SUCCESS) {
        if (response->errstat == SNMP_ERR_NOERROR) {
            vars = response->variables;
            if (vars) {
                *type = vars->type;
                /* i2v enhancement - avoid blind luck */
                memset(val, 0, sizeof(i2vSnmpData));
                switch (*type) {
                    case I2V_ASN_BOOLEAN:
                    case I2V_ASN_INTEGER:
                    case I2V_ASN_SET:
                        val->integer = *(vars->val.integer);
                        break;
                    case I2V_ASN_BIT_STR:
                    case I2V_ASN_OCTET_STR:
                    case I2V_ASN_OBJECT_ID:
                    case I2V_ASN_SEQUENCE:
                        /* will only support up to I2V_SNMP_MAX_STR_LEN for now - leaving last byte
                           as null for a real 'string' */
                        memcpy(val->string, vars->val.string, (vars->val_len > (I2V_SNMP_MAX_STR_LEN - 1)) ? 
                               (I2V_SNMP_MAX_STR_LEN - 1) : vars->val_len);
                        break;
#if !defined(MY_UNIT_TEST)
                    case I2V_ASN_COUNTER64:
                        memcpy(&val->counter64, vars->val.counter64, sizeof(struct counter64));
                        break;
#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
                    case I2V_ASN_FLOAT:
                        val->floatVal = *(vars->val.floatVal);
                        break;
                    case I2V_ASN_DOUBLE:
                        val->doubleVal = *(vars->val.doubleVal);
                        break;
#endif /* NETSNMP_WITH_OPAQUE_SPECIAL_TYPES */
#endif /* MY_UNIT_TEST */
                    default:   /* this covers null since memset done before switch */
                        break;
                }
            }
        } else {
            exitval = I2V_SNMP_PKT_ERROR;

            /* this code is left for possible use in the future but
               it should never be called */
#if 0   /* JJG - see comment above, why was this left operational? now commented out */
            /* retry if the errored variable was successfully removed */
            if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
					NETSNMP_DS_APP_DONT_FIX_PDUS)) {
                pdu = snmp_fix_pdu(response, SNMP_MSG_GET);
                snmp_free_pdu(response);
                response = NULL;
                if (pdu != NULL) {
                    goto retry;
                }
            }
#endif /* if 0 */
        } /* endif -- SNMP_ERR_NOERROR */
    } else if (status == STAT_TIMEOUT) {
        exitval = I2V_SNMP_ERR_TIMEOUT;
    } else {     /* status == STAT_ERROR */
        snmp_sess_perror("snmpget", ss);
        exitval = I2V_SNMP_ERR_OTHER;
    }  /* endif -- STAT_SUCCESS */

    if (response) {
        snmp_free_pdu(response);
    }
    /* snmp_close(ss); */  /* memory leak test point */
    DN_SOCK_CLEANUP;
    return exitval;

}
