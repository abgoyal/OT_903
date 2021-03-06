

#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/blackfin.h>
#include <asm/asm-offsets.h>
#include <asm/trace.h>

__INIT

/* Lay the initial stack into the L1 scratch area of Core B */
#define INITIAL_STACK	(COREB_L1_SCRATCH_START + L1_SCRATCH_LENGTH - 12)

ENTRY(_coreb_trampoline_start)
	/* Set the SYSCFG register */
	R0 = 0x36;
	SYSCFG = R0; /*Enable Cycle Counter and Nesting Of Interrupts(3rd Bit)*/
	R0 = 0;

	/*Clear Out All the data and pointer  Registers*/
	R1 = R0;
	R2 = R0;
	R3 = R0;
	R4 = R0;
	R5 = R0;
	R6 = R0;
	R7 = R0;

	P0 = R0;
	P1 = R0;
	P2 = R0;
	P3 = R0;
	P4 = R0;
	P5 = R0;

	LC0 = r0;
	LC1 = r0;
	L0 = r0;
	L1 = r0;
	L2 = r0;
	L3 = r0;

	/* Clear Out All the DAG Registers*/
	B0 = r0;
	B1 = r0;
	B2 = r0;
	B3 = r0;

	I0 = r0;
	I1 = r0;
	I2 = r0;
	I3 = r0;

	M0 = r0;
	M1 = r0;
	M2 = r0;
	M3 = r0;

	trace_buffer_init(p0,r0);

	/* Turn off the icache */
	p0.l = LO(IMEM_CONTROL);
	p0.h = HI(IMEM_CONTROL);
	R1 = [p0];
	R0 = ~ENICPLB;
	R0 = R0 & R1;

	/* Disabling of CPLBs should be proceeded by a CSYNC */
	CSYNC;
	[p0] = R0;
	SSYNC;

	/* Turn off the dcache */
	p0.l = LO(DMEM_CONTROL);
	p0.h = HI(DMEM_CONTROL);
	R1 = [p0];
	R0 = ~ENDCPLB;
	R0 = R0 & R1;

	/* Disabling of CPLBs should be proceeded by a CSYNC */
	CSYNC;
	[p0] = R0;
	SSYNC;

	/* in case of double faults, save a few things */
	p0.l = _init_retx_coreb;
	p0.h = _init_retx_coreb;
	R0 = RETX;
	[P0] = R0;

#ifdef CONFIG_DEBUG_DOUBLEFAULT
	/* Only save these if we are storing them,
	 * This happens here, since L1 gets clobbered
	 * below
	 */
	GET_PDA(p0, r0);
	r7 = [p0 + PDA_DF_RETX];
	p1.l = _init_saved_retx_coreb;
	p1.h = _init_saved_retx_coreb;
	[p1] = r7;

	r7 = [p0 + PDA_DF_DCPLB];
	p1.l = _init_saved_dcplb_fault_addr_coreb;
	p1.h = _init_saved_dcplb_fault_addr_coreb;
	[p1] = r7;

	r7 = [p0 + PDA_DF_ICPLB];
	p1.l = _init_saved_icplb_fault_addr_coreb;
	p1.h = _init_saved_icplb_fault_addr_coreb;
	[p1] = r7;

	r7 = [p0 + PDA_DF_SEQSTAT];
	p1.l = _init_saved_seqstat_coreb;
	p1.h = _init_saved_seqstat_coreb;
	[p1] = r7;
#endif

	/* Initialize stack pointer */
	sp.l = lo(INITIAL_STACK);
	sp.h = hi(INITIAL_STACK);
	fp = sp;
	usp = sp;

	/* This section keeps the processor in supervisor mode
	 * during core B startup.  Branches to the idle task.
	 */

	/* EVT15 = _real_start */

	p0.l = lo(EVT15);
	p0.h = hi(EVT15);
	p1.l = _coreb_start;
	p1.h = _coreb_start;
	[p0] = p1;
	csync;

	p0.l = lo(IMASK);
	p0.h = hi(IMASK);
	p1.l = IMASK_IVG15;
	p1.h = 0x0;
	[p0] = p1;
	csync;

	raise 15;
	p0.l = .LWAIT_HERE;
	p0.h = .LWAIT_HERE;
	reti = p0;
#if defined(ANOMALY_05000281)
	nop; nop; nop;
#endif
	rti;

.LWAIT_HERE:
	jump .LWAIT_HERE;
ENDPROC(_coreb_trampoline_start)
ENTRY(_coreb_trampoline_end)

.section ".text"
ENTRY(_set_sicb_iwr)
	P0.H = hi(SICB_IWR0);
	P0.L = lo(SICB_IWR0);
	P1.H = hi(SICB_IWR1);
	P1.L = lo(SICB_IWR1);
	[P0] = R0;
	[P1] = R1;
	SSYNC;
	RTS;
ENDPROC(_set_sicb_iwr)

ENTRY(_coreb_sleep)
	sp.l = lo(INITIAL_STACK);
	sp.h = hi(INITIAL_STACK);
	fp = sp;
	usp = sp;

	call _set_sicb_iwr;

	CLI R2;
	SSYNC;
	IDLE;
	STI R2;

	R0 = IWR_DISABLE_ALL;
	R1 = IWR_DISABLE_ALL;
	call _set_sicb_iwr;

	p0.h = hi(COREB_L1_CODE_START);
	p0.l = lo(COREB_L1_CODE_START);
	jump (p0);
ENDPROC(_coreb_sleep)

__CPUINIT
ENTRY(_coreb_start)
	[--sp] = reti;

	p0.l = lo(WDOGB_CTL);
	p0.h = hi(WDOGB_CTL);
	r0 = 0xAD6(z);
	w[p0] = r0;	/* Clear the watchdog. */
	ssync;

	/*
	 * switch to IDLE stack.
	 */
	p0.l = _secondary_stack;
	p0.h = _secondary_stack;
	sp = [p0];
	usp = sp;
	fp = sp;
#ifdef CONFIG_HOTPLUG_CPU
	p0.l = _hotplug_coreb;
	p0.h = _hotplug_coreb;
	r0 = [p0];
	cc = BITTST(r0, 0);
	if cc jump 3f;
#endif
	sp += -12;
	call _init_pda
	sp += 12;
#ifdef CONFIG_HOTPLUG_CPU
3:
#endif
	call _secondary_start_kernel;
.L_exit:
	jump.s	.L_exit;
ENDPROC(_coreb_start)
