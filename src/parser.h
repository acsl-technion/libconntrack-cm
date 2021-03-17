/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#pragma once

#include "libconntrack-cm.h"

struct ib_mad_hdr;

/* Return the MAD header if the UDP packet contains a CM MAD. */
const ib_mad_hdr *parse_packet(ctcm_packet& packet);