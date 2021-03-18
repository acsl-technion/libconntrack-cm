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
void ctcm_fill_cnp_template(const struct ctcm_context *ctcm,
                            struct ctcm_packet *cnp)
{
    struct iphdr *ip = cnp->l3.ipv4;
    memset(ip, 0, sizeof(iphdr) + sizeof(udphdr) + CTCM_CNP_TOTAL_LENGTH);
    ip->version = 4;
    ip->ihl = sizeof(iphdr) / 4;
    ip->ttl = 255;
    ip->protocol = IPPROTO_UDP;
    ip->frag_off = htons(IP_DF);
	ip->tos = 0xc2; // DSCP 48, ECT(0)
	ip->tot_len = htons(sizeof(iphdr) + sizeof(udphdr) + CTCM_CNP_TOTAL_LENGTH);

    cnp->udp = reinterpret_cast<udphdr *>(ip + 1);
    auto udp = cnp->udp;
    udp->uh_sport = htons(0xf000);
    udp->uh_dport = htons(UDP_PORT_ROCE_V2);
    udp->uh_ulen = htons(sizeof(udphdr) + CTCM_CNP_TOTAL_LENGTH);

    cnp->bth = reinterpret_cast<rxe_bth *>(udp + 1);
    auto bth = cnp->bth;
	__bth_set_opcode(bth, 0x81); // RoCE CNP opcode
	__bth_set_becn(bth, 1);
	__bth_set_pkey(bth, 0xffff);
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
                       struct ctcm_packet *cnp,
                       uint32_t dest_qpn)
{
    cnp->udp = reinterpret_cast<udphdr *>(cnp->l3.ipv4 + 1);
    cnp->bth = reinterpret_cast<rxe_bth *>(cnp->udp + 1);

    __bth_set_qpn(cnp->bth, dest_qpn);

	// ICRC
	cnp_icrc_pseudo_packet phdr;
	memset(&phdr.reserved_1, 0xff, sizeof(phdr.reserved_1));
    memcpy(&phdr.ip, cnp->l3.ipv4, sizeof(phdr) - sizeof(phdr.reserved_1));

	phdr.ip.check = 0xffff;
	phdr.ip.ttl = 0xff;
	phdr.ip.tos = 0xff;
	phdr.udp.check = 0xffff;
	phdr.bth.qpn = htonl(BTH_FECN_MASK | BTH_BECN_MASK | BTH_RESV6A_MASK | __bth_qpn(&phdr.bth));
	uint32_t icrc = rte_net_crc_calc(&phdr, sizeof(phdr), RTE_NET_CRC32_ETH);

	uint32_t *cnp_icrc = (uint32_t *)((char *)(cnp->bth + 1) + CTCM_CNP_LENGTH);
    *cnp_icrc = icrc;
}