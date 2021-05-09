# libconntrack-cm

The libconntrack-cm library implements connection tracking for the RDMA CM
protocol.

## API

### Connection tracking and parsing

Initialize the connection tracking data structures by calling `ctcm_create`
(and cleanup with `ctcm_destroy`).

Pass packets to the library using

    int ctcm_process_packets(
        struct ctcm_context *ctcm,
        enum ctcm_direction dir,
        struct ctcm_packet* packets,
        unsigned int num_packets);

Passing an array of pre-parsed packet pointers which point to the packet L3 
header as well as the MAD header.

If needed, one may use `ctcm_parse_packets` to parse the BTH / MAD headers
and filter the CM packets.

You can then query the data structure to find the source QP number of a given
flow by calling `ctcm_query_ipv4`.

### CNP generation

The library provide two helper functions for RoCE v2 Congestion Notification 
Packet (CNP) generation.

The `ctcm_fill_cnp_template` function fills a buffer with a template of a CNP
that can be copied and reused for different destinations and flows, and the
`ctcm_generate_cnp` function fills out destination QP number and calculates 
the ICRC. An application using this API should use `ctcm_fill_cnp_template`,
then fill out the remainder fields (IPv4 source/destination IP), and call
`ctcm_generate_cnp` to fill out the remaining fields. The code does not fill out
IP checksum, as this can be done using common NIC offloads.


## Dependencies

The library is written in C++17. It uses `meson` to build, and relies on DPDK 
and Boost for its implementation. Unit tests are written with gtest.

## Compilation

Use the following commands to build:

    meson build
    ninja -C build

On BlueField-2, set `PKG_CONFIG_PATH` to allow meson to find dpdk:

    PKG_CONFIG_PATH=/opt/mellanox/dpdk/lib/aarch64-linux-gnu/pkgconfig meson build
    ninjc -C build

For system-wide installation:

    ninja -C build install

# License

Original code is licensed under [BSD-2-Clause](https://spdx.org/licenses/BSD-2-Clause.html),
but some header files needed for parsing RoCE packets were taken from the Linux 
kernel and are licensed [GPL-2.0](https://spdx.org/licenses/GPL-2.0)/[GPL-2.0-only](https://spdx.org/licenses/GPL-2.0-only) or [Linux-OpenIB](https://spdx.org/licenses/Linux-OpenIB.html).