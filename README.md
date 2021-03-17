# libconntrack-cm

The libconntrack-cm library implements connection tracking for the RDMA CM
protocol.

## API

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
