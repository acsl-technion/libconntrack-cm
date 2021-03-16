#pragma once

#include <infiniband/verbs.h>

#include <type_traits>

typedef unsigned int u32;
typedef unsigned int __u32;
typedef unsigned int __be32;
typedef unsigned long long u64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned short __be16;
typedef long long s64;
typedef int s32;

#define BIT(i) (1ull << (i))

#define __packed	__attribute__((packed))

#define be16_to_cpu be16toh
#define be32_to_cpu be32toh
#define be64_to_cpu be64toh

#define cpu_to_be16 htobe16
#define cpu_to_be32 htobe32
#define cpu_to_be64 htobe64

#define le16_to_cpu le16toh
#define le32_to_cpu le32toh
#define le64_to_cpu le64toh

#define cpu_to_le16 htole16
#define cpu_to_le32 htole32
#define cpu_to_le64 htole64

#define ib_gid ibv_gid

#include <linux/const.h>

#define _UL(x)		(_AC(x, UL))
#define _ULL(x)		(_AC(x, ULL))

#define UL(x)		(_UL(x))
#define ULL(x)		(_ULL(x))

#include <asm-generic/bitsperlong.h>

/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GENMASK(h, l) \
	(((~UL(0)) - (UL(1) << (l)) + 1) & \
	 (~UL(0) >> (__BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l) \
	(((~ULL(0)) - (ULL(1) << (l)) + 1) & \
	 (~ULL(0) >> (__BITS_PER_LONG_LONG - 1 - (h))))

# define __compiletime_error(message) __attribute__((error(message)))

#ifdef __OPTIMIZE__
# define __compiletime_assert(condition, msg, prefix, suffix)		\
	do {								\
		extern void prefix ## suffix(void) __compiletime_error(msg); \
		if (!(condition))					\
			prefix ## suffix();				\
	} while (0)
#else
# define __compiletime_assert(condition, msg, prefix, suffix) do { } while (0)
#endif

#define _compiletime_assert(condition, msg, prefix, suffix) \
	__compiletime_assert(condition, msg, prefix, suffix)

/**
 * compiletime_assert - break build and emit msg if condition is false
 * @condition: a compile-time constant condition to check
 * @msg:       a message to emit if condition is false
 *
 * In tradition of POSIX assert, this macro will break the build if the
 * supplied condition is *false*, emitting the supplied error message if the
 * compiler has support to do so.
 */
#define compiletime_assert(condition, msg) \
	_compiletime_assert(condition, msg, __compiletime_assert_, __COUNTER__)
