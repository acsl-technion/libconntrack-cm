/*
 * Copyright (c) 2004 Mellanox Technologies Ltd.  All rights reserved.
 * Copyright (c) 2004 Infinicon Corporation.  All rights reserved.
 * Copyright (c) 2004 Intel Corporation.  All rights reserved.
 * Copyright (c) 2004 Topspin Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 Voltaire Corporation.  All rights reserved.
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
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
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

#if !defined(IB_MAD_H)
#define IB_MAD_H

#include "linux-compat.h"

/* Management base versions */
#define IB_MGMT_BASE_VERSION			1
#define OPA_MGMT_BASE_VERSION			0x80

#define OPA_SM_CLASS_VERSION			0x80

/* Management classes */
#define IB_MGMT_CLASS_SUBN_LID_ROUTED		0x01
#define IB_MGMT_CLASS_SUBN_DIRECTED_ROUTE	0x81
#define IB_MGMT_CLASS_SUBN_ADM			0x03
#define IB_MGMT_CLASS_PERF_MGMT			0x04
#define IB_MGMT_CLASS_BM			0x05
#define IB_MGMT_CLASS_DEVICE_MGMT		0x06
#define IB_MGMT_CLASS_CM			0x07
#define IB_MGMT_CLASS_SNMP			0x08
#define IB_MGMT_CLASS_DEVICE_ADM		0x10
#define IB_MGMT_CLASS_BOOT_MGMT			0x11
#define IB_MGMT_CLASS_BIS			0x12
#define IB_MGMT_CLASS_CONG_MGMT			0x21
#define IB_MGMT_CLASS_VENDOR_RANGE2_START	0x30
#define IB_MGMT_CLASS_VENDOR_RANGE2_END		0x4F

#define	IB_OPENIB_OUI				(0x001405)

/* Management methods */
#define IB_MGMT_METHOD_GET			0x01
#define IB_MGMT_METHOD_SET			0x02
#define IB_MGMT_METHOD_GET_RESP			0x81
#define IB_MGMT_METHOD_SEND			0x03
#define IB_MGMT_METHOD_TRAP			0x05
#define IB_MGMT_METHOD_REPORT			0x06
#define IB_MGMT_METHOD_REPORT_RESP		0x86
#define IB_MGMT_METHOD_TRAP_REPRESS		0x07

#define IB_MGMT_METHOD_RESP			0x80
#define IB_BM_ATTR_MOD_RESP			cpu_to_be32(1)

#define IB_MGMT_MAX_METHODS			128

/* MAD Status field bit masks */
#define IB_MGMT_MAD_STATUS_SUCCESS			0x0000
#define IB_MGMT_MAD_STATUS_BUSY				0x0001
#define IB_MGMT_MAD_STATUS_REDIRECT_REQD		0x0002
#define IB_MGMT_MAD_STATUS_BAD_VERSION			0x0004
#define IB_MGMT_MAD_STATUS_UNSUPPORTED_METHOD		0x0008
#define IB_MGMT_MAD_STATUS_UNSUPPORTED_METHOD_ATTRIB	0x000c
#define IB_MGMT_MAD_STATUS_INVALID_ATTRIB_VALUE		0x001c

/* RMPP information */
#define IB_MGMT_RMPP_VERSION			1

#define IB_MGMT_RMPP_TYPE_DATA			1
#define IB_MGMT_RMPP_TYPE_ACK			2
#define IB_MGMT_RMPP_TYPE_STOP			3
#define IB_MGMT_RMPP_TYPE_ABORT			4

#define IB_MGMT_RMPP_FLAG_ACTIVE		1
#define IB_MGMT_RMPP_FLAG_FIRST			(1<<1)
#define IB_MGMT_RMPP_FLAG_LAST			(1<<2)

#define IB_MGMT_RMPP_NO_RESPTIME		0x1F

#define	IB_MGMT_RMPP_STATUS_SUCCESS		0
#define	IB_MGMT_RMPP_STATUS_RESX		1
#define	IB_MGMT_RMPP_STATUS_ABORT_MIN		118
#define	IB_MGMT_RMPP_STATUS_T2L			118
#define	IB_MGMT_RMPP_STATUS_BAD_LEN		119
#define	IB_MGMT_RMPP_STATUS_BAD_SEG		120
#define	IB_MGMT_RMPP_STATUS_BADT		121
#define	IB_MGMT_RMPP_STATUS_W2S			122
#define	IB_MGMT_RMPP_STATUS_S2B			123
#define	IB_MGMT_RMPP_STATUS_BAD_STATUS		124
#define	IB_MGMT_RMPP_STATUS_UNV			125
#define	IB_MGMT_RMPP_STATUS_TMR			126
#define	IB_MGMT_RMPP_STATUS_UNSPEC		127
#define	IB_MGMT_RMPP_STATUS_ABORT_MAX		127

#define IB_QP0		0
#define IB_QP1		cpu_to_be32(1)
#define IB_QP1_QKEY	0x80010000
#define IB_QP_SET_QKEY	0x80000000

#define IB_DEFAULT_PKEY_PARTIAL 0x7FFF
#define IB_DEFAULT_PKEY_FULL	0xFFFF

/*
 * Generic trap/notice types
 */
#define IB_NOTICE_TYPE_FATAL	0x80
#define IB_NOTICE_TYPE_URGENT	0x81
#define IB_NOTICE_TYPE_SECURITY	0x82
#define IB_NOTICE_TYPE_SM	0x83
#define IB_NOTICE_TYPE_INFO	0x84

/*
 * Generic trap/notice producers
 */
#define IB_NOTICE_PROD_CA		cpu_to_be16(1)
#define IB_NOTICE_PROD_SWITCH		cpu_to_be16(2)
#define IB_NOTICE_PROD_ROUTER		cpu_to_be16(3)
#define IB_NOTICE_PROD_CLASS_MGR	cpu_to_be16(4)

enum {
	IB_MGMT_MAD_HDR = 24,
	IB_MGMT_MAD_DATA = 232,
	IB_MGMT_RMPP_HDR = 36,
	IB_MGMT_RMPP_DATA = 220,
	IB_MGMT_VENDOR_HDR = 40,
	IB_MGMT_VENDOR_DATA = 216,
	IB_MGMT_SA_HDR = 56,
	IB_MGMT_SA_DATA = 200,
	IB_MGMT_DEVICE_HDR = 64,
	IB_MGMT_DEVICE_DATA = 192,
	IB_MGMT_MAD_SIZE = IB_MGMT_MAD_HDR + IB_MGMT_MAD_DATA,
	OPA_MGMT_MAD_DATA = 2024,
	OPA_MGMT_RMPP_DATA = 2012,
	OPA_MGMT_MAD_SIZE = IB_MGMT_MAD_HDR + OPA_MGMT_MAD_DATA,
};

struct ib_mad_hdr {
	u8	base_version;
	u8	mgmt_class;
	u8	class_version;
	u8	method;
	__be16	status;
	__be16	class_specific;
	__be64	tid;
	__be16	attr_id;
	__be16	resv;
	__be32	attr_mod;
};

struct ib_rmpp_hdr {
	u8	rmpp_version;
	u8	rmpp_type;
	u8	rmpp_rtime_flags;
	u8	rmpp_status;
	__be32	seg_num;
	__be32	paylen_newwin;
};

typedef u64 __bitwise ib_sa_comp_mask;

#define IB_SA_COMP_MASK(n) ((__force ib_sa_comp_mask) cpu_to_be64(1ull << (n)))

/*
 * ib_sa_hdr and ib_sa_mad structures must be packed because they have
 * 64-bit fields that are only 32-bit aligned. 64-bit architectures will
 * lay them out wrong otherwise.  (And unfortunately they are sent on
 * the wire so we can't change the layout)
 */
struct ib_sa_hdr {
	__be64			sm_key;
	__be16			attr_offset;
	__be16			reserved;
	ib_sa_comp_mask		comp_mask;
} __packed;

struct ib_mad {
	struct ib_mad_hdr	mad_hdr;
	u8			data[IB_MGMT_MAD_DATA];
};

struct opa_mad {
	struct ib_mad_hdr	mad_hdr;
	u8			data[OPA_MGMT_MAD_DATA];
};

struct ib_rmpp_mad {
	struct ib_mad_hdr	mad_hdr;
	struct ib_rmpp_hdr	rmpp_hdr;
	u8			data[IB_MGMT_RMPP_DATA];
};

struct opa_rmpp_mad {
	struct ib_mad_hdr	mad_hdr;
	struct ib_rmpp_hdr	rmpp_hdr;
	u8			data[OPA_MGMT_RMPP_DATA];
};

struct ib_sa_mad {
	struct ib_mad_hdr	mad_hdr;
	struct ib_rmpp_hdr	rmpp_hdr;
	struct ib_sa_hdr	sa_hdr;
	u8			data[IB_MGMT_SA_DATA];
} __packed;

struct ib_vendor_mad {
	struct ib_mad_hdr	mad_hdr;
	struct ib_rmpp_hdr	rmpp_hdr;
	u8			reserved;
	u8			oui[3];
	u8			data[IB_MGMT_VENDOR_DATA];
};

#define IB_MGMT_CLASSPORTINFO_ATTR_ID	cpu_to_be16(0x0001)

#define IB_CLASS_PORT_INFO_RESP_TIME_MASK	0x1F
#define IB_CLASS_PORT_INFO_RESP_TIME_FIELD_SIZE 5

struct ib_class_port_info {
	u8			base_version;
	u8			class_version;
	__be16			capability_mask;
	  /* 27 bits for cap_mask2, 5 bits for resp_time */
	__be32			cap_mask2_resp_time;
	u8			redirect_gid[16];
	__be32			redirect_tcslfl;
	__be16			redirect_lid;
	__be16			redirect_pkey;
	__be32			redirect_qp;
	__be32			redirect_qkey;
	u8			trap_gid[16];
	__be32			trap_tcslfl;
	__be16			trap_lid;
	__be16			trap_pkey;
	__be32			trap_hlqp;
	__be32			trap_qkey;
};

/* PortInfo CapabilityMask */
enum ib_port_capability_mask_bits {
	IB_PORT_SM = 1 << 1,
	IB_PORT_NOTICE_SUP = 1 << 2,
	IB_PORT_TRAP_SUP = 1 << 3,
	IB_PORT_OPT_IPD_SUP = 1 << 4,
	IB_PORT_AUTO_MIGR_SUP = 1 << 5,
	IB_PORT_SL_MAP_SUP = 1 << 6,
	IB_PORT_MKEY_NVRAM = 1 << 7,
	IB_PORT_PKEY_NVRAM = 1 << 8,
	IB_PORT_LED_INFO_SUP = 1 << 9,
	IB_PORT_SM_DISABLED = 1 << 10,
	IB_PORT_SYS_IMAGE_GUID_SUP = 1 << 11,
	IB_PORT_PKEY_SW_EXT_PORT_TRAP_SUP = 1 << 12,
	IB_PORT_EXTENDED_SPEEDS_SUP = 1 << 14,
	IB_PORT_CAP_MASK2_SUP = 1 << 15,
	IB_PORT_CM_SUP = 1 << 16,
	IB_PORT_SNMP_TUNNEL_SUP = 1 << 17,
	IB_PORT_REINIT_SUP = 1 << 18,
	IB_PORT_DEVICE_MGMT_SUP = 1 << 19,
	IB_PORT_VENDOR_CLASS_SUP = 1 << 20,
	IB_PORT_DR_NOTICE_SUP = 1 << 21,
	IB_PORT_CAP_MASK_NOTICE_SUP = 1 << 22,
	IB_PORT_BOOT_MGMT_SUP = 1 << 23,
	IB_PORT_LINK_LATENCY_SUP = 1 << 24,
	IB_PORT_CLIENT_REG_SUP = 1 << 25,
	IB_PORT_OTHER_LOCAL_CHANGES_SUP = 1 << 26,
	IB_PORT_LINK_SPEED_WIDTH_TABLE_SUP = 1 << 27,
	IB_PORT_VENDOR_SPECIFIC_MADS_TABLE_SUP = 1 << 28,
	IB_PORT_MCAST_PKEY_TRAP_SUPPRESSION_SUP = 1 << 29,
	IB_PORT_MCAST_FDB_TOP_SUP = 1 << 30,
	IB_PORT_HIERARCHY_INFO_SUP = 1ULL << 31,
};

enum ib_port_capability_mask2_bits {
	IB_PORT_SET_NODE_DESC_SUP		= 1 << 0,
	IB_PORT_EX_PORT_INFO_EX_SUP		= 1 << 1,
	IB_PORT_VIRT_SUP			= 1 << 2,
	IB_PORT_SWITCH_PORT_STATE_TABLE_SUP	= 1 << 3,
	IB_PORT_LINK_WIDTH_2X_SUP		= 1 << 4,
	IB_PORT_LINK_SPEED_HDR_SUP		= 1 << 5,
};

#define OPA_CLASS_PORT_INFO_PR_SUPPORT BIT(26)

struct opa_class_port_info {
	u8 base_version;
	u8 class_version;
	__be16 cap_mask;
	__be32 cap_mask2_resp_time;

	u8 redirect_gid[16];
	__be32 redirect_tc_fl;
	__be32 redirect_lid;
	__be32 redirect_sl_qp;
	__be32 redirect_qkey;

	u8 trap_gid[16];
	__be32 trap_tc_fl;
	__be32 trap_lid;
	__be32 trap_hl_qp;
	__be32 trap_qkey;

	__be16 trap_pkey;
	__be16 redirect_pkey;

	u8 trap_sl_rsvd;
	u8 reserved[3];
} __packed;

/**
 * ib_get_cpi_resp_time - Returns the resp_time value from
 * cap_mask2_resp_time in ib_class_port_info.
 * @cpi: A struct ib_class_port_info mad.
 */
static inline u8 ib_get_cpi_resp_time(struct ib_class_port_info *cpi)
{
	return (u8)(be32_to_cpu(cpi->cap_mask2_resp_time) &
		    IB_CLASS_PORT_INFO_RESP_TIME_MASK);
}

/**
 * ib_set_cpi_resptime - Sets the response time in an
 * ib_class_port_info mad.
 * @cpi: A struct ib_class_port_info.
 * @rtime: The response time to set.
 */
static inline void ib_set_cpi_resp_time(struct ib_class_port_info *cpi,
					u8 rtime)
{
	cpi->cap_mask2_resp_time =
		(cpi->cap_mask2_resp_time &
		 cpu_to_be32(~IB_CLASS_PORT_INFO_RESP_TIME_MASK)) |
		cpu_to_be32(rtime & IB_CLASS_PORT_INFO_RESP_TIME_MASK);
}

/**
 * ib_get_cpi_capmask2 - Returns the capmask2 value from
 * cap_mask2_resp_time in ib_class_port_info.
 * @cpi: A struct ib_class_port_info mad.
 */
static inline u32 ib_get_cpi_capmask2(struct ib_class_port_info *cpi)
{
	return (be32_to_cpu(cpi->cap_mask2_resp_time) >>
		IB_CLASS_PORT_INFO_RESP_TIME_FIELD_SIZE);
}

/**
 * ib_set_cpi_capmask2 - Sets the capmask2 in an
 * ib_class_port_info mad.
 * @cpi: A struct ib_class_port_info.
 * @capmask2: The capmask2 to set.
 */
static inline void ib_set_cpi_capmask2(struct ib_class_port_info *cpi,
				       u32 capmask2)
{
	cpi->cap_mask2_resp_time =
		(cpi->cap_mask2_resp_time &
		 cpu_to_be32(IB_CLASS_PORT_INFO_RESP_TIME_MASK)) |
		cpu_to_be32(capmask2 <<
			    IB_CLASS_PORT_INFO_RESP_TIME_FIELD_SIZE);
}

/**
 * opa_get_cpi_capmask2 - Returns the capmask2 value from
 * cap_mask2_resp_time in ib_class_port_info.
 * @cpi: A struct opa_class_port_info mad.
 */
static inline u32 opa_get_cpi_capmask2(struct opa_class_port_info *cpi)
{
	return (be32_to_cpu(cpi->cap_mask2_resp_time) >>
		IB_CLASS_PORT_INFO_RESP_TIME_FIELD_SIZE);
}

struct ib_mad_notice_attr {
	u8 generic_type;
	u8 prod_type_msb;
	__be16 prod_type_lsb;
	__be16 trap_num;
	__be16 issuer_lid;
	__be16 toggle_count;

	union {
		struct {
			u8	details[54];
		} raw_data;

		struct {
			__be16	reserved;
			__be16	lid;		/* where violation happened */
			u8	port_num;	/* where violation happened */
		} __packed ntc_129_131;

		struct {
			__be16	reserved;
			__be16	lid;		/* LID where change occurred */
			u8	reserved2;
			u8	local_changes;	/* low bit - local changes */
			__be32	new_cap_mask;	/* new capability mask */
			u8	reserved3;
			u8	change_flags;	/* low 3 bits only */
		} __packed ntc_144;

		struct {
			__be16	reserved;
			__be16	lid;		/* lid where sys guid changed */
			__be16	reserved2;
			__be64	new_sys_guid;
		} __packed ntc_145;

		struct {
			__be16	reserved;
			__be16	lid;
			__be16	dr_slid;
			u8	method;
			u8	reserved2;
			__be16	attr_id;
			__be32	attr_mod;
			__be64	mkey;
			u8	reserved3;
			u8	dr_trunc_hop;
			u8	dr_rtn_path[30];
		} __packed ntc_256;

		struct {
			__be16		reserved;
			__be16		lid1;
			__be16		lid2;
			__be32		key;
			__be32		sl_qp1;	/* SL: high 4 bits */
			__be32		qp2;	/* high 8 bits reserved */
			union ib_gid	gid1;
			union ib_gid	gid2;
		} __packed ntc_257_258;

	} details;
};

/**
 * ib_mad_send_buf - MAD data buffer and work request for sends.
 * @next: A pointer used to chain together MADs for posting.
 * @mad: References an allocated MAD data buffer for MADs that do not have
 *   RMPP active.  For MADs using RMPP, references the common and management
 *   class specific headers.
 * @mad_agent: MAD agent that allocated the buffer.
 * @ah: The address handle to use when sending the MAD.
 * @context: User-controlled context fields.
 * @hdr_len: Indicates the size of the data header of the MAD.  This length
 *   includes the common MAD, RMPP, and class specific headers.
 * @data_len: Indicates the total size of user-transferred data.
 * @seg_count: The number of RMPP segments allocated for this send.
 * @seg_size: Size of the data in each RMPP segment.  This does not include
 *   class specific headers.
 * @seg_rmpp_size: Size of each RMPP segment including the class specific
 *   headers.
 * @timeout_ms: Time to wait for a response.
 * @retries: Number of times to retry a request for a response.  For MADs
 *   using RMPP, this applies per window.  On completion, returns the number
 *   of retries needed to complete the transfer.
 *
 * Users are responsible for initializing the MAD buffer itself, with the
 * exception of any RMPP header.  Additional segment buffer space allocated
 * beyond data_len is padding.
 */
struct ib_mad_send_buf {
	struct ib_mad_send_buf	*next;
	void			*mad;
	struct ib_mad_agent	*mad_agent;
	struct ib_ah		*ah;
	void			*context[2];
	int			hdr_len;
	int			data_len;
	int			seg_count;
	int			seg_size;
	int			seg_rmpp_size;
	int			timeout_ms;
	int			retries;
};

/**
 * ib_response_mad - Returns if the specified MAD has been generated in
 *   response to a sent request or trap.
 */
int ib_response_mad(const struct ib_mad_hdr *hdr);

/**
 * ib_get_rmpp_resptime - Returns the RMPP response time.
 * @rmpp_hdr: An RMPP header.
 */
static inline u8 ib_get_rmpp_resptime(struct ib_rmpp_hdr *rmpp_hdr)
{
	return rmpp_hdr->rmpp_rtime_flags >> 3;
}

/**
 * ib_get_rmpp_flags - Returns the RMPP flags.
 * @rmpp_hdr: An RMPP header.
 */
static inline u8 ib_get_rmpp_flags(struct ib_rmpp_hdr *rmpp_hdr)
{
	return rmpp_hdr->rmpp_rtime_flags & 0x7;
}

/**
 * ib_set_rmpp_resptime - Sets the response time in an RMPP header.
 * @rmpp_hdr: An RMPP header.
 * @rtime: The response time to set.
 */
static inline void ib_set_rmpp_resptime(struct ib_rmpp_hdr *rmpp_hdr, u8 rtime)
{
	rmpp_hdr->rmpp_rtime_flags = ib_get_rmpp_flags(rmpp_hdr) | (rtime << 3);
}

/**
 * ib_set_rmpp_flags - Sets the flags in an RMPP header.
 * @rmpp_hdr: An RMPP header.
 * @flags: The flags to set.
 */
static inline void ib_set_rmpp_flags(struct ib_rmpp_hdr *rmpp_hdr, u8 flags)
{
	rmpp_hdr->rmpp_rtime_flags = (rmpp_hdr->rmpp_rtime_flags & 0xF8) |
				     (flags & 0x7);
}

#endif /* IB_MAD_H */
