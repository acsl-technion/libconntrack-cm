/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#include "parser.h"

#include <arpa/inet.h>
#include <netinet/udp.h>
#include "rxe_hdr.h"
#include "ib_pack.h"
#include "ib_mad.h"

#include <stdexcept>
#include <array>

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

ib_mad_hdr *parser_context::parse_packet(rte_mbuf* packet) const
{
    udphdr *udp = mbuf_udp(packet);
    size_t len = ntohs(udp->len);
    auto bth = extract_bth(udp, len);
    mbuf_bth(packet, bth);
    mbuf_mad(packet, nullptr);
    if (!bth)
        return nullptr;
    // TODO: validate BTH
    auto deth = extract_deth(bth, len);
    if (!deth)
        return nullptr;
    // TODO: validate DETH

    if (__bth_qpn(const_cast<rxe_bth *>(bth)) != 1)
        return nullptr;
    
    auto mad = extract_mad(deth, len);
    if (!mad)
        return nullptr;
    
    // TODO: check base version, class version, etc.
    if (mad->mgmt_class != IB_MGMT_CLASS_CM)
        return nullptr;
    
    mbuf_mad(packet, mad);
    return mad;
}

parser_context::parser_context()
{
    std::array dynfields{
        rte_mbuf_dynfield{
            "BTH",
            sizeof(uint16_t),
            sizeof(uint16_t),
        },
        rte_mbuf_dynfield{
            "IB_MAD_HDR",
            sizeof(uint16_t),
            sizeof(uint16_t),
        },
    };

    for (size_t i = 0; i < dynfields.size(); ++i) {
        int ret = rte_mbuf_dynfield_register(&dynfields[i]);
        if (ret < 0)
            throw std::runtime_error("error registering dpdk dynamic rte_mbuf field");
        switch (i) {
        case 0:
            dynfield_offsets.bth = ret;
            break;
        case 1:
            dynfield_offsets.mad = ret;
            break;
        default:
            assert(0);
        }
    }
}
