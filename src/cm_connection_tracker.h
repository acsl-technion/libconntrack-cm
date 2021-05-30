/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2020-2021 Haggai Eran
 */

#pragma once

#include "libconntrack-cm.h"
#include "parser.h"

#include "ib_cm.h"

#include <netinet/ip.h>

#include <unordered_map>
#include <tuple>
#include <functional>
#include <vector>
#include <memory>

#include <boost/preprocessor.hpp>
#include <boost/functional/hash.hpp>

typedef uint32_t id_t;
typedef uint32_t qpn_t;
typedef std::tuple<in_addr_t, qpn_t> flow_key; /* Dest IP, Dest QPN */
typedef std::unordered_map<flow_key, qpn_t, boost::hash<flow_key>> qpn_map_t;

#define FLOW_STATES \
	(IDLE) \
	(REQ_SENT) \
	(REQ_RCVD) \
	(MRA_REQ_RCVD) \
	(MRA_REQ_SENT) \
	(MRA_REP_SENT) \
	(MRA_REP_RCVD) \
	(REP_SENT) \
	(REP_RCVD) \
	(ESTABLISHED) \
	(DREQ_SENT) \
	(DREQ_RCVD) \
	(TIMEWAIT)

using cm_flow_key_base = std::tuple<in_addr_t, id_t>;

struct cm_flow_key : public cm_flow_key_base
{
	cm_flow_key() {}
	cm_flow_key(const cm_flow_key_base &base) : cm_flow_key_base(base) {}
	operator bool() const { return std::get<0>(*this) && std::get<1>(*this); }

	in_addr_t addr() const { return std::get<0>(*this); }
	id_t id() const { return std::get<1>(*this); }

	static cm_flow_key from_dest(const rte_mbuf *p, id_t remote_id) {
		return std::make_tuple(mbuf_ip(p)->daddr, remote_id);
	}

	static cm_flow_key from_src(const rte_mbuf *p, id_t remote_id) {
		return std::make_tuple(mbuf_ip(p)->saddr, remote_id);
	}
};

struct flow_state
{
	flow_state() {}

	enum {
	    BOOST_PP_SEQ_ENUM(FLOW_STATES)
	} state = IDLE;

	static const char *state_names[];

	id_t local_id = 0;
	cm_flow_key remote_id = cm_flow_key();
	qpn_t local_qpn = 0;
	qpn_t remote_qpn = 0;

	bool in_qpn_map = false;

	auto get_cm_flow_key() const
	{ return remote_id; }

	auto get_flow_key() const
	{
		auto remote_ip = std::get<0>(remote_id);
		return std::make_tuple(remote_ip, remote_qpn);
	}

	auto flow_value() const
	{ return qpn_map_t::value_type(get_flow_key(), local_qpn); }

	void log(const char *func, const char *msg = "");
};

using flow_state_ptr = std::shared_ptr<flow_state>;

class cm_connection_tracker
{
public:
	cm_connection_tracker(parser_context& parser);

	void process(const rte_mbuf *p, enum ctcm_direction dir);

	qpn_t get_source_qpn(flow_key flow) const
	{
		auto it = qpn_map.find(flow);
		if (it != qpn_map.end())
			return it->second;
		else
			return 0;
	}

private:
        parser_context &parser;
	std::unordered_map<id_t, flow_state_ptr> local_map;
	std::unordered_map<cm_flow_key, flow_state_ptr, boost::hash<cm_flow_key>> remote_map;

	flow_state_ptr get_flow(id_t local_id, cm_flow_key remote_id = cm_flow_key());
	void erase_flow(id_t local_id, cm_flow_key remote_id = cm_flow_key());
	flow_state_ptr add_new_flow(id_t local_id, cm_flow_key remote_id = cm_flow_key());

	void on_established(flow_state_ptr state);
	void on_disconnected(flow_state_ptr state);

	void req_sent(const rte_mbuf *);
	void rej_sent(const rte_mbuf *);
	void rep_sent(const rte_mbuf *);
	void rtu_sent(const rte_mbuf *);
	void dreq_sent(const rte_mbuf *);
	void drep_sent(const rte_mbuf *);

	void req_received(const rte_mbuf *);
	void rej_received(const rte_mbuf *);
	void rep_received(const rte_mbuf *);
	void rtu_received(const rte_mbuf *);
	void dreq_received(const rte_mbuf *);
	void drep_received(const rte_mbuf *);

	using handler = std::function<void(const rte_mbuf *)>;

	std::vector<handler> host_handlers;
	std::vector<handler> net_handlers;

	void process_packet(const std::vector<handler> &handlers, const rte_mbuf *p);

	qpn_map_t qpn_map;
};
