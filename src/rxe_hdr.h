/*
 * Copyright (c) 2016 Mellanox Technologies Ltd. All rights reserved.
 * Copyright (c) 2015 System Fabric Works, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *	- Redistributions of source code must retain the above
 *	  copyright notice, this list of conditions and the following
 *	  disclaimer.
 *
 *	- Redistributions in binary form must reproduce the above
 *	  copyright notice, this list of conditions and the following
 *	  disclaimer in the documentation and/or other materials
 *	  provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "linux-compat.h"
#include "rxe_opcode.h"
#include <arpa/inet.h>

#ifndef RXE_HDR_H
#define RXE_HDR_H

extern "C" {

/* extracted information about a packet carried in an sk_buff struct fits in
 * the skbuff cb array. Must be at most 48 bytes. stored in control block of
 * sk_buff for received packets.
 */
struct rxe_pkt_info {
	struct rxe_dev		*rxe;		/* device that owns packet */
	struct rxe_qp		*qp;		/* qp that owns packet */
	struct rxe_send_wqe	*wqe;		/* send wqe */
	u8			*hdr;		/* points to bth */
	u32			mask;		/* useful info about pkt */
	u32			psn;		/* bth psn of packet */
	u16			pkey_index;	/* partition of pkt */
	u16			paylen;		/* length of bth - icrc */
	u8			port_num;	/* port pkt received on */
	u8			opcode;		/* bth opcode of packet */
	u8			offset;		/* bth offset from pkt->hdr */
};

/*
 * IBA header types and methods
 *
 * Some of these are for reference and completeness only since
 * rxe does not currently support RD transport
 * most of this could be moved into IB core. ib_pack.h has
 * part of this but is incomplete
 *
 * Header specific routines to insert/extract values to/from headers
 * the routines that are named __hhh_(set_)fff() take a pointer to a
 * hhh header and get(set) the fff field. The routines named
 * hhh_(set_)fff take a packet info struct and find the
 * header and field based on the opcode in the packet.
 * Conversion to/from network byte order from cpu order is also done.
 */

#define RXE_ICRC_SIZE		(4)
#define RXE_MAX_HDR_LENGTH	(80)

/******************************************************************************
 * Base Transport Header
 ******************************************************************************/
struct rxe_bth {
	u8			opcode;
	u8			flags;
	__be16			pkey;
	__be32			qpn;
	__be32			apsn;
};

#define BTH_TVER		(0)
#define BTH_DEF_PKEY		(0xffff)

#define BTH_SE_MASK		((u8)0x80)
#define BTH_MIG_MASK		((u8)0x40)
#define BTH_PAD_MASK		((u8)0x30)
#define BTH_TVER_MASK		((u8)0x0f)
#define BTH_FECN_MASK		(0x80000000)
#define BTH_BECN_MASK		(0x40000000)
#define BTH_RESV6A_MASK		(0x3f000000)
#define BTH_QPN_MASK		(0x00ffffff)
#define BTH_ACK_MASK		(0x80000000)
#define BTH_RESV7_MASK		(0x7f000000)
#define BTH_PSN_MASK		(0x00ffffff)

static inline u8 __bth_opcode(void *arg)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	return bth->opcode;
}

static inline void __bth_set_opcode(void *arg, u8 opcode)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	bth->opcode = opcode;
}

static inline u8 __bth_se(void *arg)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	return 0 != (BTH_SE_MASK & bth->flags);
}

static inline void __bth_set_se(void *arg, int se)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	if (se)
		bth->flags |= BTH_SE_MASK;
	else
		bth->flags = (u8)(bth->flags & ~BTH_SE_MASK);
}

static inline u8 __bth_mig(void *arg)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	return 0 != (BTH_MIG_MASK & bth->flags);
}

static inline void __bth_set_mig(void *arg, u8 mig)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	if (mig)
		bth->flags |= BTH_MIG_MASK;
	else
		bth->flags = (u8)(bth->flags & ~BTH_MIG_MASK);
}

static inline u8 __bth_pad(void *arg)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	return (BTH_PAD_MASK & bth->flags) >> 4;
}

static inline void __bth_set_pad(void *arg, u8 pad)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	bth->flags = (u8)((BTH_PAD_MASK & (pad << (u8)(4))) |
			(~BTH_PAD_MASK & bth->flags));
}

static inline u8 __bth_tver(void *arg)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	return BTH_TVER_MASK & bth->flags;
}

static inline void __bth_set_tver(void *arg, u8 tver)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	bth->flags = (u8)((BTH_TVER_MASK & tver) |
			(~BTH_TVER_MASK & bth->flags));
}

static inline u16 __bth_pkey(void *arg)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	return ntohs(bth->pkey);
}

static inline void __bth_set_pkey(void *arg, u16 pkey)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	bth->pkey = htons(pkey);
}

static inline u32 __bth_qpn(void *arg)
{
	struct rxe_bth *bth = (rxe_bth *)arg;

	return BTH_QPN_MASK & ntohl(bth->qpn);
}

static inline void __bth_set_qpn(void *arg, u32 qpn)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;
	u32 resvqpn = ntohl(bth->qpn);

	bth->qpn = htonl((BTH_QPN_MASK & qpn) |
			       (~BTH_QPN_MASK & resvqpn));
}

static inline int __bth_fecn(void *arg)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	return 0 != (htonl(BTH_FECN_MASK) & bth->qpn);
}

static inline void __bth_set_fecn(void *arg, int fecn)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	if (fecn)
		bth->qpn |= htonl(BTH_FECN_MASK);
	else
		bth->qpn &= ~htonl(BTH_FECN_MASK);
}

static inline int __bth_becn(void *arg)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	return 0 != (htonl(BTH_BECN_MASK) & bth->qpn);
}

static inline void __bth_set_becn(void *arg, int becn)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	if (becn)
		bth->qpn |= htonl(BTH_BECN_MASK);
	else
		bth->qpn &= ~htonl(BTH_BECN_MASK);
}

static inline u8 __bth_resv6a(void *arg)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	return (BTH_RESV6A_MASK & ntohl(bth->qpn)) >> 24;
}

static inline void __bth_set_resv6a(void *arg)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	bth->qpn = htonl(~BTH_RESV6A_MASK);
}

static inline int __bth_ack(void *arg)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	return 0 != (htonl(BTH_ACK_MASK) & bth->apsn);
}

static inline void __bth_set_ack(void *arg, int ack)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	if (ack)
		bth->apsn |= htonl(BTH_ACK_MASK);
	else
		bth->apsn &= ~htonl(BTH_ACK_MASK);
}

static inline void __bth_set_resv7(void *arg)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	bth->apsn &= ~htonl(BTH_RESV7_MASK);
}

static inline u32 __bth_psn(void *arg)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;

	return BTH_PSN_MASK & ntohl(bth->apsn);
}

static inline void __bth_set_psn(void *arg, u32 psn)
{
	struct rxe_bth *bth = (struct rxe_bth *)arg;
	u32 apsn = ntohl(bth->apsn);

	bth->apsn = htonl((BTH_PSN_MASK & psn) |
			(~BTH_PSN_MASK & apsn));
}

static inline u8 bth_opcode(struct rxe_pkt_info *pkt)
{
	return __bth_opcode(pkt->hdr + pkt->offset);
}

static inline void bth_set_opcode(struct rxe_pkt_info *pkt, u8 opcode)
{
	__bth_set_opcode(pkt->hdr + pkt->offset, opcode);
}

static inline u8 bth_se(struct rxe_pkt_info *pkt)
{
	return __bth_se(pkt->hdr + pkt->offset);
}

static inline void bth_set_se(struct rxe_pkt_info *pkt, int se)
{
	__bth_set_se(pkt->hdr + pkt->offset, se);
}

static inline u8 bth_mig(struct rxe_pkt_info *pkt)
{
	return __bth_mig(pkt->hdr + pkt->offset);
}

static inline void bth_set_mig(struct rxe_pkt_info *pkt, u8 mig)
{
	__bth_set_mig(pkt->hdr + pkt->offset, mig);
}

static inline u8 bth_pad(struct rxe_pkt_info *pkt)
{
	return __bth_pad(pkt->hdr + pkt->offset);
}

static inline void bth_set_pad(struct rxe_pkt_info *pkt, u8 pad)
{
	__bth_set_pad(pkt->hdr + pkt->offset, pad);
}

static inline u8 bth_tver(struct rxe_pkt_info *pkt)
{
	return __bth_tver(pkt->hdr + pkt->offset);
}

static inline void bth_set_tver(struct rxe_pkt_info *pkt, u8 tver)
{
	__bth_set_tver(pkt->hdr + pkt->offset, tver);
}

static inline u16 bth_pkey(struct rxe_pkt_info *pkt)
{
	return __bth_pkey(pkt->hdr + pkt->offset);
}

static inline void bth_set_pkey(struct rxe_pkt_info *pkt, u16 pkey)
{
	__bth_set_pkey(pkt->hdr + pkt->offset, pkey);
}

static inline u32 bth_qpn(struct rxe_pkt_info *pkt)
{
	return __bth_qpn(pkt->hdr + pkt->offset);
}

static inline void bth_set_qpn(struct rxe_pkt_info *pkt, u32 qpn)
{
	__bth_set_qpn(pkt->hdr + pkt->offset, qpn);
}

static inline int bth_fecn(struct rxe_pkt_info *pkt)
{
	return __bth_fecn(pkt->hdr + pkt->offset);
}

static inline void bth_set_fecn(struct rxe_pkt_info *pkt, int fecn)
{
	__bth_set_fecn(pkt->hdr + pkt->offset, fecn);
}

static inline int bth_becn(struct rxe_pkt_info *pkt)
{
	return __bth_becn(pkt->hdr + pkt->offset);
}

static inline void bth_set_becn(struct rxe_pkt_info *pkt, int becn)
{
	__bth_set_becn(pkt->hdr + pkt->offset, becn);
}

static inline u8 bth_resv6a(struct rxe_pkt_info *pkt)
{
	return __bth_resv6a(pkt->hdr + pkt->offset);
}

static inline void bth_set_resv6a(struct rxe_pkt_info *pkt)
{
	__bth_set_resv6a(pkt->hdr + pkt->offset);
}

static inline int bth_ack(struct rxe_pkt_info *pkt)
{
	return __bth_ack(pkt->hdr + pkt->offset);
}

static inline void bth_set_ack(struct rxe_pkt_info *pkt, int ack)
{
	__bth_set_ack(pkt->hdr + pkt->offset, ack);
}

static inline void bth_set_resv7(struct rxe_pkt_info *pkt)
{
	__bth_set_resv7(pkt->hdr + pkt->offset);
}

static inline u32 bth_psn(struct rxe_pkt_info *pkt)
{
	return __bth_psn(pkt->hdr + pkt->offset);
}

static inline void bth_set_psn(struct rxe_pkt_info *pkt, u32 psn)
{
	__bth_set_psn(pkt->hdr + pkt->offset, psn);
}

static inline void bth_init(struct rxe_pkt_info *pkt, u8 opcode, int se,
			    int mig, int pad, u16 pkey, u32 qpn, int ack_req,
			    u32 psn)
{
	struct rxe_bth *bth = (struct rxe_bth *)(pkt->hdr + pkt->offset);

	bth->opcode = opcode;
	bth->flags = (pad << 4) & BTH_PAD_MASK;
	if (se)
		bth->flags |= BTH_SE_MASK;
	if (mig)
		bth->flags |= BTH_MIG_MASK;
	bth->pkey = htons(pkey);
	bth->qpn = htonl(qpn & BTH_QPN_MASK);
	psn &= BTH_PSN_MASK;
	if (ack_req)
		psn |= BTH_ACK_MASK;
	bth->apsn = htonl(psn);
}

/******************************************************************************
 * Reliable Datagram Extended Transport Header
 ******************************************************************************/
struct rxe_rdeth {
	__be32			een;
};

#define RDETH_EEN_MASK		(0x00ffffff)

static inline u32 __rdeth_een(void *arg)
{
	struct rxe_rdeth *rdeth = (struct rxe_rdeth *)arg;

	return RDETH_EEN_MASK & ntohl(rdeth->een);
}

static inline void __rdeth_set_een(void *arg, u32 een)
{
	struct rxe_rdeth *rdeth = (struct rxe_rdeth *)arg;

	rdeth->een = htonl(RDETH_EEN_MASK & een);
}

static inline u32 rdeth_een(struct rxe_pkt_info *pkt)
{
	return __rdeth_een(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RDETH]);
}

static inline void rdeth_set_een(struct rxe_pkt_info *pkt, u32 een)
{
	__rdeth_set_een(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RDETH], een);
}

/******************************************************************************
 * Datagram Extended Transport Header
 ******************************************************************************/
struct rxe_deth {
	__be32			qkey;
	__be32			sqp;
};

#define GSI_QKEY		(0x80010000)
#define DETH_SQP_MASK		(0x00ffffff)

static inline u32 __deth_qkey(void *arg)
{
	struct rxe_deth *deth = (struct rxe_deth *)arg;

	return ntohl(deth->qkey);
}

static inline void __deth_set_qkey(void *arg, u32 qkey)
{
	struct rxe_deth *deth = (struct rxe_deth *)arg;

	deth->qkey = htonl(qkey);
}

static inline u32 __deth_sqp(void *arg)
{
	struct rxe_deth *deth = (struct rxe_deth *)arg;

	return DETH_SQP_MASK & ntohl(deth->sqp);
}

static inline void __deth_set_sqp(void *arg, u32 sqp)
{
	struct rxe_deth *deth = (struct rxe_deth *)arg;

	deth->sqp = htonl(DETH_SQP_MASK & sqp);
}

static inline u32 deth_qkey(struct rxe_pkt_info *pkt)
{
	return __deth_qkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_DETH]);
}

static inline void deth_set_qkey(struct rxe_pkt_info *pkt, u32 qkey)
{
	__deth_set_qkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_DETH], qkey);
}

static inline u32 deth_sqp(struct rxe_pkt_info *pkt)
{
	return __deth_sqp(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_DETH]);
}

static inline void deth_set_sqp(struct rxe_pkt_info *pkt, u32 sqp)
{
	__deth_set_sqp(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_DETH], sqp);
}

/******************************************************************************
 * RDMA Extended Transport Header
 ******************************************************************************/
struct rxe_reth {
	__be64			va;
	__be32			rkey;
	__be32			len;
};

static inline u64 __reth_va(void *arg)
{
	struct rxe_reth *reth = (struct rxe_reth *)arg;

	return be64toh(reth->va);
}

static inline void __reth_set_va(void *arg, u64 va)
{
	struct rxe_reth *reth = (struct rxe_reth *)arg;

	reth->va = htobe64(va);
}

static inline u32 __reth_rkey(void *arg)
{
	struct rxe_reth *reth = (struct rxe_reth *)arg;

	return ntohl(reth->rkey);
}

static inline void __reth_set_rkey(void *arg, u32 rkey)
{
	struct rxe_reth *reth = (struct rxe_reth *)arg;

	reth->rkey = htonl(rkey);
}

static inline u32 __reth_len(void *arg)
{
	struct rxe_reth *reth = (struct rxe_reth *)arg;

	return ntohl(reth->len);
}

static inline void __reth_set_len(void *arg, u32 len)
{
	struct rxe_reth *reth = (struct rxe_reth *)arg;

	reth->len = htonl(len);
}

static inline u64 reth_va(struct rxe_pkt_info *pkt)
{
	return __reth_va(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RETH]);
}

static inline void reth_set_va(struct rxe_pkt_info *pkt, u64 va)
{
	__reth_set_va(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RETH], va);
}

static inline u32 reth_rkey(struct rxe_pkt_info *pkt)
{
	return __reth_rkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RETH]);
}

static inline void reth_set_rkey(struct rxe_pkt_info *pkt, u32 rkey)
{
	__reth_set_rkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RETH], rkey);
}

static inline u32 reth_len(struct rxe_pkt_info *pkt)
{
	return __reth_len(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RETH]);
}

static inline void reth_set_len(struct rxe_pkt_info *pkt, u32 len)
{
	__reth_set_len(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_RETH], len);
}

/******************************************************************************
 * Atomic Extended Transport Header
 ******************************************************************************/
struct rxe_atmeth {
	__be64			va;
	__be32			rkey;
	__be64			swap_add;
	__be64			comp;
} __packed;

static inline u64 __atmeth_va(void *arg)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	return be64toh(atmeth->va);
}

static inline void __atmeth_set_va(void *arg, u64 va)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	atmeth->va = htobe64(va);
}

static inline u32 __atmeth_rkey(void *arg)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	return ntohl(atmeth->rkey);
}

static inline void __atmeth_set_rkey(void *arg, u32 rkey)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	atmeth->rkey = htonl(rkey);
}

static inline u64 __atmeth_swap_add(void *arg)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	return be64toh(atmeth->swap_add);
}

static inline void __atmeth_set_swap_add(void *arg, u64 swap_add)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	atmeth->swap_add = htobe64(swap_add);
}

static inline u64 __atmeth_comp(void *arg)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	return be64toh(atmeth->comp);
}

static inline void __atmeth_set_comp(void *arg, u64 comp)
{
	struct rxe_atmeth *atmeth = (struct rxe_atmeth *)arg;

	atmeth->comp = htobe64(comp);
}

static inline u64 atmeth_va(struct rxe_pkt_info *pkt)
{
	return __atmeth_va(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH]);
}

static inline void atmeth_set_va(struct rxe_pkt_info *pkt, u64 va)
{
	__atmeth_set_va(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH], va);
}

static inline u32 atmeth_rkey(struct rxe_pkt_info *pkt)
{
	return __atmeth_rkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH]);
}

static inline void atmeth_set_rkey(struct rxe_pkt_info *pkt, u32 rkey)
{
	__atmeth_set_rkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH], rkey);
}

static inline u64 atmeth_swap_add(struct rxe_pkt_info *pkt)
{
	return __atmeth_swap_add(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH]);
}

static inline void atmeth_set_swap_add(struct rxe_pkt_info *pkt, u64 swap_add)
{
	__atmeth_set_swap_add(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH], swap_add);
}

static inline u64 atmeth_comp(struct rxe_pkt_info *pkt)
{
	return __atmeth_comp(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH]);
}

static inline void atmeth_set_comp(struct rxe_pkt_info *pkt, u64 comp)
{
	__atmeth_set_comp(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMETH], comp);
}

/******************************************************************************
 * Ack Extended Transport Header
 ******************************************************************************/
struct rxe_aeth {
	__be32			smsn;
};

#define AETH_SYN_MASK		(0xff000000)
#define AETH_MSN_MASK		(0x00ffffff)

enum aeth_syndrome {
	AETH_TYPE_MASK		= 0xe0,
	AETH_ACK		= 0x00,
	AETH_RNR_NAK		= 0x20,
	AETH_RSVD		= 0x40,
	AETH_NAK		= 0x60,
	AETH_ACK_UNLIMITED	= 0x1f,
	AETH_NAK_PSN_SEQ_ERROR	= 0x60,
	AETH_NAK_INVALID_REQ	= 0x61,
	AETH_NAK_REM_ACC_ERR	= 0x62,
	AETH_NAK_REM_OP_ERR	= 0x63,
	AETH_NAK_INV_RD_REQ	= 0x64,
};

static inline u8 __aeth_syn(void *arg)
{
	struct rxe_aeth *aeth = (struct rxe_aeth *)arg;

	return (u8)((AETH_SYN_MASK & ntohl(aeth->smsn)) >> 24);
}

static inline void __aeth_set_syn(void *arg, u8 syn)
{
	struct rxe_aeth *aeth = (struct rxe_aeth *)arg;
	u32 smsn = ntohl(aeth->smsn);

	aeth->smsn = htonl((AETH_SYN_MASK & (syn << 24)) |
			 (~AETH_SYN_MASK & smsn));
}

static inline u32 __aeth_msn(void *arg)
{
	struct rxe_aeth *aeth = (struct rxe_aeth *)arg;

	return AETH_MSN_MASK & ntohl(aeth->smsn);
}

static inline void __aeth_set_msn(void *arg, u32 msn)
{
	struct rxe_aeth *aeth = (struct rxe_aeth *)arg;
	u32 smsn = ntohl(aeth->smsn);

	aeth->smsn = htonl((AETH_MSN_MASK & msn) |
			 (~AETH_MSN_MASK & smsn));
}

static inline u8 aeth_syn(struct rxe_pkt_info *pkt)
{
	return __aeth_syn(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_AETH]);
}

static inline void aeth_set_syn(struct rxe_pkt_info *pkt, u8 syn)
{
	__aeth_set_syn(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_AETH], syn);
}

static inline u32 aeth_msn(struct rxe_pkt_info *pkt)
{
	return __aeth_msn(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_AETH]);
}

static inline void aeth_set_msn(struct rxe_pkt_info *pkt, u32 msn)
{
	__aeth_set_msn(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_AETH], msn);
}

/******************************************************************************
 * Atomic Ack Extended Transport Header
 ******************************************************************************/
struct rxe_atmack {
	__be64			orig;
};

static inline u64 __atmack_orig(void *arg)
{
	struct rxe_atmack *atmack = (struct rxe_atmack *)arg;

	return be64toh(atmack->orig);
}

static inline void __atmack_set_orig(void *arg, u64 orig)
{
	struct rxe_atmack *atmack = (struct rxe_atmack *)arg;

	atmack->orig = htobe64(orig);
}

static inline u64 atmack_orig(struct rxe_pkt_info *pkt)
{
	return __atmack_orig(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMACK]);
}

static inline void atmack_set_orig(struct rxe_pkt_info *pkt, u64 orig)
{
	__atmack_set_orig(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_ATMACK], orig);
}

/******************************************************************************
 * Immediate Extended Transport Header
 ******************************************************************************/
struct rxe_immdt {
	__be32			imm;
};

static inline __be32 __immdt_imm(void *arg)
{
	struct rxe_immdt *immdt = (struct rxe_immdt *)arg;

	return immdt->imm;
}

static inline void __immdt_set_imm(void *arg, __be32 imm)
{
	struct rxe_immdt *immdt = (struct rxe_immdt *)arg;

	immdt->imm = imm;
}

static inline __be32 immdt_imm(struct rxe_pkt_info *pkt)
{
	return __immdt_imm(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_IMMDT]);
}

static inline void immdt_set_imm(struct rxe_pkt_info *pkt, __be32 imm)
{
	__immdt_set_imm(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_IMMDT], imm);
}

/******************************************************************************
 * Invalidate Extended Transport Header
 ******************************************************************************/
struct rxe_ieth {
	__be32			rkey;
};

static inline u32 __ieth_rkey(void *arg)
{
	struct rxe_ieth *ieth = (struct rxe_ieth *)arg;

	return ntohl(ieth->rkey);
}

static inline void __ieth_set_rkey(void *arg, u32 rkey)
{
	struct rxe_ieth *ieth = (struct rxe_ieth *)arg;

	ieth->rkey = htonl(rkey);
}

static inline u32 ieth_rkey(struct rxe_pkt_info *pkt)
{
	return __ieth_rkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_IETH]);
}

static inline void ieth_set_rkey(struct rxe_pkt_info *pkt, u32 rkey)
{
	__ieth_set_rkey(pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_IETH], rkey);
}

enum rxe_hdr_length {
	RXE_BTH_BYTES		= sizeof(struct rxe_bth),
	RXE_DETH_BYTES		= sizeof(struct rxe_deth),
	RXE_IMMDT_BYTES		= sizeof(struct rxe_immdt),
	RXE_RETH_BYTES		= sizeof(struct rxe_reth),
	RXE_AETH_BYTES		= sizeof(struct rxe_aeth),
	RXE_ATMACK_BYTES	= sizeof(struct rxe_atmack),
	RXE_ATMETH_BYTES	= sizeof(struct rxe_atmeth),
	RXE_IETH_BYTES		= sizeof(struct rxe_ieth),
	RXE_RDETH_BYTES		= sizeof(struct rxe_rdeth),
};

static inline size_t header_size(struct rxe_pkt_info *pkt)
{
	return pkt->offset + rxe_opcode[pkt->opcode].length;
}

static inline void *payload_addr(struct rxe_pkt_info *pkt)
{
	return pkt->hdr + pkt->offset
		+ rxe_opcode[pkt->opcode].offset[RXE_PAYLOAD];
}

static inline size_t payload_size(struct rxe_pkt_info *pkt)
{
	return pkt->paylen - rxe_opcode[pkt->opcode].offset[RXE_PAYLOAD]
		- bth_pad(pkt) - RXE_ICRC_SIZE;
}

}

#endif /* RXE_HDR_H */
