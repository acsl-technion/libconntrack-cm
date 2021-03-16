/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2020-2021 Haggai Eran
 */

#pragma once

#include <rte_log.h>

#define CTCM_LOGTYPE RTE_LOGTYPE_USER1
#define CTCM_PREFIX "conntrack-cm: "

#define log_debug(...) \
    rte_log(RTE_LOG_DEBUG, CTCM_LOGTYPE, CTCM_PREFIX __VA_ARGS__)
