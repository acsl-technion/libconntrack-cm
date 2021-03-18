/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#pragma once

#include "libconntrack-cm.h"

struct ib_mad_hdr;

/* Return the MAD header if the UDP packet contains a CM MAD. */
ib_mad_hdr *parse_packet(ctcm_packet& packet);

#define UDP_PORT_ROCE_V2 4791

#define ctcm_public __attribute__((visibility("default")))