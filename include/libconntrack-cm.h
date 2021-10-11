/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdint.h>
#include <netinet/in.h>
#include <rte_mbuf.h>
#include <rte_mbuf_dyn.h>

struct iphdr;
struct ib_mad_hdr;

enum ctcm_direction {
    CTCM_FROM_HOST,
    CTCM_FROM_NET,
};

struct ctcm_context;

struct ctcm_context* ctcm_create();
void ctcm_destroy(struct ctcm_context* ctcm);

struct ctcm_dynfield_offsets {
    uint32_t size;
    int bth;
    int mad;
};

/* Return the offsets of the mbuf header offset dynamic fields registered by
 * libconntrack-cm. */
int ctcm_dynfield_offsets(struct ctcm_context *ctcm,
                          struct ctcm_dynfield_offsets* offsets);

static inline uint16_t* ctcm_mbuf_bth_offset(
    const struct ctcm_dynfield_offsets* dynfield_offsets,
    struct rte_mbuf *packet)
{
    return RTE_MBUF_DYNFIELD(packet, dynfield_offsets->bth, uint16_t*);
}

static inline uint16_t* ctcm_mbuf_mad_offset(
    const struct ctcm_dynfield_offsets* dynfield_offsets,
    struct rte_mbuf *packet)
{
    return RTE_MBUF_DYNFIELD(packet, dynfield_offsets->mad, uint16_t*);
}

static inline struct rxe_bth *ctcm_mbuf_get_bth(
    const struct ctcm_dynfield_offsets* dynfield_offsets,
    struct rte_mbuf *packet)
{
    uint16_t offset = *ctcm_mbuf_bth_offset(dynfield_offsets, packet);
    if (offset) {
        assert(offset < rte_pktmbuf_pkt_len(packet));
        return rte_pktmbuf_mtod_offset(packet, rxe_bth *, offset);
    } else {
        return nullptr;
    }
}

static inline void ctcm_mbuf_set_bth(
    const struct ctcm_dynfield_offsets* dynfield_offsets,
    struct rte_mbuf *packet,
    struct rxe_bth *bth)
{
    size_t offset = bth ?
        ((char *)bth - rte_pktmbuf_mtod(packet, char *)) : 0;
    assert(offset < rte_pktmbuf_pkt_len(packet));
    *ctcm_mbuf_bth_offset(dynfield_offsets, packet) = (uint16_t)(offset);
}

static inline struct ib_mad_hdr *ctcm_mbuf_get_mad(
    const struct ctcm_dynfield_offsets* dynfield_offsets,
    struct rte_mbuf *packet)
{
    uint16_t offset = *ctcm_mbuf_mad_offset(dynfield_offsets, packet);
    if (offset) {
        assert(offset < rte_pktmbuf_pkt_len(packet));
        return rte_pktmbuf_mtod_offset(packet, ib_mad_hdr *, offset);
    } else {
        return nullptr;
    }
}

static inline void ctcm_mbuf_set_mad(
    const struct ctcm_dynfield_offsets* dynfield_offsets,
    struct rte_mbuf *packet,
    struct ib_mad_hdr *mad)
{
    size_t offset = mad ?
        ((char *)mad - rte_pktmbuf_mtod(packet, char *)) : 0;
    assert(offset < rte_pktmbuf_pkt_len(packet));
    *ctcm_mbuf_mad_offset(dynfield_offsets, packet) = (uint16_t)(offset);
}

/* Parse a packet. The packet's l2_len/l3_len needs to be valid.
 * Packets are updated with bth/mad header offset if valid. */
int ctcm_parse_packet(const struct ctcm_context *ctcm,
                       struct rte_mbuf *packet);

/* Process a packet. dir determines whether packets are coming from the
 * local interface or the remote one. Requires that the l3 and mad offset fields
 * are valid. */
int ctcm_process_packet(
    struct ctcm_context *ctcm,
    enum ctcm_direction dir,
    const struct rte_mbuf* packet);

/* Return the source QP number for a given flow, determined by the 
 * destination IP and QP number */
uint32_t ctcm_query_ipv4(const struct ctcm_context *ctcm,
                         in_addr_t dest_ip, uint32_t dqpn);

#define CTCM_UDP_LENGTH 8
#define CTCM_BTH_LENGTH 12
#define CTCM_ICRC_LENGTH 4
#define CTCM_CNP_LENGTH 16
#define CTCM_CNP_TOTAL_LENGTH (CTCM_BTH_LENGTH + \
    CTCM_CNP_LENGTH + CTCM_ICRC_LENGTH)

/* Fill a packet with CNP header templates, including IPv4 header (except
 * addresses), UDP header, and BTH. */
int ctcm_fill_cnp_template(const struct ctcm_context *ctcm,
                           struct rte_mbuf *cnp);

/* Complete a CNP for a specific QP and calculate ICRC. */
void ctcm_generate_cnp(const struct ctcm_context *ctcm,
                       struct rte_mbuf *cnp,
                       uint32_t dest_qpn);

#ifdef __cplusplus
}
#endif
