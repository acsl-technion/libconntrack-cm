/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#include "gtest/gtest.h"

#include <boost/algorithm/hex.hpp>

#include <libconntrack-cm.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

class CTCM : public ::testing::Test { 
public:
    ctcm_context *ctcm;

    void SetUp() {
        char * args[] = {};
        int ret = rte_eal_init(0, args);
        ASSERT_EQ(0, ret);
        ctcm = ctcm_create();
        ASSERT_TRUE(ctcm);
    }

    void TearDown() { 
        ctcm_destroy(ctcm);
        int ret = rte_eal_cleanup();
        ASSERT_EQ(0, ret);
    }
};

struct cnp {
    struct iphdr ip;
    struct udphdr udp;
    uint8_t reserved_2[CTCM_BTH_LENGTH + CTCM_CNP_LENGTH + CTCM_ICRC_LENGTH];
};

/* Expected output based on
 * https://community.mellanox.com/s/article/rocev2-cnp-packet-format-example
 *
 * pkt = IP(src='22.22.22.8', dst='22.22.22.7', id=0x98c6, flags='DF',
 *          ttl=0x20, tos=0x89)/ \
 *       UDP(sport=56238, dport=4791, chksum=0)/ \
 *       cnp(dqpn=0xd2)
 */
const char expected_cnp[] =
    "4589003C98C640002011692716161608"
    "16161607DBAE12B7002800008100FFFF"
    "400000D2000000000000000000000000"
    "0000000000000000DAFC1BEF";

void hexdump(const std::string& strbuf)
{
    for (size_t i = 0; i < strbuf.size(); i += 16) {
        size_t cur_end = std::min(i + 16, strbuf.size());
        char line[16 * 2 + 1];
        auto it = std::begin(strbuf);
        boost::algorithm::hex(it + i, it + cur_end, line);
        line[(cur_end - i) * 2] = 0;
        std::cout << std::hex << std::setfill('0') << std::setw(4) << i << ": " << line << "\n";
    }
}

TEST_F(CTCM, cnp_gen)
{
    cnp cnpacket{};
    rte_mbuf p{};
    p.buf_addr = &cnpacket;
    p.buf_len = sizeof(cnpacket);
    ctcm_fill_cnp_template(ctcm, &p);
    inet_aton("22.22.22.8", (in_addr *)&cnpacket.ip.saddr);
    inet_aton("22.22.22.7", (in_addr *)&cnpacket.ip.daddr);
    cnpacket.ip.id = htons(0x98c6);
    cnpacket.ip.ttl = 0x20;
    cnpacket.ip.tos = 0x89;
    cnpacket.udp.uh_sport = htons(56238);
    ctcm_generate_cnp(ctcm, &p, 0xd2);

    const std::string generated((char *)&cnpacket, sizeof(cnpacket));

    cnp expected_cnpacket;
    boost::algorithm::unhex(std::string(expected_cnp), (char *)&expected_cnpacket);
    expected_cnpacket.ip.check = 0;
    const std::string expected((char *)&expected_cnpacket, sizeof(expected_cnpacket));

    if (generated != expected) {
        std::cout << "Expected: \n";
        hexdump(expected);
        std::cout << "Generated: \n";
        hexdump(generated);
    }
    EXPECT_EQ(generated, expected);
}
