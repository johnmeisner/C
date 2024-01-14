#ifndef _RTXCONTEXT_HH_
#define _RTXCONTEXT_HH_

#define OSCTXTINIT 0x1aa2a34a

/* bits of rtxCheckLicense */
#define LIC_RT     0x00
#define LIC_XER    0x01
#define LIC_PRO    0x02
#define LIC_BER    0x04
#define LIC_PER    0x08
#define LIC_CPP    0x10
#define LIC_INIT   0x80

#define LCHECK(pctxt,bits)
#define LCHECKBER(pctxt)
#define LCHECKPER(pctxt)
#define LCHECKXER(pctxt)
#define LCHECKCPP(pctxt)
#define LCHECKIN(pctxt)
#define LCHECKX(pctxt)
#define LCHECKX2(pctxt)
#define LPRINTSTAT(pctxt)
#define LGETSTATMSG(pctxt) 0
#define LGETEXPTIME(pctxt) 0
#define LCLOSE()

#endif

