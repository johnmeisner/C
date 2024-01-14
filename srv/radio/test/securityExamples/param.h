#ifndef _SECURITY_EXAMPLES_PARAM_H_
#define _SECURITY_EXAMPLES_PARAM_H_

#define CHANNEL            172
#define UNSECURED_WSM_PSID 0x20
#define SIGNED_WSM_PSID    0x20
#if defined(HEAVY_DUTY)
#define SERVICE_PSID       0xeffffd01
#else
#define SERVICE_PSID       e0000009
#endif
#define SERVICE_PSID2      0x26

#endif // _SECURITY_EXAMPLES_PARAM_H_

