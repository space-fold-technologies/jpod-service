#ifndef __DAEMON_DOMAIN_NETWORKING_FREEBSD_INTERNAL__
#define __DAEMON_DOMAIN_NETWORKING_FREEBSD_INTERNAL__

#include <cstdint>
namespace domain::networking::freebsd
{
    const uint8_t RTM_ADD = 0x1;    /* (1) Add Route */
    const uint8_t RTM_DELETE = 0x2; /* (1) Delete Route */
    const uint8_t RTM_VERSION = 5;  /* Up the ante and ignore older versions */
    const uint32_t RTF_UP = 0x1;
    const uint32_t RTF_GATEWAY = 0x2;
    const uint32_t RTF_STATIC = 0x800;
    const uint32_t RTF_PINNED = 0x100000;
    const uint32_t RTA_DST = 0x1;
    const uint32_t RTA_GATEWAY = 0x2;
    const uint32_t RTA_NETMASK = 0x4;
    struct rt_metrics
    {
        uint64_t rmx_locks;     /* Kernel must leave these values alone */
        uint64_t rmx_mtu;       /* MTU for this path */
        uint64_t rmx_hopcount;  /* max hops expected */
        uint64_t rmx_expire;    /* lifetime for route, e.g. redirect */
        uint64_t rmx_recvpipe;  /* inbound delay-bandwidth product */
        uint64_t rmx_sendpipe;  /* outbound delay-bandwidth product */
        uint64_t rmx_ssthresh;  /* outbound gateway buffer limit */
        uint64_t rmx_rtt;       /* estimated round trip time */
        uint64_t rmx_rttvar;    /* estimated rtt variance */
        uint64_t rmx_pksent;    /* packets sent using this route */
        uint64_t rmx_weight;    /* route weight */
        uint64_t rmx_nhidx;     /* route nexhop index */
        uint64_t rmx_filler[2]; /* will be used for T/TCP later */
    };
    struct rt_msg_header
    {
        uint16_t rtm_msglen;
        uint8_t rtm_version;
        uint8_t rtm_type;
        uint16_t rtm_index;
        uint16_t _rtm_spare1;
        uint32_t rtm_flags;
        uint32_t rtm_addrs;
        uint32_t rtm_pid;
        uint32_t rtm_seq;
        uint32_t rtm_errno;
        uint32_t rtm_fmask;
        uint64_t rtm_inits;
        rt_metrics rtm_rmx;
    };
    template<typename T>
    struct rt_message
    {
        rt_msg_header header;
        T payload;
    };
}

#endif // __DAEMON_DOMAIN_NETWORKING_FREEBSD_INTERNAL__