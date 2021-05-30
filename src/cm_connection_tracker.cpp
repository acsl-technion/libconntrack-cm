/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright 2021 Haggai Eran
 */

#include "cm_connection_tracker.h"
#include "logging.h"

#include "ibta_vol1_c12.h"

#include <boost/current_function.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char *flow_state::state_names[] = {
#define _(r, data, elem) BOOST_PP_STRINGIZE(elem) BOOST_PP_COMMA()
	BOOST_PP_SEQ_FOR_EACH(_, , FLOW_STATES)
#undef _
};

cm_connection_tracker::cm_connection_tracker(parser_context& parser) :
    parser(parser)
{
	host_handlers.resize(CM_MAX_ATTR_ID);
	host_handlers[CM_REQ_ATTR_ID] = std::bind(&cm_connection_tracker::req_sent, this, std::placeholders::_1);
	host_handlers[CM_REJ_ATTR_ID] = std::bind(&cm_connection_tracker::rej_sent, this, std::placeholders::_1);
	host_handlers[CM_REP_ATTR_ID] = std::bind(&cm_connection_tracker::rep_sent, this, std::placeholders::_1);
	host_handlers[CM_RTU_ATTR_ID] = std::bind(&cm_connection_tracker::rtu_sent, this, std::placeholders::_1);
	host_handlers[CM_DREQ_ATTR_ID] = std::bind(&cm_connection_tracker::dreq_sent, this, std::placeholders::_1);
	host_handlers[CM_DREP_ATTR_ID] = std::bind(&cm_connection_tracker::drep_sent, this, std::placeholders::_1);

	net_handlers.resize(CM_MAX_ATTR_ID);
	net_handlers[CM_REQ_ATTR_ID] = std::bind(&cm_connection_tracker::req_received, this, std::placeholders::_1);
	net_handlers[CM_REJ_ATTR_ID] = std::bind(&cm_connection_tracker::rej_received, this, std::placeholders::_1);
	net_handlers[CM_REP_ATTR_ID] = std::bind(&cm_connection_tracker::rep_received, this, std::placeholders::_1);
	net_handlers[CM_RTU_ATTR_ID] = std::bind(&cm_connection_tracker::rtu_received, this, std::placeholders::_1);
	net_handlers[CM_DREQ_ATTR_ID] = std::bind(&cm_connection_tracker::dreq_received, this, std::placeholders::_1);
	net_handlers[CM_DREP_ATTR_ID] = std::bind(&cm_connection_tracker::drep_received, this, std::placeholders::_1);
}

flow_state_ptr cm_connection_tracker::get_flow(id_t local_id, cm_flow_key remote_id)
{
	flow_state_ptr state;

	assert(local_id || remote_id);

	if (local_id) {
		auto it = local_map.find(local_id);

		if (it != local_map.end())
			state = it->second;
	} else {
		auto it = remote_map.find(remote_id);

		if (it != remote_map.end())
			state = it->second;
	}

	if (!state)
		return add_new_flow(local_id, remote_id);
	else
		return state;
}

void cm_connection_tracker::erase_flow(id_t local_id, cm_flow_key remote_id)
{
	assert(local_id || remote_id);

	auto f = [this, local_id, remote_id](auto &map1, auto &map2, auto id, auto id_other) {
		auto it = map1.find(id);
		if (it == map1.end()) {
			log_debug("Unknown ID erased: 0x%x, 0x%x\n", local_id, remote_id.id());
		} else {
			auto state = it->second;
			if (state->in_qpn_map)
				on_disconnected(state);
			map1.erase(it);
			map2.erase(id_other);
		}
	};

	if (local_id)
		f(local_map, remote_map, local_id, remote_id);
	else
		f(remote_map, local_map, remote_id, local_id);
}

flow_state_ptr cm_connection_tracker::add_new_flow(id_t local_id, cm_flow_key remote_id)
{
	log_debug("add_new_flow(0x%x, 0x%x)\n", local_id, remote_id.id());

	auto local_it = local_map.find(local_id);
	auto remote_it = remote_map.find(remote_id);

	flow_state_ptr state;

	if (local_it != local_map.end() && remote_it != remote_map.end()) {
		assert(local_it->second == remote_it->second);
		state = local_it->second;
		assert(state->local_id == local_id && state->remote_id == remote_id);
		log_debug("%s", "Warning: adding an already existing entry\n");
		return state;
	} else if (local_it == local_map.end() && remote_it == remote_map.end()) {
		assert(local_id || remote_id);
		state = std::make_shared<flow_state>();
		log_debug("%s", "New flow_state{}\n");
	} else if (local_it != local_map.end()) {
		state = local_it->second;
		log_debug("flow_state in local: local_id 0x%x, remote_id 0x%x\n",
			state->local_id, state->remote_id.id());
	} else if (remote_it != remote_map.end()) {
		state = remote_it->second;
		log_debug("flow_state in remote: local_id 0x%x, remote_id 0x%x\n",
			state->local_id, state->remote_id.id());
	}

	assert(state);

	if (local_it == local_map.end() && local_id) {
		assert(state->local_id == 0);
		state->local_id = local_id;
		local_map[local_id] = state;
	}

	if (remote_it == remote_map.end() && remote_id) {
		assert(!state->remote_id);
		state->remote_id = remote_id;
		remote_map[remote_id] = state;
	}

	return state;
}

void flow_state::log(const char *func, const char *msg)
{
	log_debug("%s:%s state: %s, IDs: (0x%x, 0x%x), QPNs: (0x%x, 0x%x)\n",
		func, msg, state_names[state], local_id, remote_id.id(),
		local_qpn, remote_qpn);
}

uint16_t get_attr_id(parser_context& parser, const rte_mbuf *p)
{
	return be16toh(parser.mbuf_mad(p)->attr_id);
}

void cm_connection_tracker::req_sent(const rte_mbuf *p)
{
	auto msg = (cm_req_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_REQ_LOCAL_COMM_ID, msg);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::IDLE:
	case flow_state::REQ_SENT:
		state->state = flow_state::REQ_SENT;
		state->local_qpn = IBA_GET(CM_REQ_LOCAL_QPN, msg);
		state->log(BOOST_CURRENT_FUNCTION);
		break;
	default:
		log_debug("CM req -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::req_received(const rte_mbuf *p)
{
	auto msg = (cm_req_msg *)parser.mbuf_mad(p);
	id_t remote_id = IBA_GET(CM_REQ_LOCAL_COMM_ID, msg);
	auto state = get_flow(0, cm_flow_key::from_src(p, remote_id));
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::REQ_RCVD:
		assert(state->remote_qpn == IBA_GET(CM_REQ_LOCAL_QPN, msg));
		/* Fallthrough */
	case flow_state::IDLE:
		state->state = flow_state::REQ_RCVD;
		state->remote_qpn = IBA_GET(CM_REQ_LOCAL_QPN, msg);
		state->log(BOOST_CURRENT_FUNCTION);
		break;
	default:
		log_debug("CM req received -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::rej_sent(const rte_mbuf *p)
{
	auto msg = (cm_rej_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_REJ_LOCAL_COMM_ID, msg);
	id_t remote_id = IBA_GET(CM_REJ_REMOTE_COMM_ID, msg);
	auto remote_flow = cm_flow_key::from_dest(p, remote_id);
	auto state = get_flow(local_id, remote_flow);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::IDLE:
		log_debug("CM[0x%x] rej: warning IDLE state or not found.\n",
			local_id);
		/* Fallthrough */

	case flow_state::REQ_RCVD:
	case flow_state::REQ_SENT:
	case flow_state::MRA_REQ_RCVD:
	case flow_state::MRA_REQ_SENT:
	case flow_state::REP_RCVD:
	case flow_state::MRA_REP_SENT:
		erase_flow(local_id, remote_flow);
		log_debug("CM rej -> IDLE. local ID = 0x%x\n", local_id);
		break;

	case flow_state::REP_SENT:
	case flow_state::MRA_REP_RCVD:
		// TODO timewait
		erase_flow(local_id, remote_flow);
		log_debug("CM rej -> IDLE. local ID = 0x%x\n", local_id);
		break;

	default:
		log_debug("CM rej -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::rej_received(const rte_mbuf *p)
{
	auto msg = (cm_rej_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_REJ_REMOTE_COMM_ID, msg);
	id_t remote_id = IBA_GET(CM_REJ_LOCAL_COMM_ID, msg);
	auto remote_flow = cm_flow_key::from_src(p, remote_id);
	auto state = get_flow(local_id, remote_flow);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::IDLE:
		log_debug("CM[0x%x] rej received: warning IDLE state or not found.\n",
			local_id);
		/* Fallthrough */

	case flow_state::REQ_SENT:
	case flow_state::MRA_REQ_RCVD:
	case flow_state::REP_SENT:
	case flow_state::MRA_REP_RCVD:
		// fallthrough
	case flow_state::MRA_REQ_SENT:
		// TODO timewait if stale
		erase_flow(local_id, remote_flow);
		log_debug("CM rej received -> IDLE. local ID = 0x%x\n", local_id);
		break;

	case flow_state::DREQ_SENT:
	case flow_state::REP_RCVD:
	case flow_state::MRA_REP_SENT:
	case flow_state::ESTABLISHED:
		// TODO timewait
		erase_flow(local_id, remote_flow);
		log_debug("CM rej received -> IDLE. local ID = 0x%x\n", local_id);
		break;

	default:
		log_debug("CM rej received -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::rep_sent(const rte_mbuf *p)
{
	auto msg = (cm_rep_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_REP_LOCAL_COMM_ID, msg);
	id_t remote_id = IBA_GET(CM_REP_REMOTE_COMM_ID, msg);
	auto remote_flow = cm_flow_key::from_dest(p, remote_id);
	auto state = get_flow(local_id, remote_flow);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::REQ_RCVD:
	case flow_state::MRA_REQ_SENT:
		state->state = flow_state::REP_SENT;
		add_new_flow(local_id, remote_flow);
		state->local_qpn = IBA_GET(CM_REP_LOCAL_QPN, msg);
		log_debug("CM rep -> REP_SENT. local ID = 0x%x, qpn = 0x%x\n",
			local_id, state->local_qpn);
		break;
	case flow_state::REP_SENT:
		log_debug("CM rep -> REP_SENT duplicate. local ID = 0x%x, qpn = 0x%x\n",
			local_id, state->local_qpn);
		break;
	default:
		log_debug("CM rep -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::rep_received(const rte_mbuf *p)
{
	auto msg = (cm_rep_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_REP_REMOTE_COMM_ID, msg);
	id_t remote_id = IBA_GET(CM_REP_LOCAL_COMM_ID, msg);
	auto remote_flow = cm_flow_key::from_src(p, remote_id);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::REQ_SENT:
	case flow_state::MRA_REQ_RCVD:
		state->state = flow_state::REP_RCVD;
		state->remote_qpn = IBA_GET(CM_REP_LOCAL_QPN, msg);
		add_new_flow(local_id, remote_flow);
		log_debug("CM rep received -> REP_SENT. local ID = 0x%x, remote ID = 0x%x, local qpn = 0x%x remote qpn = 0x%x\n",
			local_id, remote_id, state->local_qpn, state->remote_qpn);
		break;
	default:
		log_debug("CM rep received -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::rtu_sent(const rte_mbuf *p)
{
	auto msg = (cm_rtu_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_RTU_LOCAL_COMM_ID, msg);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::REP_RCVD:
	case flow_state::MRA_REP_SENT:
		state->state = flow_state::ESTABLISHED;
		on_established(state);
		log_debug("CM rtu -> ESTABLISHED. local ID = 0x%x, qpn = 0x%x\n",
			local_id, state->local_qpn);
		break;
	default:
		log_debug("CM rtu -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::rtu_received(const rte_mbuf *p)
{
	auto msg = (cm_rtu_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_RTU_REMOTE_COMM_ID, msg);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::REP_SENT:
	case flow_state::MRA_REP_RCVD:
		state->state = flow_state::ESTABLISHED;
		on_established(state);
		log_debug("CM rtu received -> ESTABLISHED. local ID = 0x%x, qpn = 0x%x\n",
			local_id, state->local_qpn);
		break;
	default:
		log_debug("CM rtu received -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::dreq_sent(const rte_mbuf *p)
{
	auto msg = (cm_dreq_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_DREQ_LOCAL_COMM_ID, msg);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::IDLE:
		log_debug("CM[%d] dreq (IDLE): maybe we missed some states?\n",
			local_id);
		/* Fallthrough */
	case flow_state::ESTABLISHED:
	case flow_state::DREQ_SENT:
	case flow_state::DREQ_RCVD:
		state->state = flow_state::DREQ_SENT;
		log_debug("CM dreq -> DREQ_SENT. local ID = 0x%x\n",
			local_id);
		break;
	default:
		log_debug("CM dreq -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::dreq_received(const rte_mbuf *p)
{
	auto msg = (cm_dreq_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_DREQ_REMOTE_COMM_ID, msg);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::REP_SENT:
	case flow_state::DREQ_SENT:
	case flow_state::ESTABLISHED:
	case flow_state::MRA_REP_RCVD:
	case flow_state::TIMEWAIT:
	case flow_state::DREQ_RCVD:
		state->state = flow_state::DREQ_RCVD;
		log_debug("CM dreq received -> DREQ_RCVD. local ID = 0x%x\n",
			local_id);
		break;
	default:
		log_debug("CM dreq received -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::drep_sent(const rte_mbuf *p)
{
	auto msg = (cm_drep_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_DREP_LOCAL_COMM_ID, msg);
	id_t remote_id = IBA_GET(CM_DREP_REMOTE_COMM_ID, msg);
	auto remote_flow = cm_flow_key::from_dest(p, remote_id);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
        case flow_state::DREQ_SENT:
	case flow_state::DREQ_RCVD:
		// TODO timewait?
		erase_flow(local_id, remote_flow);
		log_debug("CM drep -> IDLE. local ID = 0x%x\n",
			local_id);
		break;
	default:
		log_debug("CM drep -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::drep_received(const rte_mbuf *p)
{
	auto msg = (cm_drep_msg *)parser.mbuf_mad(p);
	id_t local_id = IBA_GET(CM_DREP_REMOTE_COMM_ID, msg);
	id_t remote_id = IBA_GET(CM_DREP_LOCAL_COMM_ID, msg);
	auto remote_flow = cm_flow_key::from_src(p, remote_id);
	auto state = get_flow(local_id);
	state->log(BOOST_CURRENT_FUNCTION);
	switch (state->state) {
	case flow_state::DREQ_SENT:
	case flow_state::DREQ_RCVD:
		// TODO timewait?
		erase_flow(local_id, remote_flow);
		log_debug("CM drep received -> IDLE. local ID = 0x%x\n",
			local_id);
		break;
	default:
		log_debug("CM drep received -> unexpected state: 0x%x\n", state->state);
	}
}

void cm_connection_tracker::process_packet(const std::vector<handler> &handlers, const rte_mbuf *p)
{
	uint16_t attr_id = get_attr_id(parser, p);
	handler h;
	if (attr_id < CM_MAX_ATTR_ID) {
		h = handlers[attr_id];
	}
	if (h) {
		h(p);
	} else {
		log_debug("Unknown attr_id received in %s: 0x%x\n",
			BOOST_CURRENT_FUNCTION, attr_id);
		auto hdr = parser.mbuf_mad(p);
		log_debug("RoCE MAD packet: \n"
			"base_version: 0x%x, "
			"mgmt_class: 0x%x, "
			"class_version: 0x%x, "
			"method: 0x%x\n", hdr->base_version, hdr->mgmt_class,
				hdr->class_version, hdr->method);
	}
}

void cm_connection_tracker::process(const rte_mbuf *p, enum ctcm_direction dir)
{
	switch (dir) {
	case CTCM_FROM_HOST:
		process_packet(host_handlers, p);
		break;
	case CTCM_FROM_NET:
		process_packet(net_handlers, p);
		break;
	}
}

void cm_connection_tracker::on_established(flow_state_ptr state)
{
	if (!state->local_qpn || !state->remote_qpn) {
		log_debug("Bad QPNs local: 0x%x remote: 0x%x\n", state->local_qpn,
			state->remote_qpn);
		return;
	}

	auto [it, inserted] = qpn_map.insert(state->flow_value());
	(void) it;
	if (!inserted) {
		log_debug("QP already in table: 0x%x\n", state->local_qpn);
		return;
	}

	state->in_qpn_map = true;

	in_addr remote_ip = in_addr{std::get<0>(state->remote_id)};
	log_debug("Established: local: 0x%x, remote: %s:0x%x\n", state->local_qpn,
		inet_ntoa(remote_ip), state->remote_qpn);
}

void cm_connection_tracker::on_disconnected(flow_state_ptr state)
{
	if (!state->local_qpn || !state->remote_qpn) {
		log_debug("Bad QPNs local: 0x%x remote: 0x%x\n", state->local_qpn,
			state->remote_qpn);
		return;
	}

	auto erased = qpn_map.erase(state->get_flow_key());

	if (!erased) {
		log_debug("QP was not in table: 0x%x\n", state->local_qpn);
		return;
	}

	state->in_qpn_map = false;

	log_debug("Disconnected: local: 0x%x, remote: 0x%x\n", state->local_qpn,
		state->remote_qpn);
}
