
#ifndef _IP6T_RATE_H
#define _IP6T_RATE_H

/* timings are in milliseconds. */
#define IP6T_LIMIT_SCALE 10000

struct ip6t_rateinfo {
	u_int32_t avg;    /* Average secs between packets * scale */
	u_int32_t burst;  /* Period multiplier for upper limit. */

#ifdef KERNEL_64_USERSPACE_32
	u_int64_t prev;
	u_int64_t placeholder;
#else
	/* Used internally by the kernel */
	unsigned long prev;
	/* Ugly, ugly fucker. */
	struct ip6t_rateinfo *master;
#endif
	u_int32_t credit;
	u_int32_t credit_cap, cost;
};
#endif /*_IPT_RATE_H*/
