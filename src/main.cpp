/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#include <libconntrack-cm.h>
#include "cm_connection_tracker.h"
#include "parser.h"

struct ctcm_context {
    ctcm_context() :
        parser{},
        tracker{parser}
    {}

    parser_context parser;
    cm_connection_tracker tracker;
};

ctcm_public
struct ctcm_context *ctcm_create()
{
    return new ctcm_context{};
}

ctcm_public
void ctcm_destroy(struct ctcm_context *ctcm)
{
    delete ctcm;
}

ctcm_public
int ctcm_dynfield_offsets(struct ctcm_context *ctcm,
                          struct ctcm_dynfield_offsets* offsets)
{
    if (offsets->size < sizeof(*offsets)) {
        errno = ENOMEM;
        return -1;
    }

    offsets->bth = ctcm->parser.dynfield_bth_offset();
    offsets->mad = ctcm->parser.dynfield_mad_offset();

    return 0;
}

ctcm_public
int ctcm_parse_packet(const struct ctcm_context *ctcm,
                      struct rte_mbuf *packet)
{
    ctcm->parser.parse_packet(packet);
    
    return 0;
}

ctcm_public
int ctcm_process_packet(struct ctcm_context *ctcm,
                        enum ctcm_direction dir,
                        const struct rte_mbuf *packet)
{
    if (ctcm->parser.mbuf_mad(packet))
        ctcm->tracker.process(packet, dir);
    
    return 0;
}

ctcm_public
uint32_t ctcm_query_ipv4(const struct ctcm_context *ctcm,
                         in_addr_t dest_ip, uint32_t dqpn)
{
    return ctcm->tracker.get_source_qpn(flow_key(dest_ip, dqpn));
}
