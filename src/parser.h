/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#pragma once

#include "libconntrack-cm.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <rte_mbuf.h>
#ifdef __cplusplus
}
#endif

struct ib_mad_hdr;

class parser_context {
public:
    parser_context();

    const struct rxe_bth *mbuf_bth(const struct rte_mbuf *packet) const
    {
        return ctcm_mbuf_get_bth(&dynfield_offsets, const_cast<rte_mbuf *>(packet));
    }

    void mbuf_bth(struct rte_mbuf *packet, struct rxe_bth *bth) const
    {
        ctcm_mbuf_set_bth(&dynfield_offsets, packet, bth);
    }

    const struct ib_mad_hdr *mbuf_mad(const struct rte_mbuf *packet) const
    {
        return ctcm_mbuf_get_mad(&dynfield_offsets, const_cast<rte_mbuf *>(packet));
    }

    void mbuf_mad(struct rte_mbuf *packet, struct ib_mad_hdr *mad) const
    {
        ctcm_mbuf_set_mad(&dynfield_offsets, packet, mad);
    }

    /* Return the MAD header if the UDP packet contains a CM MAD. */
    ib_mad_hdr *parse_packet(rte_mbuf* packet) const;

    int dynfield_bth_offset() const { return dynfield_offsets.bth; }
    int dynfield_mad_offset() const { return dynfield_offsets.mad; }

private:
    struct ctcm_dynfield_offsets dynfield_offsets;
};

static inline const struct iphdr *mbuf_ip(const struct rte_mbuf *packet)
{
    assert(RTE_ETH_IS_IPV4_HDR(packet->packet_type));
    return rte_pktmbuf_mtod_offset(packet, struct iphdr *, packet->l2_len);
}

static inline const struct udphdr *mbuf_udp(const struct rte_mbuf *packet)
{
    assert(packet->l3_len && 
           ((packet->packet_type & RTE_PTYPE_L4_MASK) == RTE_PTYPE_L4_UDP));
    return rte_pktmbuf_mtod_offset(packet, struct udphdr *, packet->l2_len + packet->l3_len);
}

static inline const struct rxe_bth *mbuf_bth(const struct rte_mbuf *packet)
{
    assert(packet->l3_len && packet->l4_len);
    return rte_pktmbuf_mtod_offset(packet, struct rxe_bth *,
                                   packet->l2_len + packet->l3_len +
                                   packet->l4_len);
}

static inline struct iphdr *mbuf_ip(struct rte_mbuf *packet)
{
    return const_cast<iphdr *>(mbuf_ip(static_cast<const rte_mbuf *>(packet)));
}

static inline struct udphdr *mbuf_udp(struct rte_mbuf *packet)
{
    return const_cast<udphdr *>(mbuf_udp(static_cast<const rte_mbuf *>(packet)));
}

static inline struct rxe_bth *mbuf_bth(struct rte_mbuf *packet)
{
    return const_cast<rxe_bth *>(mbuf_bth(static_cast<const rte_mbuf *>(packet)));
}

#define UDP_PORT_ROCE_V2 4791

#define ctcm_public __attribute__((visibility("default")))
