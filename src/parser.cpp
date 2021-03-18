/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#include "parser.h"

#include <arpa/inet.h>
#include <netinet/udp.h>
#include "rxe_hdr.h"
#include "ib_pack.h"
#include "ib_mad.h"

static rxe_bth *extract_bth(udphdr *udp, size_t& len)
{
    if (len < sizeof(rxe_bth) + sizeof(udphdr) + 4 /* icrc */ ||
        udp->uh_dport != htons(UDP_PORT_ROCE_V2))
        return nullptr;
    
    len -= sizeof(rxe_bth) + 4;

    return reinterpret_cast<rxe_bth *>(udp + 1);
}

static rxe_deth *extract_deth(rxe_bth *bth, size_t& len)
{
    if (__bth_opcode(const_cast<rxe_bth *>(bth)) != IB_OPCODE_UD_SEND_ONLY ||
        len < sizeof(rxe_deth))
        return nullptr;
    
    // TODO check ICRC
    // Validate BTH & DETH
    len -= sizeof(rxe_deth);

    return reinterpret_cast<rxe_deth *>(bth + 1);
}

static ib_mad_hdr *extract_mad(rxe_deth *deth, size_t& len)
{
    if (len < sizeof(ib_mad_hdr))
        return nullptr;
    return reinterpret_cast<ib_mad_hdr *>(deth + 1);
}

ib_mad_hdr *parse_packet(ctcm_packet& packet)
{
    size_t len = ntohs(packet.udp->len);
    packet.bth = extract_bth(packet.udp, len);
    if (!packet.bth)
        return nullptr;
    // TODO: validate BTH
    auto deth = extract_deth(packet.bth, len);
    if (!deth)
        return nullptr;
    // TODO: validate DETH

    if (__bth_qpn(const_cast<rxe_bth *>(packet.bth)) != 1)
        return nullptr;
    
    auto mad = extract_mad(deth, len);
    if (!mad)
        return nullptr;
    
    // TODO: check base version, class version, etc.
    if (mad->mgmt_class != IB_MGMT_CLASS_CM)
        return nullptr;
    
    packet.mad = mad;
    return mad;
}