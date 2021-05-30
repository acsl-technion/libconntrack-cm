/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#include <libconntrack-cm.h>

#include <netinet/ip.h>
#include <netinet/udp.h>
#include "rxe_hdr.h"
#include "parser.h"

#include <rte_net_crc.h>

/* Fill a packet with CNP header templates, including IPv4 header (except
 * addresses), UDP header, and BTH. */
ctcm_public
int ctcm_fill_cnp_template(const struct ctcm_context *ctcm,
                           struct rte_mbuf *cnp)
{
    cnp->packet_type = RTE_PTYPE_L4_UDP | RTE_PTYPE_L3_IPV4;
    cnp->l3_len = sizeof(iphdr);
    cnp->l4_len = sizeof(udphdr);
    const size_t len = sizeof(iphdr) + sizeof(udphdr) + CTCM_CNP_TOTAL_LENGTH;
    auto ip = reinterpret_cast<iphdr *>(rte_pktmbuf_append(cnp, len));
    if (!ip)
        return -1;
    memset(ip, 0, len);
    ip->version = 4;
    ip->ihl = sizeof(iphdr) / 4;
    ip->ttl = 255;
    ip->protocol = IPPROTO_UDP;
    ip->frag_off = htons(IP_DF);
    ip->tos = 0xc2; // DSCP 48, ECT(0)
    ip->tot_len = htons(len);

    auto udp = reinterpret_cast<udphdr *>(ip + 1);
    udp->uh_sport = htons(0xf000);
    udp->uh_dport = htons(UDP_PORT_ROCE_V2);
    udp->uh_ulen = htons(sizeof(udphdr) + CTCM_CNP_TOTAL_LENGTH);

    auto bth = reinterpret_cast<rxe_bth *>(udp + 1);
    __bth_set_opcode(bth, 0x81); // RoCE CNP opcode
    __bth_set_becn(bth, 1);
    __bth_set_pkey(bth, 0xffff);

    return 0;
}

struct cnp_icrc_pseudo_packet {
    uint8_t reserved_1[8];
    struct iphdr ip;
    struct udphdr udp;
    struct rxe_bth bth;
    uint8_t reserved_2[CTCM_CNP_LENGTH];
};

/* Complete a CNP for a specific QP and calculate ICRC. */
ctcm_public
void ctcm_generate_cnp(const struct ctcm_context *ctcm,
                       struct rte_mbuf *cnp,
                       uint32_t dest_qpn)
{
    auto ip = mbuf_ip(cnp);
    auto bth = mbuf_bth(cnp);

    __bth_set_qpn(bth, dest_qpn);

    // ICRC
    cnp_icrc_pseudo_packet phdr;
    memset(&phdr.reserved_1, 0xff, sizeof(phdr.reserved_1));
    memcpy(&phdr.ip, ip, sizeof(phdr) - sizeof(phdr.reserved_1));

    phdr.ip.check = 0xffff;
    phdr.ip.ttl = 0xff;
    phdr.ip.tos = 0xff;
    phdr.udp.check = 0xffff;
    phdr.bth.qpn = htonl(BTH_FECN_MASK | BTH_BECN_MASK | BTH_RESV6A_MASK | __bth_qpn(&phdr.bth));
    uint32_t icrc = rte_net_crc_calc(&phdr, sizeof(phdr), RTE_NET_CRC32_ETH);

    uint32_t *cnp_icrc = (uint32_t *)((char *)(bth + 1) + CTCM_CNP_LENGTH);
    *cnp_icrc = icrc;
}
