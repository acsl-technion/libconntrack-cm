/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#include <libconntrack-cm.h>
#include "cm_connection_tracker.h"

#define ctcm_public __attribute__((visibility("default")))

struct ctcm_context {
    cm_connection_tracker tracker = {};
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
int ctcm_process_packets(struct ctcm_context *ctcm,
                         enum ctcm_direction dir,
                         struct ctcm_packet *packets,
                         unsigned int num_packets)
{
    auto end = packets + num_packets;
    for (auto packet = packets; packet != end; ++packet)
        ctcm->tracker.process(*packet, dir);
    
    return num_packets;
}

ctcm_public
uint32_t ctcm_query_ipv4(struct ctcm_context *ctcm,
                         in_addr_t dest_ip, uint32_t dqpn)
{
    return ctcm->tracker.get_source_qpn(flow_key(dest_ip, dqpn));
}