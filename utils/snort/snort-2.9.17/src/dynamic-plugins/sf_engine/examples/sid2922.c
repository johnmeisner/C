/*
 * VRT RULES
 *
 * Copyright (C) 2014-2020 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * This file is autogenerated via rules2c, by Brian Caswell <bmc@sourcefire.com>
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


/* declare detection functions */
int rule2922eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_server; */
static FlowFlags rule2922flow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule2922option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &rule2922flow0 }
};
/* byte_test:size 1, value 16, operator <, offset 2; */
static ByteData rule2922byte_test1 =
{
    1, /* size */
    CHECK_LT, /* operator */
    16, /* value */
    2, /* offset */
    0, /*multiplier */
    BYTE_BIG_ENDIAN|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE, /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule2922option1 =
{
    OPTION_TYPE_BYTE_TEST,
    { &rule2922byte_test1 }
};
/* byte_test:size 1, value 8, operator &, offset 2; */
static ByteData rule2922byte_test2 =
{
    1, /* size */
    CHECK_AND, /* operator */
    8, /* value */
    2, /* offset */
    0, /*multiplier */
    BYTE_BIG_ENDIAN|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE, /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule2922option2 =
{
    OPTION_TYPE_BYTE_TEST,
    { &rule2922byte_test2 }
};

/* references for sid 2922 */
/* reference: bugtraq "2302"; */
static RuleReference rule2922ref1 =
{
    "bugtraq", /* type */
    "2302" /* value */
};

/* reference: cve "2001-0010"; */
static RuleReference rule2922ref2 =
{
    "cve", /* type */
    "2001-0010" /* value */
};

static RuleReference *rule2922refs[] =
{
    &rule2922ref1,
    &rule2922ref2,
    NULL
};
RuleOption *rule2922options[] =
{
    &rule2922option0,
    &rule2922option1,
    &rule2922option2,
    NULL
};

Rule rule2922 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       1, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "53", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       2922, /* sigid */
       1, /* revision */

       "attempted-recon", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "DNS TCP inverse query",     /* message */
       rule2922refs, /* ptr to references */
       NULL /* metadata */
   },
   rule2922options, /* ptr to rule options */
   NULL, // &rule2922eval, /* use the built in detection function */
   0, /* am I initialized yet? */
   0,                                  /* Rule option count, used internally */
   0,                                  /* Flag with no alert, used internally */
   NULL /* ptr to internal data... setup during rule registration */
};


/* detection functions */
int rule2922eval(void *p) {
    //u_int8_t *cursor_uri = 0;
    //u_int8_t *cursor_raw = 0;
    u_int8_t *cursor_normal = 0;


    // flow:established, to_server;
    if (checkFlow(p, rule2922options[0]->option_u.flowFlags) > 0 ) {
        // byte_test:size 1, value 16, operator <, offset 2;
        if (byteTest(p, rule2922options[1]->option_u.byte, cursor_normal) > 0) {
            // byte_test:size 1, value 8, operator &, offset 2;
            if (byteTest(p, rule2922options[2]->option_u.byte, cursor_normal) > 0) {
                return RULE_MATCH;
            }
        }
    }
    return RULE_NOMATCH;
}
