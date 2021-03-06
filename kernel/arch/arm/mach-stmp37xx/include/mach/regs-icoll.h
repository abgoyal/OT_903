
#ifndef _MACH_REGS_ICOLL
#define _MACH_REGS_ICOLL

#define REGS_ICOLL_BASE	(STMP3XXX_REGS_BASE + 0x0)

#define HW_ICOLL_VECTOR		0x0

#define HW_ICOLL_LEVELACK	0x10

#define HW_ICOLL_CTRL		0x20
#define BM_ICOLL_CTRL_CLKGATE	0x40000000
#define BM_ICOLL_CTRL_SFTRST	0x80000000

#define HW_ICOLL_STAT		0x30

#define HW_ICOLL_PRIORITY0	(0x60 + 0 * 0x10)
#define HW_ICOLL_PRIORITY1	(0x60 + 1 * 0x10)
#define HW_ICOLL_PRIORITY2	(0x60 + 2 * 0x10)
#define HW_ICOLL_PRIORITY3	(0x60 + 3 * 0x10)

#define HW_ICOLL_PRIORITYn	0x60

#endif
