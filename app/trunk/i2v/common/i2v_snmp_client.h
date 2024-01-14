/**************************************************************************
 *                                                                        *
 *     File Name: i2v_snmp_client.h                                       *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#ifndef I2V_SNMP_CLIENT_H
#define I2V_SNMP_CLIENT_H

/* see i2v_snmp_client.c for an explanation of what is defined here */

#define I2V_SNMP_MAX_STR_LEN          100
#define NETSNMP_DS_APP_DONT_FIX_PDUS  0

/* error codes - mirrored from snmpfunc */
#define I2V_SNMP_WRONG_USAGE   -1
#define I2V_SNMP_SOCK_ERROR    -2
#define I2V_SNMP_INVALID_OID   -3
#define I2V_SNMP_ERR_TIMEOUT   -4
#define I2V_SNMP_ERR_OTHER     -5
#define I2V_SNMP_PKT_ERROR     -6

/* netsnmp asn1.h defines - modified slightly to avoid collision
   with snmpfunc.h in case both headers included in a file - 
   ALWAYS include this one after snmpfunc.h if including both -
   not all of these defines supported by the code currently */
#ifndef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
#define NETSNMP_WITH_OPAQUE_SPECIAL_TYPES 1
#endif /* NETSNMP_WITH_OPAQUE_SPECIAL_TYPES */

/* consider adding SNMP into prefix - i2v may need the ASN types */
#define I2V_ASN_BOOLEAN         ((unsigned char)0x01)
#define I2V_ASN_INTEGER         ((unsigned char)0x02)
#define I2V_ASN_BIT_STR         ((unsigned char)0x03)
#define I2V_ASN_OCTET_STR       ((unsigned char)0x04)
#define I2V_ASN_NULL            ((unsigned char)0x05)
#define I2V_ASN_OBJECT_ID       ((unsigned char)0x06)
#define I2V_ASN_SEQUENCE        ((unsigned char)0x10)
#define I2V_ASN_SET             ((unsigned char)0x11)

#define I2V_ASN_UNIVERSAL       ((unsigned char)0x00)
#define I2V_ASN_APPLICATION     ((unsigned char)0x40)
#define I2V_ASN_CONTEXT         ((unsigned char)0x80)
#define I2V_ASN_PRIVATE         ((unsigned char)0xC0)

#define I2V_ASN_PRIMITIVE       ((unsigned char)0x00)
#define I2V_ASN_CONSTRUCTOR     ((unsigned char)0x20)

#define I2V_ASN_LONG_LEN        (0x80)
#define I2V_ASN_EXTENSION_ID    (0x1F)
#define I2V_ASN_BIT8            (0x80)

#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
#define I2V_ASN_OPAQUE_TAG2     ((unsigned char)0x30)

#define I2V_ASN_APP_OPAQUE      (I2V_ASN_APPLICATION | 4)
#define I2V_ASN_APP_COUNTER64   (I2V_ASN_APPLICATION | 6)
#define I2V_ASN_APP_FLOAT       (I2V_ASN_APPLICATION | 8)
#define I2V_ASN_APP_DOUBLE      (I2V_ASN_APPLICATION | 9)

#define I2V_ASN_COUNTER64       (I2V_ASN_OPAQUE_TAG2 + I2V_ASN_APP_COUNTER64)
#define I2V_ASN_FLOAT           (I2V_ASN_OPAQUE_TAG2 + I2V_ASN_APP_FLOAT)
#define I2V_ASN_DOUBLE          (I2V_ASN_OPAQUE_TAG2 + I2V_ASN_APP_DOUBLE)
#endif /* NETSNMP_WITH_OPAQUE_SPECIAL_TYPES */

/* netsnmp asn1.h struct */
#if !defined NET_SNMP_TYPES_H && !defined ASN1_H
struct counter64 {
    unsigned long    high;
    unsigned long    low;
};
#endif /* !NET_SNMP_TYPES_H && !ASN1_H */

typedef union {
    long             integer;
    unsigned char    string[I2V_SNMP_MAX_STR_LEN];    /* FIXME: this works for immediate implementation
                                                        but for future revisions strings may be longer
                                                        depending on the SNMP object */
    /* unsigned char    objid[I2V_SNMP_MAX_STR_LEN];
    unsigned char    bitstring[I2V_SNMP_MAX_STR_LEN]; */  /* these are duplicates and not needed - covered by string */
    struct counter64 counter64;
#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
    float            floatVal;
    double           doubleVal;
#endif /* NETSNMP_WITH_OPAQUE_SPECIAL_TYPES */
} i2vSnmpData;


int i2vSnmpGet(int numParams, char *paramBuff[], unsigned char *type, i2vSnmpData *val);

#endif /* I2V_SNMP_CLIENT_H */
