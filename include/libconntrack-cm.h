/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <netinet/in.h>

struct iphdr;
struct ib_mad_hdr;

enum ctcm_direction {
    CTCM_FROM_HOST,
    CTCM_FROM_NET,
};

/* Pointer to a packet to be processed. Includes pointers to the IP header, UDP
 * header, and MAD header. */
struct ctcm_packet {
    union {
        const struct iphdr *ipv4;
    } l3;
    const struct udphdr *udp;
    const struct rxe_bth *bth;
    const struct ib_mad_hdr *mad;
};

struct ctcm_context;

struct ctcm_context* ctcm_create();
void ctcm_destroy(struct ctcm_context* ctcm);

/* Parse an array of packets. The packet's udp header needs to be valid. 
 * Packets are updated in-place with mad header if valid. */
int ctcm_parse_packets(const struct ctcm_context *ctcm,
                       struct ctcm_packet *packets,
                       unsigned int num_packets);

/* Process an array of packets. dir determines whether packets are coming from the
 * local interface or the remote one. Requires that the l3 and mad header pointers 
 * are valid. */
int ctcm_process_packets(
    struct ctcm_context *ctcm,
    enum ctcm_direction dir,
    const struct ctcm_packet* packets,
    unsigned int num_packets);

/* Return the source QP number for a given flow, determined by the 
 * destination IP and QP number */
uint32_t ctcm_query_ipv4(const struct ctcm_context *ctcm,
                         in_addr_t dest_ip, uint32_t dqpn);

#ifdef __cplusplus
}
#endif