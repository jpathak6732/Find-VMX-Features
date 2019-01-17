/*  
 *  cmpe283-1.c - Kernel module for CMPE283 assignment 1
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <asm/msr.h>

#define MAX_MSG 80

/*
 * Model specific registers (MSRs) by the module.
 * See SDM volume 4, section 2.1
 */
#define IA32_VMX_PINBASED_CTLS 0x481
#define IA32_VMX_ENTRY_CTLS 0x484
#define IA32_VMX_EXIT_CTLS 0x483
#define IA32_VMX_PROCBASED_CTLS 0x482
#define IA32_VMX_PROCBASED_CTLS2 0x48B

// To determine VMX true controls capability 
#define IA32_VMX_BASIC 0x480
#define IA32_VMX_TRUE_PINBASED_CTLS 0x48D
#define IA32_VMX_TRUE_PROCBASED_CTLS 0x48E
#define IA32_VMX_TRUE_EXIT_CTLS 0x48F
#define IA32_TRUE_ENTRY_CTLS 0x490

/*
 * struct caapability_info
 *
 * Represents a single capability (bit number and description).
 * Used by report_capability to output VMX capabilities.
 */
struct capability_info {
	uint8_t bit;
	const char *name;
};


/*
 * Pinbased capabilities
 * See SDM volume 3, section 24.6.1
 */
struct capability_info pinbased[5] =
{
	{ 0, "External Interrupt Exiting" },
	{ 3, "NMI Exiting" },
	{ 5, "Virtual NMIs" },
	{ 6, "Activate VMX Preemption Timer" },
	{ 7, "Process Posted Interrupts" }
};

/*
 * Primary Proc-based capabilities
 * See SDM volume 3C, section 24.6.2
 */
struct capability_info procbased[21] =
{
	{ 2, " Interrupt-window exiting" },
	{ 3, "Use TSC offsetting " },
	{ 7, "HLT exiting " },
	{ 9, "INVLPG exiting " },
	{ 10, "MWAIT exiting" },
	{ 11, "RDPMC exiting" },
	{ 12, "RDTSC exiting" },
	{ 15, "CR3-load exiting" },
	{ 16, "CR3-store exiting" },
	{ 19, "CR8-load exiting" },
	{ 20, "CR8-store exiting" },
	{ 21, "Use TPR shadow " },
	{ 22, "NMI-window exiting" },
	{ 23, "MOV-DR exiting" },
	{ 24, "Unconditional I/O exiting" },
	{ 25, "Use I/O bitmaps " },
	{ 27, "Monitor trap flag " },
	{ 28, "Use MSR bitmaps" },
	{ 29, "MONITOR exiting" },
	{ 30, "PAUSE exiting" },
	{ 31, "Activate secondary controls" }
}; 

/*
 * Secondary Proc-based capabilities
 * See SDM volume 3, section 24.6.2
 */
struct capability_info secondary_procbased[23] =
{
	{ 0, " Virtualize APIC accesses" },
	{ 1, "Enable EPT " },
	{ 2, "Descriptor-table exiting " },
	{ 3, "Enable RDTSCP " },
	{ 4, "Virtualize x2APIC mode" },
	{ 5, "Enable VPID" },
	{ 6, "WBINVD exiting" },
	{ 7, "Unrestricted guest" },
	{ 8, "APIC-register virtualization" },
	{ 9, "Virtual-interrupt delivery" },
	{ 10, "PAUSE-loop exiting" },
	{ 11, "RDRAND exiting " },
	{ 12, "Enable INVPCID" },
	{ 13, "Enable VM functions" },
	{ 14, "VMCS shadowing" },
	{ 15, "Enable ENCLS exiting " },
	{ 16, "RDSEED exiting " },
	{ 17, "Enable PML" },
	{ 18, "EPT-violation #VE" },
	{ 19, "Conceal VMX non-root operation from Intel PT" },
	{ 20, "Enable XSAVES/XRSTORS" },
	{ 22, "Mode-based execution control for EPT" },
	{ 25, "Use TSC scaling" }
}; 

/*
 * Primary Proc-based capabilities
 * See SDM volume 3C, section 24.7.1
 */
struct capability_info entry_controls[9] =
{
	{ 2, "Load debug controls" },
	{ 9, "IA-32e mode guest" },
	{ 10, "Entry to SMM" },
	{ 11, "Deactivate dual-monitor treatment " },
	{ 13, "Load IA32_PERF_GLOBAL_CTRL" },
	{ 14, "Load IA32_PAT" },
	{ 15, "Load IA32_EFER" },
	{ 16, "Load IA32_BNDCFGS" },
	{ 17, "Conceal VM entries from intel PT" }
};

/*
 * Primary Proc-based capabilities
 * See SDM volume 3C, section 24.8.1
 */
struct capability_info exit_controls[11] =
{
	{ 2, "Save debug controls" },
	{ 9, "Host address-space size" },
	{ 12, "Load IA32_PERF_GLOB AL_CTRL" },
	{ 15, "Acknowledge interrupt on exit " },
	{ 18, "Save IA32_PAT" },
	{ 19, "Load IA32_PAT" },
	{ 20, "Save IA32_EFER" },
	{ 21, "Load IA32_EFER" },
	{ 22, "Save VMX-preemption timer value" },
	{ 23, "Clear IA32_BNDCFGS" },
	{ 24, "Conceal VM exits from Intel PT" }
};

/*
 * report_capability
 *
 * Reports capabilities present in 'cap' using the corresponding MSR values
 * provided in 'lo' and 'hi'.
 *
 * Parameters:
 *  cap: capability_info structure for this feature
 *  len: number of entries in 'cap'
 *  lo: low 32 bits of capability MSR value describing this feature
 *  hi: high 32 bits of capability MSR value describing this feature
 */
void
report_capability(struct capability_info *cap, uint8_t len, uint32_t lo,
    uint32_t hi)
{
	uint8_t i;
	struct capability_info *c;
	char msg[MAX_MSG];

	memset(msg, 0, sizeof(msg));

	for (i = 0; i < len; i++) {
		c = &cap[i];
		snprintf(msg, 79, "  %s: Can set=%s, Can clear=%s\n",
		    c->name,
		    (hi & (1 << c->bit)) ? "Yes" : "No",
		    !(lo & (1 << c->bit)) ? "Yes" : "No");
		printk(msg);
	}
}

/*
 * detect_true_vmx_features
 *
 * Detects and prints true VMX capabilities of this host's CPU.
 */
void
detect_true_vmx_features(void)
{
	uint32_t lo, hi;

	/* True Pinbased Controls */
	rdmsr(IA32_VMX_TRUE_PINBASED_CTLS, lo, hi);
	pr_info("True Pinbased Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(pinbased, 5, lo, hi);


	/* True Primary Procbased controls */
	rdmsr(IA32_VMX_TRUE_PROCBASED_CTLS, lo, hi);
	pr_info("True Primary Procbased Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(procbased, 21, lo, hi);

	/* True Secondary Procbased controls */
	rdmsr(IA32_VMX_TRUE_PROCBASED_CTLS, lo, hi);
	pr_info("True Secondary Procbased Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(secondary_procbased, 23, lo, hi);


	/* True Entry controls */
	rdmsr(IA32_TRUE_ENTRY_CTLS, lo, hi);
	pr_info("True Entry Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(entry_controls, 9, lo, hi);


	/* True Exit controls */
	rdmsr(IA32_VMX_TRUE_EXIT_CTLS, lo, hi);
	pr_info("True Exit Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(exit_controls, 7, lo, hi);

}

/* Determine if true controls are available:
 * 
 * Read the IA32_VMX_BASIC MSR
 * Check bit 55 – if set, true controls are available
 */
void
check_bit_55(void)
{
	uint32_t lo, hi;

	rdmsr(IA32_VMX_BASIC, lo, hi);
	
	if ( hi & (1 << (55 - 32)))
	{
		printk("**** True VMX Capabilities are available !!!  ****");
		detect_true_vmx_features();
	}
	else
	{
		printk("**** True VMX Capabilities are NOT available !!!  ****");	
	}
}

/* Determine if secondary procbased controls are available:
 * 
 * Read the IA32_VMX_PROCBASED_CTLS
 * Check bit 63 – if set, secondary procbased controls are available
 */
void
check_bit_63(void)
{
	uint32_t lo, hi;

	/* Procbased controls */
	rdmsr(IA32_VMX_PROCBASED_CTLS, lo, hi);

	if ( hi & (1 << (63 - 32)))
	{
		printk("**** Secondary procbased controls are available !!! ****");
			/* secondary procbased controls */
			rdmsr(IA32_VMX_PROCBASED_CTLS2, lo, hi);
			pr_info("Secondary procbased Controls MSR: 0x%llx\n",
				(uint64_t)(lo | (uint64_t)hi << 32));
			report_capability(secondary_procbased, 23, lo, hi);
	}
	else
	{
		printk("**** Secondary procbased controls are NOT available !!! ****");
	}
}

/*
 * detect_vmx_features
 *
 * Detects and prints VMX capabilities of this host's CPU.
 */
void
detect_vmx_features(void)
{
	uint32_t lo, hi;

	/* Pinbased controls */
	rdmsr(IA32_VMX_PINBASED_CTLS, lo, hi);
	pr_info("Pinbased Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(pinbased, 5, lo, hi);


	/* Procbased controls */
	rdmsr(IA32_VMX_PROCBASED_CTLS, lo, hi);
	pr_info("Procbased Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(procbased, 21, lo, hi);
	
	/* Check availability of Secondary Procbased controls */
	check_bit_63();
	
	/* Entry controls */
	rdmsr(IA32_VMX_ENTRY_CTLS, lo, hi);
	pr_info("Entry Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(entry_controls, 9, lo, hi);


	/* Exit controls */
	rdmsr(IA32_VMX_EXIT_CTLS, lo, hi);
	pr_info("Exit Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(exit_controls, 7, lo, hi);
	
	/* Check True VMX capabilities */
	check_bit_55();
}

/*
 * init_module
 *
 * Module entry point
 *
 * Return Values:
 *  Always 0
 */
int
init_module(void)
{
	printk(KERN_INFO "CMPE 283 Assignment 1 Module Start\n");
	detect_vmx_features();

	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

/*
 * cleanup_module
 *
 * Function called on module unload
 */
void
cleanup_module(void)
{
	printk(KERN_INFO "CMPE 283 Assignment 1 Module Exits\n");
}
