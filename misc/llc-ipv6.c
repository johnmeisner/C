/**
 * @addtogroup cohda_llc LLC module
 * @{
 *
 * @addtogroup cohda_llc_intern LLC internals
 * @{
 *
 * @file
 * LLC: LLC native IPv6 functionality
 *
 */


//-----------------------------------------------------------------------------
// Copyright (c) 2016 Cohda Wireless Pty Ltd
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Included headers
//-----------------------------------------------------------------------------
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
/* DENSO */
#include <linux/inet.h>
#include <linux/rtnetlink.h>
/* END DENSO */
#include <linux/skbuff.h>
#include <linux/errqueue.h>
#include <net/dsfield.h>
#include <linux/workqueue.h>

#include "linux/cohda/pktbuf.h"
#include "llc-internal.h"

#define D_SUBMODULE LLC_IPV6
#include "debug-levels.h"

//-----------------------------------------------------------------------------
// Macros & Constants
//-----------------------------------------------------------------------------

/// CCH IPv6 device name
#define CCH_IPV6_DEV_NAME      ("llc-cch-ipv6")

/// SCH IPv6 device name
#define SCH_IPV6_DEV_NAME      ("llc-sch-ipv6")

/// IPv6 MTU
#define IPV6_DEV_MTU       (1500)

/// reserved header space (hard_header)
#define IPV6_DEV_HDR       (96)

/// SNAP header DSAP field
#define SNAP_HEADER_DSAP     0xAA
/// SNAP header SSAP field
#define SNAP_HEADER_SSAP     0xAA
/// SNAP header control field
#define SNAP_HEADER_CONTROL  0x03

/* DENSO - Lazy port of IPv6 NDP for RSU from 5910 */
#define NDP_SUCCESS                         0
#define NDP_INVALID_ETHERNET_ADDRESS       -1
#define NDP_INVALID_RT_COMMAND             -2
#define NDP_ERROR_WRITING_ROUTING_SOCKET   -3 // errno is set
#define NDP_ERROR_READING_ROUTING_SOCKET   -4 // errno is set
#define NDP_ERROR_CREATING_ROUTING_SOCKET  -5 // errno is set
#define NDP_GETADDRINFO_ERROR              -6 // getaddrinfo() result in byte pointed to by gai_error
#define NDP_INVALID_SDL_FAMILY             -7
#define NDP_INVALID_RT_FLAGS               -8
#define NDP_INVALID_SDL_TYPE               -9
/* END DENSO */

//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
typedef enum IPV6Service
{
  /// Packet should be (was) transmitted using normal ACK policy
  IPV6_QOS_ACK = 0,
  /// Packet should be (was) transmitted without Acknowledgement.
  IPV6_QOS_NOACK = 1
} eIPV6Service;

/// MAC Address
typedef uint8_t tMACAddr[6];
/// MAC frame type
typedef enum MacFrameType
{
  /// Managment (00)
  MAC_FRAME_TYPE_MGNT = 0x0,
  /// Control (01)
  MAC_FRAME_TYPE_CONTROL = 0x1,
  /// Data (10)
  MAC_FRAME_TYPE_DATA = 0x2
} eMacFrameType;
/// @copydoc eMacFrameType
typedef uint8_t tMacFrameType;

/// MAC frame subtype
typedef enum MacFrameSubType
{
  /// Management - Action
  MAC_FRAME_SUB_TYPE_ACTION = 0xD,
  /// Data - Data (non QoS)
  MAC_FRAME_SUB_TYPE_DATA = 0x0,
  /// Data - QoS Data
  MAC_FRAME_SUB_TYPE_QOS_DATA = 0x8
} eMacFrameSubType;
/// @copydoc eMacFrameSubType
typedef uint8_t tMacFrameSubType;


/// 802.11 header frame control
typedef union MACFrameCtrl
{
  struct
  {
#ifdef __LITTLE_ENDIAN_BITFIELD
    /// Protocol version. Currently 0
    uint16_t ProtocolVer:2;
    /// Type -00 management frame 01-control frame,10-Data frame
    uint16_t Type:2;
    /// Subtype
    uint16_t SubType:4;
    /// To the distribution system
    uint16_t ToDS: 1;
    /// Exit from the distribution system
    uint16_t FromDS: 1;
    /// more fragment frames to follow (last or unfragmented frame=0)
    uint16_t MoreFrag: 1;
    /// This re-transmission
    uint16_t Retry: 1;
    /// Station in power save mode
    uint16_t PwrMgt: 1;
    /// Additional frames buffered for the destination address
    uint16_t MoreData: 1;
    /// 1= data processed with WEP algorithm 0= no WEP
    uint16_t WEP: 1;
    /// Frames must be strictly ordered
    uint16_t Order: 1;
#else // __BIG_ENDIAN_BITFIELD
    /// Frames must be strictly ordered
    uint16_t Order :1;
    /// 1= data processed with WEP algorithm 0= no WEP
    uint16_t WEP :1;
    /// Additional frames buffered for the destination address
    uint16_t MoreData :1;
    /// Station in power save mode
    uint16_t PwrMgt :1;
    /// This re-transmission
    uint16_t Retry :1;
    /// more fragment frames to follow (last or unfragmented frame=0)
    uint16_t MoreFrag :1;
    /// Exit from the distribution system
    uint16_t FromDS :1;
    /// To the distribution system
    uint16_t ToDS :1;
    /// Subtype
    uint16_t SubType :4;
    /// Type -00 management frame 01-control frame,10-Data frame
    uint16_t Type :2;
    /// Protocol version. Currently 0
    uint16_t ProtocolVer :2;
#endif
  } Fields;

  uint16_t FrameCtrl;
} __attribute__ ((packed)) tMACFrameCtrl;

/// 802.11 sequence control bits
typedef union MACSeqCtrl
{
  struct
  {
#ifdef __LITTLE_ENDIAN_BITFIELD
    uint16_t FragmentNo:4; //Frame fragment number
    uint16_t SeqNo:12; //Frame sequence number
#else // __BIG_ENDIAN_BITFIELD
    uint16_t SeqNo :12; //Frame sequence number
    uint16_t FragmentNo :4; //Frame fragment number
#endif
  } Fields;

  uint16_t SeqCtrl;
} __attribute__ ((packed)) tMACSeqCtrl;

/// 802.11 header QoS control
typedef union MACQoSCtrl
{
  struct
  {
#ifdef __LITTLE_ENDIAN_BITFIELD
    /// TID
    uint16_t TID:4;
    /// EOSP
    uint16_t EOSP:1;
    /// Ack Policy
    uint16_t AckPolicy:2;
    /// Reserved
    uint16_t Reserved:1;
    /// 'TXOP Duration Requested' or 'Queue size'
    uint16_t TXOPorQueue:8;
#else // __BIG_ENDIAN_BITFIELD
    /// 'TXOP Duration Requested' or 'Queue size'
    uint16_t TXOPorQueue :8;
    /// Reserved
    uint16_t Reserved :1;
    /// Ack Policy
    uint16_t AckPolicy :2;
    /// EOSP
    uint16_t EOSP :1;
    /// TID
    uint16_t TID :4;
#endif
  } Fields;

  uint16_t QoSCtrl;
} __attribute__ ((packed)) tMACQoSCtrl;

/// 802.11 MAC header (for QoS data frames)
typedef struct IEEE80211QoSHeader
{
  /// Frame control info
  tMACFrameCtrl FrameControl;
  /// Duration ID, for data frames= duration of frames
  uint16_t DurationId;
  /// SA Source address
  tMACAddr Address1;
  /// DA Destination address
  tMACAddr Address2;
  /// BSSID Receiving station address (destination wireless station)
  tMACAddr Address3;
  /// Sequence control info
  tMACSeqCtrl SeqControl;
  /// QoS control info
  tMACQoSCtrl QoSControl;

} __attribute__ ((packed)) tIEEE80211QoSHeader;

/// 802.11 MAC header
typedef struct IEEE80211Header
{
  /// Frame control info
  tMACFrameCtrl FrameControl;
  /// Duration ID, for data frames= duration of frames
  uint16_t DurationId;
  /// SA Source address
  tMACAddr Address1;
  /// DA Destination address
  tMACAddr Address2;
  /// BSSID Receiving station address (destination wireless station)
  tMACAddr Address3;
  /// Sequence control info
  tMACSeqCtrl SeqControl;

} __attribute__ ((packed)) tIEEE80211Header;

/// Ethernet-II header
typedef struct Ethernet2Header
{
  uint8_t DA[6]; //Destination address
  uint8_t SA[6]; //Source address
  uint16_t EtherType; //Ether type
} __attribute__ ((packed)) tEthernet2Header;


/* Start DENSO Changes */
/* We don't use SNAP headers */
///// 802.2 SNAP header
//typedef struct SNAPHeader
//{
//  uint8_t DSAP;
//  uint8_t SSAP;
//  uint8_t Control;
//  uint8_t OUI[3];
//  /// @note network order
//  uint16_t Type;
//} __attribute__ ((packed)) tSNAPHeader;
/* End DENSO Changes */

/// WAVE receive message header format
typedef struct RxMsgHdr
{
  /// MK2 transmit descriptor
  struct MKxRxPacket RxPacket;
  /// 802.11 MAC header
  union
  {
    struct IEEE80211Header MACHeader;
    /// 802.11 MAC header
    struct IEEE80211QoSHeader QoSHeader;
  };
} __attribute__ ((packed)) tRxMsgHdr;


typedef struct LLCIPv6WorkStruct
{
  struct LLCDevice *pDev;
  struct PktBuf *pPkb;
  struct work_struct Task;
} tLLCIPv6WorkStruct;


//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

static struct workqueue_struct *pWorkQueueCCH = NULL;
static struct workqueue_struct *pWorkQueueSCH = NULL;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#include <linux/etherdevice.h>
#else
static int eth_mac_addr(struct net_device *pNetDev, void *pAddr)
{
  struct sockaddr *pSockAddr = pAddr;

  if (netif_running(pNetDev))
    return -EBUSY;
  if (!is_valid_ether_addr(pSockAddr->sa_data))
    return -EADDRNOTAVAIL;
  memcpy(pNetDev->dev_addr, pSockAddr->sa_data, ETH_ALEN);
  return 0;
}

static int eth_change_mtu(struct net_device *pNetDev, int NewMTU)
{
  if (NewMTU < 68 || NewMTU > LLC_MAX_MTU)
    return -EINVAL;
  pNetDev->mtu = NewMTU;
  return 0;
}

static int eth_validate_addr(struct net_device *pNetDev)
{
  if (!is_valid_ether_addr(pNetDev->dev_addr))
    return -EADDRNOTAVAIL;

  return 0;
}
#endif // (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))

static int LLC_IPv6Tx (struct LLCDevice *pDev, struct PktBuf *pPkb);

//-----------------------------------------------------------------------------
// LLC IPv6 netdevice  interface related functionality
//-----------------------------------------------------------------------------

/* DENSO */
/* Converts a string Ethernet address to a byte array */
static int ndp_ether_aton(char *a, unsigned char *n)
{
    int i, o[6];

    i = sscanf(a, "%x:%x:%x:%x:%x:%x", &o[0], &o[1], &o[2],
        &o[3], &o[4], &o[5]);
    if (i != 6) {
        return NDP_INVALID_ETHERNET_ADDRESS;
    }
    for (i = 0; i < 6; i++)
        n[i] = o[i];
    return NDP_SUCCESS;
}

static int llc_ndp_msg(struct LLCDevice *dev, int radio, int cmd, const struct sockaddr_in6 *sin, unsigned char *eth_mac, int flags)
{
    struct net *net;
    struct net_device *pdev;
    struct neighbour *n;

    /* prevent driver errors */
    if ((NULL == dev) || (NULL == sin) || (NULL == eth_mac)) {
        return -EINVAL;
    }

    if ((cmd != RTM_GETNEIGH) && (cmd != RTM_DELNEIGH) && (cmd != RTM_NEWNEIGH)) {
        return -EINVAL;
    }
    if (radio == MKX_RADIO_A) {
        net = dev_net(dev->pDriver->IPv6.pNetDevCCH);
        pdev = dev->pDriver->IPv6.pNetDevCCH;
    } else if (radio == MKX_RADIO_B) {
        net = dev_net(dev->pDriver->IPv6.pNetDevSCH);
        pdev = dev->pDriver->IPv6.pNetDevSCH;
    } else {
        return -EINVAL;
    }

    if (NULL == (n = neigh_lookup(&nd_tbl, &sin->sin6_addr, pdev))) {
        if (NULL == (n = neigh_create(&nd_tbl, &sin->sin6_addr, pdev))) {
            return -ENOMEM;
        }
    }
    /* JJG NOTE: adding as NUD_PERMANENT means the rule is never deleted; on the 5910 there was a thread that was added
       to clean up the entries so nothing remained stale; consider adding a user space utility (maybe part of rsuhealth)
       to perform this action; it nearly drove me freaking insane just getting this working; there's no freaking way i
       would dare try to have a 'thread' launched by the driver in the kernel */
    neigh_update(n, eth_mac, NUD_PERMANENT, flags, 0);

    return 0;
}

/* Adds an entry to the network neighbor table. */
static int addNeighborTableEntry(struct LLCDevice *llcdev, int radio, char *host, char *eaddr)
{
    struct sockaddr_in6 sin_m;
    register struct sockaddr_in6 *mysin = &sin_m;
    unsigned char mac[ETH_ALEN];
    int ret;

    memset(&sin_m, 0, sizeof(sin_m));
    sin_m.sin6_family = AF_INET6;

#ifdef DENSO_DEBUG
    printk(KERN_INFO "IP and MAC: %s %s\n", host, eaddr);
#endif
    if (0 == (ret = in6_pton(host, -1, mysin->sin6_addr.s6_addr, -1, NULL))) {
        return NDP_GETADDRINFO_ERROR;
    }

    ret = ndp_ether_aton(eaddr, mac);
    if (ret != NDP_SUCCESS) {
        return ret;
    }

    ret = llc_ndp_msg(llcdev, radio, RTM_NEWNEIGH, &sin_m, mac, NLM_F_REQUEST|NLM_F_CREATE|NLM_F_REPLACE);
    return ret;
}

#ifdef FUTURE_IPV6_SUPPORT
/* Deletes an entry from the network neighbor table. */
static int deleteNeighborTableEntry(struct LLCDevice *llcdev, int radio, char *host)
{
    struct sockaddr_in6 sin_m;
    register struct sockaddr_in6 *mysin = &sin_m;
    unsigned char mac[ETH_ALEN] = {0};
    int ret;

    memset(&sin_m, 0, sizeof(sin_m));
    sin_m.sin6_family = AF_INET6;

    if (0 == (ret = in6_pton(host, -1, mysin->sin6_addr.s6_addr, -1, NULL))) {
        return NDP_GETADDRINFO_ERROR;
    }

    ret = llc_ndp_msg(llcdev, radio, RTM_DELNEIGH, &sin_m, mac, NLM_F_REQUEST|NLM_F_NONREC);
    return ret;
}
#endif /* FUTURE_IPV6_SUPPORT */

/* courtesy Apple originally from Paul Vixie, 1996 */
#ifndef NS_INADDRSZ
#define NS_INADDRSZ 4
#endif
#ifndef NS_IN6ADDRSZ
#define NS_IN6ADDRSZ    16
#endif
#ifndef NS_INT16SZ
#define NS_INT16SZ  2
#endif
static const char *
inet_ntop4(const u_char *src, char *dst, size_t size)
{
    static const char fmt[] = "%u.%u.%u.%u";
    char tmp[sizeof "255.255.255.255"];
    int nprinted;

    nprinted = snprintf(tmp, sizeof(tmp), fmt, src[0], src[1], src[2], src[3]);
    if (nprinted < 0)
        return (NULL);  /* we assume "errno" was set by "snprintf()" */
    if ((size_t)nprinted >= size) {
        //errno = ENOSPC;
        return (NULL);
    }
    strlcpy(dst, tmp, size);
    return (dst);
}

static const char *
inet_ntop6(const unsigned char *src, char *dst, size_t size)
{
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size.  On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays.  All the world's not a VAX.
     */
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
    struct { int base, len; } best, cur;
    unsigned int words[NS_IN6ADDRSZ / NS_INT16SZ];
    int i;

    /*
     * Preprocess:
     *   Copy the input (bytewise) array into a wordwise array.
     *  Find the longest run of 0x00's in src[] for :: shorthanding.
     */
     memset(words, '\0', sizeof words);
     for (i = 0; i < NS_IN6ADDRSZ; i++)
         words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
     best.base = -1;
     best.len = 0;
     cur.base = -1;
     cur.len = 0;
     for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
         if (words[i] == 0) {
             if (cur.base == -1)
                 cur.base = i, cur.len = 1;
             else
                 cur.len++;
         } else {
             if (cur.base != -1) {
                 if (best.base == -1 || cur.len > best.len)
                     best = cur;
                 cur.base = -1;
             }
         }
     }
     if (cur.base != -1) {
         if (best.base == -1 || cur.len > best.len)
             best = cur;
     }
     if (best.base != -1 && best.len < 2)
         best.base = -1;

     /*
      * Format the result.
      */
     tp = tmp;
     for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
         /* Are we inside the best run of 0x00's? */
         if (best.base != -1 && i >= best.base &&
            i < (best.base + best.len)) {
             if (i == best.base)
                 *tp++ = ':';
             continue;
         }
         /* Are we following an initial run of 0x00s or any real hex? */
         if (i != 0)
             *tp++ = ':';
         /* Is this address an encapsulated IPv4? */
         if (i == 6 && best.base == 0 &&
           (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
             if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)))
                 return (NULL);
             tp += strlen(tp);
             break;
         }
         tp += snprintf(tp, sizeof tmp - (tp - tmp), "%x", words[i]);
     }
     /* Was it a trailing run of 0x00's? */
     if (best.base != -1 && (best.base + best.len) ==
       (NS_IN6ADDRSZ / NS_INT16SZ))
         *tp++ = ':';
     *tp++ = '\0';

     /*
      * Check for overflow, copy, and we're done.
      */
     if ((size_t)(tp - tmp) > size) {
         //errno = ENOSPC;
         return (NULL);
     }
     strlcpy(dst, tmp, size);
     return (dst);
}

/* Adds an new NDP table entry. Assumes an entry with this IPv6 address does
 * not already exist */
static int ndp_table_add_entry(struct LLCDevice *llcdev, int radio, uint8_t *ipv6_addr, uint8_t *mac_addr)
{
  int idx = 0;    /* unused; ported from 5910; initialize to a non negative */
  char ipv6Str[MAX_IPV6_ADDR_STR_LEN + 1];
  char etherStr[MAX_ETHER_ADDR_STR_LEN + 1];
  int res;

  d_fnstart(D_API, NULL, "(%p, %p)\n", ipv6_addr, mac_addr);
  /* Add a new system neighbor table entry */
  inet_ntop6(ipv6_addr, ipv6Str, sizeof(ipv6Str));
  snprintf(etherStr, sizeof(etherStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);
  res = addNeighborTableEntry(llcdev, radio, ipv6Str, etherStr);

  if (res != NDP_SUCCESS) {
    switch(res) {
    case NDP_ERROR_WRITING_ROUTING_SOCKET:
    case NDP_ERROR_READING_ROUTING_SOCKET:
    case NDP_ERROR_CREATING_ROUTING_SOCKET:
      d_printf(D_ERR, NULL, "%s %s: %s\n",
               ipv6Str, etherStr, "add neighbor failed");
      break;
    case NDP_GETADDRINFO_ERROR:
      d_printf(D_ERR, NULL, "%s %s: %s\n",
               ipv6Str, etherStr, "error converting ipv6 address");
      break;
    }

    d_printf(D_ERR, NULL, "Add neighbor table entry %s %s failed (%d)\n",
             ipv6Str, etherStr, res);
  }

  d_fnend(D_API, NULL, "(%p, %p) = %d\n", ipv6_addr, mac_addr, idx);
  return idx;
}

#ifdef DENSO_DEBUG_EXTRA
/* leaving this hear for now in case i need to see input packet data again */
void my_pkt_hex_dump(void *dataptr, size_t len)
{
    int rowsize = 16;
    int i, l, linelen, remaining;
    int li = 0;
    uint8_t *data, ch;

    printk(KERN_INFO "Packet hex dump:\n");
    data = (uint8_t *) dataptr;

    remaining = len;
    for (i = 0; i < len; i += rowsize) {
        printk("%06d\t", li);

        linelen = min(remaining, rowsize);
        remaining -= rowsize;

        for (l = 0; l < linelen; l++) {
            ch = data[l];
            printk(KERN_INFO "%02X ", (uint32_t) ch);
        }

        data += linelen;
        li += 10;

        printk(KERN_INFO "\n");
    }
}
#endif /* DENSO_DEBUG_EXTRA */
/* END DENSO */

static struct workqueue_struct *LLC_IPv6CreateQueue(const char *pName)
{
  struct workqueue_struct *pWorkQ = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0))
  pWorkQ = create_singlethread_workqueue(pName);
#else
  {
    int Realtime = 1;
    int SingleThread = 1;
    pWorkQ = __create_workqueue(pName, SingleThread, 0, Realtime);
  }
#endif

  return pWorkQ;
}

void LLC_IPv6WorkHandler(struct work_struct *pTask)
{
  struct LLCIPv6WorkStruct *pWork = container_of(pTask, struct LLCIPv6WorkStruct, Task);

  int Res = LLC_IPv6Tx(pWork->pDev, pWork->pPkb);
  if (Res != 0)
  {
    dev_kfree_skb (pWork->pPkb);
  }
  kfree (pWork);
}

static int LLC_IPv6WorkEnqueue(struct workqueue_struct *pQ,
                               struct LLCDevice *pDev,
                               struct PktBuf *pPkb)
{
  int Res = 1;
  if (pQ != NULL)
  {
    struct LLCIPv6WorkStruct *pWork = kmalloc(sizeof(struct LLCIPv6WorkStruct),  GFP_KERNEL);
    pWork->pDev = pDev;
    pWork->pPkb = pPkb;
    INIT_WORK(&(pWork->Task), LLC_IPv6WorkHandler);
    Res = queue_work(pQ, &(pWork->Task));
    Res = 1 - Res;
    if (Res != 0)
    {
      d_printf(D_ERR, NULL, "Failed to queue Tx data\n");
    }
  }
  else
  {
    d_printf(D_ERR, NULL, "Tx queue is not initialised\n");
  }

  return Res;
}

/**
 * @brief Start the virtual network interface (Called on ifconfig up)
 *
 * @param pNetDev     Either 'llc-cch-ipv6' or 'llc-sch-ipv6' network interface structure
 *
 * @return 0 if success \n
 *         <0 if failure .\n
 *
 *
 */
static int LLC_IPv6Open(struct net_device *pNetDev)
{
  int Res = -ENOSYS;
  struct workqueue_struct *pWorkQ = NULL;

  d_fnstart(D_INTERN, NULL, "(pNetDev %p)\n", pNetDev);

  if (NULL == pNetDev)
  {
    d_error(D_ERR, NULL, "Bad address\n");
    Res = -EINVAL;
    goto Error;
  }

  netif_start_queue(pNetDev);
  Res = 0;

  if (strncmp(pNetDev->name, CCH_IPV6_DEV_NAME, strlen(CCH_IPV6_DEV_NAME)) == 0)
  {
    pWorkQ = pWorkQueueCCH = LLC_IPv6CreateQueue(CCH_IPV6_DEV_NAME);
  }
  else if (strncmp(pNetDev->name, SCH_IPV6_DEV_NAME, strlen(SCH_IPV6_DEV_NAME)) == 0)
  {
    pWorkQ = pWorkQueueSCH = LLC_IPv6CreateQueue(SCH_IPV6_DEV_NAME);
  }
  else
  {
    d_error(D_ERR, NULL, "Not expected net device = %s\n", pNetDev->name);
    goto Error;
  }

  if (pWorkQ == NULL)
  {
    d_error(D_WARN, NULL, "Failed to create %s kernel thread\n", pNetDev->name);
    Res = -EAGAIN;
  }

Error:
  d_fnend(D_INTERN, NULL, "(pNetDev %p) = %d\n", pNetDev, Res);
  return Res;
}

/**
 * @brief Stop the virtual network interface (Called on ifconfig down)
 *
 * @param pNetDev   Either 'llc-cch-ipv6' or 'llc-sch-ipv6' network interface structure
 *
 * @return 0 if success \n
 *         <0 if failure .\n
 *
 *
 */
static int LLC_IPv6Close(struct net_device *pNetDev)
{
  int Res = -ENOSYS;

  d_fnstart(D_INTERN, NULL, "(pNetDev %p)\n", pNetDev);

  if (NULL == pNetDev)
  {
    d_error(D_ERR, NULL, "Bad address\n");
    Res = -EINVAL;
    goto Error;
  }
  if (strncmp(pNetDev->name, CCH_IPV6_DEV_NAME, strlen(CCH_IPV6_DEV_NAME)) == 0)
  {
    if (pWorkQueueCCH != NULL)
    {
      flush_workqueue(pWorkQueueCCH);
      destroy_workqueue(pWorkQueueCCH);
    }
  }
  else if (strncmp(pNetDev->name, SCH_IPV6_DEV_NAME, strlen(SCH_IPV6_DEV_NAME)) == 0)
  {
    if (pWorkQueueSCH != NULL)
    {
      flush_workqueue(pWorkQueueSCH);
      destroy_workqueue(pWorkQueueSCH);
    }
  }
  else
  {
    d_error(D_ERR, NULL, "Not expected net device = %s\n", pNetDev->name);
    goto Error;
  }

  netif_stop_queue(pNetDev);

  Res = 0;

Error:
  d_fnend(D_INTERN, NULL, "(pNetDev %p) = %d\n", pNetDev, Res);
  return Res;
}


/**
 * @brief Handle esoteric 'ifconfig' calls on the 'llc-cch-ipv6' or 'llc-sch-ipv6' interfaces
 *
 * @param pNetDev Either 'llc-cch-ipv6' or 'llc-sch-ipv6' network interface structure
 * @param pMap  Pointer to structure ifmap
 *
 * @return 0 if ok, < 0 errno code on error.\n
 *
 *
 */
static int LLC_IPv6Config(struct net_device *pNetDev,
                          struct ifmap *pMap)
{
  int Res = -ENOSYS;

  d_fnstart(D_INTERN, NULL, "(pNetDev %p pMap %p)\n", pNetDev, pMap);

  if ((NULL == pNetDev) || (NULL == pMap))
  {
    d_error(D_ERR, NULL, "Bad address\n");
    Res = -EINVAL;
    goto Error;
  }

  if (pNetDev->flags & IFF_UP)
  {
    return -EBUSY;
  }

  if (pMap->base_addr != pNetDev->base_addr)
  {
    return -EOPNOTSUPP;
  }

Error:
  d_fnend(D_INTERN, NULL, "(pNetDev %p pMap %p) = %d\n", pNetDev, pMap, Res);
  return Res;
}

/**
 * @brief Handle a 'llc-cch-ipv6' or 'llc-sch-ipv6'  interface stall
 *
 * @param pNetDev Either 'llc-cch-ipv6' or 'llc-sch-ipv6' network interface structure
 *
 * @return    None\n
 *
 * Called by the Linux networking code on TX timeout:
 * Kick the queue and increment the error count
 *
 */
static void LLC_IPv6TxTimeout(struct net_device *pNetDev)
{
  int Res = -ENOSYS;
  d_fnstart(D_INTERN, NULL, "(pNetDev %p)\n", pNetDev);

  if (NULL == pNetDev)
  {
    d_error(D_ERR, NULL, "Bad address\n");
    Res = -EINVAL;
    goto Error;
  }

  netif_wake_queue(pNetDev);

  pNetDev->stats.tx_errors++;

Error:
  d_fnend(D_INTERN, NULL, "(pNetDev %p) = %d\n", pNetDev, Res);
  return;
}

/**
 * @brief Handle an ioctl() call on the 'llc-cch-ipv6' or 'llc-sch-ipv6' socket
 * @param pNetDev    The 'llc-cch-ipv6' or 'llc-sch-ipv6'  network interface structure
 * @param pReq         The dreaded 'ifreq' structure
 * @param Cmd          The ioctl command number
 * @return 0 if ok, otherwise a negative errno
 *
 *
 */
static int LLC_IPv6Ioctl(struct net_device *pNetDev,
                         struct ifreq *pReq,
                         int Cmd)
{
  int Res = -ENOSYS;

  d_fnstart(D_INTERN, NULL, "(pNetDev %p pReq %p Cmd %x)\n",
            pNetDev, pReq, Cmd);

  if ((NULL == pNetDev) || (NULL == pReq))
  {
    d_error(D_ERR, NULL, "wrong data structure\n");
    Res = -EINVAL;
    goto Error;
  }

  if ((Cmd >= SIOCDEVPRIVATE) && (Cmd <= SIOCDEVPRIVATE + 15))
  {
    // Nothing... yet
    Res = 0;
  }
  else
  {
    Res = 0;
  }

Error:
  d_fnend(D_INTERN, NULL, "(pNetDev %p pReq %p Cmd %x) = %d\n",
          pNetDev, pReq, Cmd, Res);
  return Res;
}


/**
 * @brief Classify the priority of the packet
 *
 * @param pPkb The packet to classify
 * @return The tMK2Priority of the packet
 */
static int LLC_IPv6Classify(uint16_t Protocol,
                            struct PktBuf *pPkb)
{
  // TODO: do we need this and what should it be?
  int Priority = 0;
  uint8_t DiffServ = 0;

  d_fnstart(D_INTERN, NULL, "(%p)\n", pPkb);

  switch (Protocol)
  {
    case ETH_P_IP:
    {
      struct iphdr *pIPHdr = (struct iphdr *)(skb_network_header(pPkb));
      if (NULL == pIPHdr)
      {
        goto Error;
      }

      DiffServ = ipv4_get_dsfield(pIPHdr);
      break;
    }

    case ETH_P_IPV6:
    {
      struct ipv6hdr *pIPv6Hdr = ipv6_hdr(pPkb);
      if (NULL == pIPv6Hdr)
      {
        break;
      }

      DiffServ = ipv6_get_dsfield(pIPv6Hdr);
      break;
    }

    default:
    {
      // no mapping for IPv6 or other protocols
      goto Error;
    }
  }

  // map as per cfg80211_classify8021d() without having to actually depend on
  // that implementation
  Priority = (DiffServ & 0xfc) >> 5;

Error:
  d_fnend(D_INTERN, NULL, "(%u %p) = %u\n", Protocol, pPkb, Priority);
  return Priority;
}

/**
 * @brief Prepare the data payload TX header
 *
 * @param pPkb      The unaltered packet as received from userspace
 * @param pDot4      Dot4 structure pointer
 *
 * @return  DOT4_RESULT_SUCCESS if result is ok \n
 *          DOT4_RESULT_FAILURE if result is failure .\n
 *          -ENOMEM if enough memory not available for pPkb \n
 *          -EINVAL if invalid handle is passed \n
 *
 * This function check for the ethertype in ether header of received packet
 * if ethertype matched with the one stored in MLME EtherType table then process
 * the packet else do not process packet and return. This function add
 * MK2Descriptor 802.11 header and SNAP header to the received packet
 *
 * DENSO Note: This module originally used LLC LPD, which uses full SNAP
 * headers. In order to be compliant with ieee requirements, we use LLC EPD,
 * which does not include a full SNAP header and only includes the Ethertype.
 */
int LLC_IPv6TxPrepHeader(struct LLCDriver *pDrv,
                         struct net_device *pNetDev,
                         struct PktBuf *pPkb)
{
  int Res = -ENOSYS;
  struct MKxTxPacket *pLLCHdr;
  struct IEEE80211QoSHeader *pMACHdr;
  tMKxRadioConfigData *pMKxConfig = NULL;
  tMKxChanConfig *pChannelConfig = NULL;
  /* Start DENSO Changes */
  //struct SNAPHeader *pSNAPHdr = NULL;
  uint16_t *pEthertype = NULL;
  /* End DENSO Changes */
  struct ethhdr EthHdr = {};
  int Radio = -1;
  int Priority = 0;

  d_fnstart(D_INTERN, NULL, "(pPkb %p)\n", pPkb);

  if (NULL == pDrv)
  {
    d_printf(D_WARN, NULL, "LLC not yet initialised\n");
    Res = -EINVAL;
    goto Error;
  }

  if (NULL == pNetDev)
  {
    d_printf(D_WARN, NULL, "Netdev not yet initialised\n");
    Res = -EINVAL;
    goto Error;
  }

  // Check if pPkb pointer is NULL
  if (NULL == pPkb)
  {
    d_error(D_ERR, NULL, "Wrong data structure\n");
    Res = -EINVAL;
    goto Error;
  }

  // Get the MKx config structure - it stores configuration for each radio
  if (strncmp(pNetDev->name, CCH_IPV6_DEV_NAME, strlen(CCH_IPV6_DEV_NAME)) == 0)
  {
    // Get the Radio_A configuration - this will be CCH
    pMKxConfig = (tMKxRadioConfigData *) & (pDrv->Dev[0].MKx.Config.Radio[MKX_RADIO_A]);
    /* Start DENSO Changes */
    //pChannelConfig = &(pMKxConfig->ChanConfig[MKX_CHANNEL_0]);
    pChannelConfig = &(pMKxConfig->ChanConfig[MKX_CHANNEL_1]);
    /* End DENSO Changes */
    Radio = MKX_RADIO_A;
  }
  else if (strncmp(pNetDev->name, SCH_IPV6_DEV_NAME, strlen(SCH_IPV6_DEV_NAME)) == 0)
  {
    // Get the Radio_B configuration - this will be SCH
    pMKxConfig = (tMKxRadioConfigData *) & (pDrv->Dev[0].MKx.Config.Radio[MKX_RADIO_B]);
    /* Start DENSO Changes */
    //pChannelConfig = &(pMKxConfig->ChanConfig[MKX_CHANNEL_0]);
    pChannelConfig = &(pMKxConfig->ChanConfig[MKX_CHANNEL_1]);
    /* End DENSO Changes */
    Radio = MKX_RADIO_B;
  }
  else
  {
    d_printf(D_ERR, NULL, "Unknown netdev name %s\n", pNetDev->name);
    Res = -EINVAL;
    goto Error;
  }

  // Copy the Ethernet header & advance the data pointer to the data payload
  memcpy(&EthHdr, pPkb->data, sizeof(struct ethhdr));
  Priority = LLC_IPv6Classify(ntohs(EthHdr.h_proto), pPkb);

  PktBuf_Pull(pPkb, sizeof(struct ethhdr));

  /* Start DENSO Changes */
  //PktBuf_Push(pPkb, sizeof(struct SNAPHeader));
  //pSNAPHdr = (struct SNAPHeader *) pPkb->data;
  PktBuf_Push(pPkb, sizeof(uint16_t));
  pEthertype = (uint16_t *) pPkb->data;
  /* End DENSO Changes */

  PktBuf_Push(pPkb, sizeof(struct IEEE80211QoSHeader));
  pMACHdr = (struct IEEE80211QoSHeader *) pPkb->data;

  PktBuf_Push(pPkb, sizeof(struct MKxTxPacket));
  pLLCHdr = (struct MKxTxPacket *) pPkb->data;

  // Clear all header information before populating
  /* Start DENSO Changes */
  //memset(pSNAPHdr, 0, sizeof(struct SNAPHeader));
  memset(pEthertype, 0, sizeof(uint16_t));
  /* End DENSO Changes */
  memset(pMACHdr, 0, sizeof(struct IEEE80211QoSHeader));
  memset(pLLCHdr, 0, sizeof(struct MKxTxPacket));

  /* Start DENSO Changes */
  //// Setup the SNAP header
  //pSNAPHdr->DSAP = SNAP_HEADER_DSAP;
  //pSNAPHdr->SSAP = SNAP_HEADER_SSAP;
  //pSNAPHdr->Control = SNAP_HEADER_CONTROL;
  //pSNAPHdr->OUI[0] = 0x00;
  //pSNAPHdr->OUI[1] = 0x00;
  //pSNAPHdr->OUI[2] = 0x00;
  //pSNAPHdr->Type = EthHdr.h_proto;
  // Setup the Ethertype
  *pEthertype = EthHdr.h_proto;
  /* End DENSO Changes */

  pMACHdr->FrameControl.FrameCtrl = 0;
  pMACHdr->FrameControl.Fields.Type = MAC_FRAME_TYPE_DATA;
  pMACHdr->FrameControl.Fields.SubType = MAC_FRAME_SUB_TYPE_QOS_DATA;
  pMACHdr->DurationId = 0x0000;
  memcpy(pMACHdr->Address1, &EthHdr.h_dest, ETH_ALEN);
  memcpy(pMACHdr->Address2, &EthHdr.h_source, ETH_ALEN);
  memset(pMACHdr->Address3, 0xFF, ETH_ALEN);
  pMACHdr->SeqControl.SeqCtrl = 0x0000; // Set by the UpperMAC
  pMACHdr->QoSControl.QoSCtrl = 0x0000;
  pMACHdr->QoSControl.Fields.TID = Priority;
  pMACHdr->QoSControl.Fields.EOSP = 0;
  if ((EthHdr.h_dest[0] & 0x01) == 0x01)
  {
    pMACHdr->QoSControl.Fields.AckPolicy = IPV6_QOS_NOACK;
  }
  else
  {
    pMACHdr->QoSControl.Fields.AckPolicy = IPV6_QOS_ACK;
  }
  pMACHdr->QoSControl.Fields.TXOPorQueue = 0; // TODO

  pLLCHdr->TxPacketData.RadioID       = Radio;

  /* Start DENSO Changes */
  //pLLCHdr->TxPacketData.ChannelID     = MKX_CHANNEL_0;
  //// Base the Tx Antenna on the configuration - could be single antenna mode
  //pLLCHdr->TxPacketData.TxAntenna     = pMKxConfig->ChanConfig[MKX_CHANNEL_0].PHY.TxAntenna;
  //pLLCHdr->TxPacketData.MCS           = (uint8_t)LLC_GetIPv6MCS();
  //pLLCHdr->TxPacketData.TxPower       = pMKxConfig->ChanConfig[MKX_CHANNEL_0].PHY.DefaultTxPower;
  pLLCHdr->TxPacketData.ChannelID     = MKX_CHANNEL_1;
  // Base the Tx Antenna on the configuration - could be single antenna mode
  pLLCHdr->TxPacketData.TxAntenna     = pMKxConfig->ChanConfig[MKX_CHANNEL_1].PHY.TxAntenna;
  pLLCHdr->TxPacketData.MCS           = (uint8_t)LLC_GetIPv6MCS();
  pLLCHdr->TxPacketData.TxPower       = (int16_t)LLC_GetIPv6TxPwr();
  /* End DENSO Changes */
  pLLCHdr->TxPacketData.Expiry        = 0;
  pLLCHdr->TxPacketData.TxFrameLength =
    pPkb->len - sizeof(struct MKxTxPacket);

  Res = 0;

Error:
  d_fnend(D_INTERN, NULL, "(pPkb %p) Res = %d\n", pPkb, Res);
  return Res;

}

/**
 * @brief Prepare the data payload RX header
 *
 * @param pPkb  The pointer to buffer PktBuf
 *
 * @return  0 if result is ok \n
 *          <0 if result is failure .\n
 *          -ENOMEM if enough memory not available for pPkb \n
 *          -EINVAL if invalid handle is passed \n
 *
 *
 * This function prepare Data packet Rx header. Because we pretend to be
 * an ethernet device,this packet requires an ethernet header
 * (pSkv->mac_header)
 *
 * DENSO Note: This module originally used LLC LPD, which uses full SNAP
 * headers. In order to be compliant with ieee requirements, we use LLC EPD,
 * which does not include a full SNAP header and only includes the Ethertype.
 */
int LLC_IPv6RxPrepHeader (struct LLCDriver *pDrv, struct PktBuf *pPkb)
{
  int Res = -ENOSYS;
  struct IEEE80211QoSHeader *pMACHdr = NULL;
  /* Start DENSO Changes */
  //struct SNAPHeader *pSNAPHdr = NULL;
  uint16_t *pEthertype;
  /* End DENSO Changes */
  struct ethhdr *pEthHdr = NULL;
  int HdrLen;
  tMACFrameCtrl Dot11FrameCtrl;

  d_fnstart(D_INTERN, NULL, "(pPkb %p)\n", pPkb);

  if (NULL == pPkb)
  {
    d_error(D_ERR, NULL, "wrong data structure\n");
    Res = -EINVAL;
    goto Error;
  }

  d_printf(D_DBG, NULL, "pPkb data %p len %d\n", pPkb->data, pPkb->len);

  /* Start DENSO Changes */
  //HdrLen = sizeof(struct IEEE80211QoSHeader) + sizeof(struct SNAPHeader);
  HdrLen = sizeof(struct IEEE80211QoSHeader) + sizeof(uint16_t);
  /* End DENSO Changes */
  if (pPkb->len < HdrLen)
  {
    d_error(D_ERR, NULL, "Frame too short!\n");
    Res = -EINVAL;
    goto Error;
  }

  // Advance the data pointer past the headers to the data payload
  pMACHdr = (struct IEEE80211QoSHeader *) pPkb->data;
  Dot11FrameCtrl.FrameCtrl = pMACHdr->FrameControl.FrameCtrl;
  le16_to_cpus(&(Dot11FrameCtrl.FrameCtrl));
  switch (Dot11FrameCtrl.Fields.SubType)
  {
    case MAC_FRAME_SUB_TYPE_QOS_DATA:
      PktBuf_Pull(pPkb, sizeof(struct IEEE80211QoSHeader));
      break;
    case MAC_FRAME_SUB_TYPE_DATA:
      PktBuf_Pull(pPkb, sizeof(struct IEEE80211Header));
      break;
    default:
      d_printf(D_ERR, NULL, "Unhandled MAC frame type %d\n", Dot11FrameCtrl.Fields.SubType);
  }

  /* Start DENSO Changes */
  //pSNAPHdr = (struct SNAPHeader *) pPkb->data;
  //PktBuf_Pull(pPkb, sizeof(struct SNAPHeader));
  pEthertype = (uint16_t *) pPkb->data;
  PktBuf_Pull(pPkb, sizeof(uint16_t));
  /* End DENSO Changes */

  // No need to check the headroom as the new headers are smaller

  // Roll back the data pointer for the new headers
  PktBuf_Push(pPkb, sizeof(struct ethhdr));
  pEthHdr = (struct ethhdr *) pPkb->data;

  // FCS?
  PktBuf_Trim(pPkb, pPkb->len - 4);

  /* Start DENSO Changes */
  //// check that the SNAP header is valid
  //if ((SNAP_HEADER_DSAP != pSNAPHdr->DSAP) ||
  //    (SNAP_HEADER_SSAP != pSNAPHdr->SSAP) ||
  //    (SNAP_HEADER_CONTROL != pSNAPHdr->Control))
  //{
  //  d_printf(D_WARN, NULL, "Corrupt SNAP header!\n");
  //  goto Error;
  //}
  /* End DENSO Changes */

  // Populate the new header from right to left
  // First, the Ethernet header
  /* Start DENSO Changes */
  //pEthHdr->h_proto = pSNAPHdr->Type; // should optimize away
  pEthHdr->h_proto = *pEthertype;
  /* End DENSO Changes */
  memmove(pEthHdr->h_source, pMACHdr->Address2, ETH_ALEN);
  memmove(pEthHdr->h_dest, pMACHdr->Address1, ETH_ALEN);

  Res = 0;

Error:
  d_fnend(D_INTERN, NULL, "(pPkb %p )Res = %d\n", pPkb, Res);
  return Res;
}

/**
 * @brief Pass the modified Tx frame to the UMAC
 *
 * @param pPkb      The unaltered packet as received from userspace
 * @param pDot4      Dot4 structure pointer
 *
 * @return  0 if result is ok \n
 *          <0 if result is failure .\n
 *          -EINVAL if invalid handle is passed \n
 *
 * Hand the buffer to the UMAC function that evntually delivers it to the
 * LowerMAC via the HIL
 *
 *
 */
static int LLC_IPv6Tx(struct LLCDevice *pDev,
                      struct PktBuf *pPkb)
{
  int Res = -ENOSYS;
  struct sk_buff *pSkb = NULL;

  d_fnstart(D_INTERN, NULL, "(pDev %p pPkb %p)\n", pDev, pPkb);

  if ((NULL == pDev) || (NULL == pPkb))
  {
    d_error(D_WARN, NULL, "Wrong data structure\n");
    Res = -EINVAL;
    goto Error;
  }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0))
  // Linux specific Tx timestamp signalling via the error path of the socket

  pSkb = (struct sk_buff *)pPkb;
  if (pSkb->sk != NULL)
  {
    // Get the socket's Tx flags
    sock_tx_timestamp(pSkb->sk,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0))
                      pSkb->sk->sk_tsflags,
#endif
                      &(skb_shinfo(pSkb)->tx_flags));

#if 0    /* removing this block that can cause a kernel panic; timestamping is not required */
    // Is timestamping required?
    if (skb_shinfo(pSkb)->tx_flags & SKBTX_SW_TSTAMP &&
        !(skb_shinfo(pSkb)->tx_flags & SKBTX_IN_PROGRESS))
    {
      struct sk_buff *pSkbErr;

      // Set the timestamp
      {
        struct timespec Now;
        getnstimeofday(&Now);
        pSkb->tstamp = timespec_to_ktime(Now);
      }

      // Clone it
      pSkbErr = skb_clone(pSkb, GFP_ATOMIC);
      if (pSkbErr != NULL)
      {
        struct sock_exterr_skb *pSockErr;
        pSockErr = SKB_EXT_ERR(pSkbErr);
        if (pSockErr != NULL)
        {
          memset(pSockErr, 0, sizeof(*pSockErr));
          pSockErr->ee.ee_errno = ENOMSG;
          pSockErr->ee.ee_origin = SO_EE_ORIGIN_TIMESTAMPING;

          // Report the timestamp via the socket's error path
          if ((NULL != pSkb) && (NULL != pSkb->sk)) {    /* DENSO: extra error checking because of how a user space app handles the socket */
            sock_hold(pSkb->sk);    /* DENSO: do not share with Cohda/NXP; this may not be necessary (it does not harm and is described as mandatory
                                              by kernel documentation) to prevent a kernel panic that can happen with queuing race condition */
            Res = sock_queue_err_skb(pSkb->sk, pSkbErr);
            if (Res != 0)
            {
              kfree_skb(pSkbErr);
            }
            sock_put(pSkb->sk);    /* DENSO: do not share with Cohda/NXP */
          } else {    /* DENSO */
            kfree_skb(pSkbErr);    /* the sock buff memory freed; need to free pSkbErr still */
          }
        }
      }
    }
#endif
  }

#endif // (LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0))
  Res = LLC_TxReq(&(pDev->MKx),
                  (struct MKxTxPacket *)(pPkb->data),
                  pDev);
  // Note since there is no TxCnf we need to free the pkb here (on Success)
  if (Res == 0)
  {
    if (pPkb != NULL)
    {
      PktBuf_Free(pPkb);
      pPkb = NULL;
    }
  }

Error:
  d_fnend(D_INTERN, NULL, "(pDev %p pPkb %p) = %d\n", pDev, pPkb, Res);
  return Res;
}



/**
 * @brief Transmit a packet on behalf of the Linux network stack.
 *
 * @param pNetDev   The 'wave-data' network interface structure
 * @param pPkb      The un-altered packet as received from userspace
 *
 * @return  DOT4_RESULT_SUCCESS if result is ok \n
 *          DOT4_RESULT_FAILURE if result is failure .\n
 *          -EINVAL if invalid handle is passed \n
 *
 * We need to pull the tx descriptor & ethernet header and deliver it to the
 * upperMAC function which ultimately passes it to the HIL which then
 * delivers it to the LowerMAC.
 *
 *
 */
static int LLC_IPv6TxPkb(struct net_device *pNetDev,
                         struct PktBuf *pPkb)
{
  int Res = -ENOSYS;
  struct LLCDriver *pDrv = NULL;
  struct LLCIPv6Priv *pPriv = NULL;

  d_fnstart(D_INTERN, NULL, "(pNetDev %p pPkb %p)\n", pNetDev, pPkb);

  if ((NULL == pNetDev) || (NULL == pPkb))
  {
    d_error(D_WARN, NULL, "Wrong data structure\n");
    Res = -EINVAL;
    goto Error;
  }

  // Extract the LLCDev handle from within the net device's private storage
  pPriv = (struct LLCIPv6Priv *)netdev_priv(pNetDev);
  pDrv = pPriv->pDrv;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0))
  netif_trans_update(pNetDev);
#else
  pNetDev->trans_start = jiffies;
#endif

  // Check & transform the packet headers
  d_dump(D_VERBOSE, NULL, pPkb->data, pPkb->len);
  Res = LLC_IPv6TxPrepHeader(pDrv, pNetDev, pPkb);
  if (Res)
  {
    d_printf(D_ERR, NULL, "LLC_IPv6TxPrepHeader() = %d\n", Res);
    goto Error;
  }

  d_printf(D_DEBUG, NULL, "pPkb %p sending %d bytes to radio\n", pPkb, pPkb->len);
  d_dump(D_VERBOSE, NULL, pPkb->data, pPkb->len);
  if (in_atomic())
  {
    if (strncmp(pNetDev->name, CCH_IPV6_DEV_NAME, strlen(CCH_IPV6_DEV_NAME)) == 0)
    {
      LLC_IPv6WorkEnqueue(pWorkQueueCCH, &(pDrv->Dev[0]), pPkb);
    }
    else if (strncmp(pNetDev->name, SCH_IPV6_DEV_NAME, strlen(SCH_IPV6_DEV_NAME)) == 0)
    {
      LLC_IPv6WorkEnqueue(pWorkQueueSCH, &(pDrv->Dev[0]), pPkb);
    }
    else
    {
      d_error(D_ERR, NULL, "Not expected net device = %s\n", pNetDev->name);
      Res = -EINVAL;
      goto Error;
    }

  }
  else
  {
    Res = LLC_IPv6Tx(&(pDrv->Dev[0]), pPkb);
    if (Res)
    {
      d_printf(D_ERR, NULL, "LLC_IPv6Tx() = %d\n", Res);
      goto Error;
    }
  }

Error:
  d_fnend(D_INTERN, NULL, "(pNetDev %p pPkb %p) = %d\n",
          pNetDev, pPkb, Res);
  return Res;
}

/**
 * @brief Transmit a packet on behalf of the network stack
 *
 * @param pTxPkb        The transmit packet'
 * @param pNetDev       The wave-data net device structure
 *
 * @return NETDEV_TX_OK (always, even in case of error)\n
 *
 * In case of error, we just drop it. Reasons:
 *  - we add a hw header to each pktbuf, and if the network stack
 *    retries, we have no way to know if that pktbuf has it or not.
 * - network protocols have their own drop-recovery mechanisms
 * - there is not much else we can do
 *
 *
 */
static int LLC_IPv6Xmit(struct PktBuf *pTxPkb,
                        struct net_device *pNetDev)
{
  int Res = -ENOSYS;

  d_fnstart(D_INTERN, NULL, "(pTxPkb %p pNetDev %p)\n", pTxPkb, pNetDev);

  if ((NULL == pTxPkb) || (NULL == pNetDev))
  {
    d_error(D_WARN, NULL, "pTxPkb %p pNetDev %p\n", pTxPkb, pNetDev);
    Res = -EINVAL;
    goto Error;
  }

  // Transmit a packet on behalf of the Linux network stack.
  Res = LLC_IPv6TxPkb(pNetDev, pTxPkb);
  if (Res == 0)
  {
    pNetDev->stats.tx_packets++;
    pNetDev->stats.tx_bytes += pTxPkb->len;
    goto Success;
  }
  else
  {
    goto Error;
  }

Error:
  if (pNetDev != NULL)
  {
    pNetDev->stats.tx_dropped++;
    pNetDev->stats.tx_errors++;
  }
  if (pTxPkb != NULL)
  {
    PktBuf_Free(pTxPkb);
  }
Success:
  d_fnend(D_INTERN, NULL, "(pTxPkb %p pNetDev %p) = NETDEV_TX_OK (%d)\n",
          pTxPkb, pNetDev, Res);
  return NETDEV_TX_OK;
}


/**
 * @brief Check if the message is IP data
 * @return 1 = IP traffic, 0 = non-IP
 *
 * DENSO Note: This module originally used LLC LPD, which uses full SNAP
 * headers. In order to be compliant with ieee requirements, we use LLC EPD,
 * which does not include a full SNAP header and only includes the Ethertype.
 */
int LLC_IPv6CheckIPData(struct MKxRxPacket *pRxPkt,
                        void *pPriv)
{
  int Res = 0;
  struct PktBuf *pPkb = (struct PktBuf *)pPriv;
  /* Start DENSO Changes */
  //struct SNAPHeader *pSNAPHdr = NULL;
  uint16_t *pEthertype = NULL;
  /* End DENSO Changes */
  struct IEEE80211QoSHeader *pMACHdr = NULL;

  d_fnstart(D_INTERN, NULL, "(pRxPkt %p pPkb %p )\n", pRxPkt, pPkb);

  if ((NULL == pRxPkt) || (NULL == pPkb))
  {
    goto Error;
  }

  pMACHdr = (struct IEEE80211QoSHeader *)((unsigned char *) pPkb->data + sizeof(struct MKxRxPacket));

  switch (pMACHdr->FrameControl.Fields.SubType)
  {
    case MAC_FRAME_SUB_TYPE_QOS_DATA:
      /* Start DENSO Changes */
      //pSNAPHdr = (struct SNAPHeader *)
      //           ((unsigned char *) pMACHdr + sizeof(struct IEEE80211QoSHeader));
      pEthertype = (uint16_t *)
                   ((unsigned char *) pMACHdr + sizeof(struct IEEE80211QoSHeader));
      /* End DENSO Changes */
      break;
    case MAC_FRAME_SUB_TYPE_DATA:
      /* Start DENSO Changes */
      //pSNAPHdr = (struct SNAPHeader *)
      //           ((unsigned char *) pMACHdr + sizeof(struct IEEE80211Header));
      pEthertype = (uint16_t *)
                   ((unsigned char *) pMACHdr + sizeof(struct IEEE80211Header));
      /* End DENSO Changes */
      break;
    default:
      d_printf(D_INFO, NULL, "Invalid MAC frame sub type\n");
      goto Error;
  }
  /* Start DENSO Changes */
  //pPkb->protocol = pSNAPHdr->Type;
  pPkb->protocol = *pEthertype;

  //// check that the SNAP header is valid
  //if ((SNAP_HEADER_DSAP != pSNAPHdr->DSAP) ||
  //    (SNAP_HEADER_SSAP != pSNAPHdr->SSAP) ||
  //    (SNAP_HEADER_CONTROL != pSNAPHdr->Control))
  //{
  //  d_printf(D_ERR, NULL, "Corrupt SNAP header\n");
  //  goto Error;
  //}
  /* End DENSO Changes */

  // Only continue if this packet is an IPv6 packet
  if ((pPkb->protocol == htons(ETH_P_IPV6)) ||
      (pPkb->protocol == htons(ETH_P_ARP)))
  {
    Res = 1;
  }
Error:
  d_fnend(D_INTERN, NULL, "() = %d\n", Res);
  return Res;
}

/**
 * @brief Pass a 802.11 network packet to the Linux networking stack via llc-cch-ipv6
 *
 * @param pDev     'LLCDev' device handle
 * @param pPkb      The pktbuf where the packet is.
 *
 * @return  0 if result is ok \n
 *          <0 if result is failure .\n
 *          -EINVAL if invalid handle is pass\n
 *
 *  The pktbuf should be set to point at the 802.11 packet with receive metadata
 *  at the start. this function will add' fake' ethernet headers.
 *
 *
 */
int LLC_IPv6Rx(struct LLCDevice *pDev,
               struct MKxRxPacket *pRxPkt,
               void *pPriv)
{
/* DENSO */
/* don't ask, this is a hack into the data to get the IPv6 address; it works for now */
#define IPV6SOURCEADDRESS  ((uint8_t *)&pMk2Desc->RxPacketData.RxFrame[36])
#define IPV6SOURCEADDRESS0 pMk2Desc->RxPacketData.RxFrame[36]
#define IPV6SOURCEADDRESS1 pMk2Desc->RxPacketData.RxFrame[37]
/* END DENSO */
  int Res = -ENOSYS;
  struct net_device *pNetDev = NULL;
  struct PktBuf *pPkb = (struct PktBuf *)pPriv;
  struct IEEE80211QoSHeader *pMACHdr = NULL;
  struct MKxRxPacket *pMk2Desc = NULL;
  /* DENSO */
  int      idx;   /* ported from 5910; but really unused; leaving for now */
  static uint8_t IPv6Zero[IPV6_ADDR_LEN] =
  {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  static uint8_t MacZero[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  /* END DENSO */

  int Radio = -1;

  d_fnstart(D_INTERN, NULL, "(pDev %p pRxPkt %p pPkb %p )\n", pDev, pRxPkt, pPkb);

  if ((NULL == pDev) || (NULL == pRxPkt) || (NULL == pPkb))
  {
    Res = -EINVAL;
    goto Error;
  }

  pMk2Desc = (struct MKxRxPacket *) pPkb->data;
  pMACHdr = (struct IEEE80211QoSHeader *)((unsigned char *) pPkb->data + sizeof(struct MKxRxPacket));

  Radio = pMk2Desc->RxPacketData.RadioID;
  if (pMACHdr->FrameControl.Fields.Type == MAC_FRAME_TYPE_DATA)
  {
	/* DENSO; do this before any mods to the pkt */
    if (((IPV6SOURCEADDRESS0 != 0xfe) || (IPV6SOURCEADDRESS1 != 0x80)) &&
      (memcmp(IPV6SOURCEADDRESS, IPv6Zero, IPV6_ADDR_LEN) != 0) && memcmp((uint8_t *)pMACHdr->Address2, MacZero, ETH_ALEN))
      {
	    idx = ndp_table_add_entry(pDev, Radio, IPV6SOURCEADDRESS, (uint8_t *)pMACHdr->Address2);
        if (idx == -1) {
	      d_printf(D_ERR, NULL, "LLC: NDP table overflow\n");
        }
      } // link-local address
	/* END DENSO */
    PktBuf_Trim(pPkb, pRxPkt->Hdr.Len);
    // Remove the RxMsg Header so we are left with just the 802.11 header and snap header
    PktBuf_Pull(pPkb, sizeof(struct MKxRxPacket));
    if (Radio == MKX_RADIO_A)
    {
      pNetDev = pDev->pDriver->IPv6.pNetDevCCH;
    }
    else if (Radio == MKX_RADIO_B)
    {
      pNetDev = pDev->pDriver->IPv6.pNetDevSCH;
    }
    if (pNetDev == NULL)
    {
      d_printf(D_ERR, NULL, "RadioID(%d) Not correct or pNetDev(%p) Not correct\n", Radio, pNetDev);
      goto Error;
    }

    // Prepare the data payload RX header
    Res = LLC_IPv6RxPrepHeader(pDev->pDriver, pPkb);
    if (Res != 0)
    {
      d_printf(D_ERR, NULL, "Failed to prepare Rx Header = %d\n", Res);
      goto Error;
    }

    PktBuf_Orphan(pPkb);
    pPkb->dev = pNetDev;
    pPkb->pkt_type = PACKET_HOST;
    pPkb->ip_summed = CHECKSUM_UNNECESSARY;
    PktBuf_ResetMacHdr(pPkb);
    PktBuf_SetNetworkHdr(pPkb, ETH_HLEN);
    PktBuf_Pull(pPkb, ETH_HLEN);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0))
  // As of Kernl 4.11.0, the last_rx member has been removed
    pNetDev->last_rx = jiffies;
#endif
    pNetDev->stats.rx_packets++;
    pNetDev->stats.rx_bytes += pPkb->len;

    if (netif_running(pNetDev) == 0)
    {
      d_printf(D_ERR, NULL, "Netdev not running - discarding data\n");
      PktBuf_Free(pPkb);
    }
    else
    {
      if (in_atomic())
      {
        netif_rx(pPkb);
      }
      else
      {
        netif_rx_ni(pPkb);
      }
    }

    Res = 0;
  }
  else
  {
    // Not an IPv6 data frame so we can ignore
    Res = 0;
    goto Exit;
  }

Error:
Exit:
  d_fnend(D_INTERN, NULL, "(pDev %p pPkb %p) = %d\n", pDev, pPkb, Res);
  return Res;
}


/**
 * @brief Set a new Ethernet hardware address
 * @param pNetDev network device handle
 * @param SockAddr Socket address
 *
 */
static int LLC_IPv6Addr(struct net_device *pNetDev,
                        void *pSockAddr)
{
  int Res;

  Res = eth_mac_addr(pNetDev, pSockAddr);
  if (Res != 0)
  {
    goto Error;
  }
Error:
  return Res;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static const struct net_device_ops LLCIPv6Ops =
{
  .ndo_open = LLC_IPv6Open,
  .ndo_stop = LLC_IPv6Close,
  .ndo_set_config = LLC_IPv6Config,
  .ndo_start_xmit = LLC_IPv6Xmit,
  .ndo_do_ioctl = LLC_IPv6Ioctl,
  .ndo_tx_timeout = LLC_IPv6TxTimeout,
  .ndo_change_mtu = eth_change_mtu,
  .ndo_set_mac_address = LLC_IPv6Addr,
  .ndo_validate_addr = eth_validate_addr,
};

#endif // (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0))
static void LLC_IPv6DrvInfo(struct net_device *pNetDev,
                            struct ethtool_drvinfo *pInfo)
{
  if (sizeof(pInfo->driver) < sizeof(pNetDev->name))
  {
    strlcpy(pInfo->driver, pNetDev->name, sizeof(pInfo->driver));
  }
  else
  {
    strlcpy(pInfo->driver, pNetDev->name, sizeof(pNetDev->name));
  }
  strlcpy(pInfo->version, "0.0.0",       sizeof(pInfo->version));
}

static const struct ethtool_ops LLCIPv6EthOps =
{
  .get_drvinfo    = LLC_IPv6DrvInfo,
  .get_link       = ethtool_op_get_link,
  .get_ts_info    = ethtool_op_get_ts_info,
};
#endif // (LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0))


/**
 * @brief Setup the llc-sch-ipv6 or llc-cch-ipv6 interface
 *
 * @param pNetDev   The net_device created with alloc_netdev()
 *
 * @return None.\n
 *
 *  This function sets up llc-sch-ipv6 netdevice interfaces.
 *
 */
static void Do_IPv6Create(struct net_device *pNetDev)
{
  d_fnstart(D_INTERN, NULL, "(pNetDev %p)\n", pNetDev);

  if (NULL == pNetDev)
  {
    return;
  }

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(4,11,9))
  pNetDev->needs_free_netdev = false;
  pNetDev->priv_destructor = NULL;
#else
  pNetDev->destructor = free_netdev;
#endif
  ether_setup(pNetDev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0))
  pNetDev->ethtool_ops = &LLCIPv6EthOps;
#endif // (LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
  pNetDev->netdev_ops = &(LLCIPv6Ops);
#else
  pNetDev->open = LLC_IPv6Open;
  pNetDev->stop = LLC_IPv6Close;
  pNetDev->set_config = LLC_IPv6Config;
  pNetDev->hard_start_xmit = LLC_IPv6Xmit;
  pNetDev->do_ioctl = LLC_IPv6Ioctl;
  pNetDev->tx_timeout = LLC_IPv6TxTimeout;
  pNetDev->change_mtu = eth_change_mtu;
  pNetDev->set_mac_address = LLC_IPv6Addr;
  pNetDev->validate_addr = eth_validate_addr;
#endif // (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
  pNetDev->tx_queue_len = 0;
  pNetDev->hard_header_len = IPV6_DEV_HDR;
  pNetDev->type = ARPHRD_ETHER;
  get_random_bytes(pNetDev->dev_addr, ETH_ALEN);
  pNetDev->dev_addr[0] &= 0xfe; // clear the multicast bit
  pNetDev->dev_addr[0] |= 0x02; // set local assignment bit
  pNetDev->addr_len = ETH_ALEN;
  pNetDev->mtu = IPV6_DEV_MTU;

  d_fnend(D_INTERN, NULL, "(pNetDev %p) = void\n", pNetDev);
}

/**
 * @brief Initialize the 'wave-data' network interface
 *
 * @param pDot4     The Dot4 pointer
 *
 * @return  DOT4_RESULT_SUCCESS if result is ok \n
 *          DOT4_RESULT_FAILURE if result is failure .\n
 *          -EINVAL if invalid handle is pass\n
 *          -EBUSY if invalid handle is already opened\n
 *
 *  This function Initialize WAVE DATA netdevice.
 *
 *
 */
int LLC_IPv6Setup(struct LLCDriver *pDrv)
{
  int Res = -ENOSYS;
  struct net_device *pCCHNetDev = NULL;
  struct net_device *pSCHNetDev = NULL;
  struct LLCIPv6Priv *pLLCNetDevPriv = NULL;

  d_fnstart(D_INTERN, NULL, "(pDrv %p)\n", pDrv);

  if (NULL == pDrv)
  {
    Res = -EINVAL;
    goto ErrorParam;
  }


  // Ensure the net devices for sch and cch have not already been created
  if ((NULL != pDrv->IPv6.pNetDevCCH) ||
      (NULL != pDrv->IPv6.pNetDevSCH))
  {
    Res = -EBUSY;
    goto ErrorBusy;
  }

  // Allocate and create the CCH netdev
  pCCHNetDev = alloc_netdev(sizeof(struct LLCIPv6Priv), CCH_IPV6_DEV_NAME,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0))
                            NET_NAME_UNKNOWN,
#endif
                            Do_IPv6Create);

  if (pCCHNetDev == NULL)
  {
    d_printf(D_ERR, NULL, "Failed to allocate CCH netdev\n");
    goto ErrorCCHAlloc;
  }

  Res = register_netdev(pCCHNetDev);
  if (Res < 0)
  {
    d_printf(D_ERR, NULL, "Failed to register CCH netdev\n");
    goto ErrorCCHRegister;
  }

  // Store the 'LLC Device' pointer in the net device's private storage section
  pLLCNetDevPriv = netdev_priv(pCCHNetDev);
  pLLCNetDevPriv->pDrv = pDrv;

  d_printf(D_DBG, NULL, "pCCHNetDev: %p\n", pCCHNetDev);

  pDrv->IPv6.pNetDevCCH = pCCHNetDev;


// Allocate and create the SCH netdev
  pSCHNetDev = alloc_netdev(sizeof(struct LLCIPv6Priv), SCH_IPV6_DEV_NAME,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0))
                            NET_NAME_UNKNOWN,
#endif
                            Do_IPv6Create);

  if (pSCHNetDev == NULL)
  {
    d_printf(D_ERR, NULL, "Failed to allocate SCH netdev\n");
    goto ErrorSCHAlloc;
  }

  Res = register_netdev(pSCHNetDev);
  if (Res < 0)
  {
    d_printf(D_ERR, NULL, "Failed to register SCH netdev\n");
    goto ErrorSCHRegister;
  }

  // Store the 'LLC Device' pointer in the net device's private storage section
  pLLCNetDevPriv = netdev_priv(pSCHNetDev);
  pLLCNetDevPriv->pDrv = pDrv;

  d_printf(D_DBG, NULL, "pSCHNetDev: %p\n", pSCHNetDev);

  pDrv->IPv6.pNetDevSCH = pSCHNetDev;

  goto Success;

ErrorSCHRegister:
  free_netdev(pSCHNetDev);
ErrorSCHAlloc:
ErrorCCHRegister:
  free_netdev(pCCHNetDev);
ErrorCCHAlloc:
ErrorBusy:
ErrorParam:
Success:
  d_fnend(D_INTERN, NULL, "(pDrv %p) = %d\n", pDrv, Res);
  return Res;
}

/**
 * @brief De-Initialize the 'sch and cch' network interface
 *
 * @param pDrv     The 'LLC Device' handle (entire structure)
 *
 * @return  0 if result is OK \n
 *          <0 if result is failure .\n
 *          -EINVAL if invalid handle is pass\n
 *
 * This function de-Initialize the cch and sch ipv6 network interfaces
 *
 *
 */
int LLC_IPv6Release(struct LLCDriver *pDrv)
{
  int Res = -ENOSYS;
  d_fnstart(D_INTERN, NULL, "(pDrv %p)\n", pDrv);

  if (NULL == pDrv)
  {
    d_printf(D_DEBUG, NULL, "LLC Device not Initialised\n");
    Res = -EINVAL;
    goto Error;
  }

  // unregister the cch and sch netdevs
  unregister_netdev(pDrv->IPv6.pNetDevCCH);
  unregister_netdev(pDrv->IPv6.pNetDevSCH);

  pDrv->IPv6.pNetDevCCH = NULL;
  pDrv->IPv6.pNetDevSCH = NULL;
  Res = 0;

Error:
  d_fnend(D_INTERN, NULL, "(pDrv %p) = %d\n", pDrv, Res);
  return Res;
}

/**
 * @}
 */
