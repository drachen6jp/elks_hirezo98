#ifndef __LINUXMT_CONFIG_H
#define __LINUXMT_CONFIG_H

#include <autoconf.h>
#include <linuxmt/major.h>

//#ifdef CONFIG_ARCH_IBMPC
#if defined(CONFIG_ARCH_IBMPC) || defined(CONFIG_ARCH_PC98)
/*
 * Compile-time configuration
 */
#define MAX_SERIAL		4		/* max number of serial tty devices*/

/*
 * Setup data - normally queried by startup setup.S code, but can
 * be overridden for embedded systems with less overhead.
 * See setup.S for more details.
 */
#define SETUP_VID_COLS		setupb(7)		/* BIOS video # columns */
#define SETUP_VID_LINES		setupb(14)		/* BIOS video # lines */
#define SETUP_CPU_TYPE		setupb(0x20)	/* processor type */
#define SETUP_MEM_KBYTES	setupw(0x2a)	/* base memory in 1K bytes */
#ifdef CONFIG_ARCH_PC98
//#define SETUP_ROOT_DEV		0x0380	/* root device */
#define SETUP_ROOT_DEV		0x0300	/* root device */
#else
#define SETUP_ROOT_DEV		setupw(0x1fc)	/* root device, kdev_t or BIOS dev */
#endif
#define SETUP_ELKS_FLAGS	setupw(0x1f6)	/* flags for root device type */
#define SETUP_PART_OFFSETLO	setupw(0x1e2)	/* partition offset low word */
#define SETUP_PART_OFFSETHI	setupw(0x1e4)	/* partition offset high word */
#ifdef CONFIG_ROMCODE
#define SYS_CAPS	(CAP_PC_AT|CAP_DRIVE_PARMS)
#endif
#endif

#ifdef CONFIG_ARCH_8018X
#define MAX_SERIAL		2	/* max number of serial tty devices*/

#define SETUP_VID_COLS		80	/* BIOS video # columns */
#define SETUP_VID_LINES		25	/* BIOS video # lines */
#define SETUP_CPU_TYPE		5	/* processor type 80186 */
#define SETUP_MEM_KBYTES	128	/* base memory in 1K bytes */
#define SETUP_ROOT_DEV		0x0600	/* root device ROMFS */
#define SETUP_ELKS_FLAGS	0	/* flags for root device type */
#define SETUP_PART_OFFSETLO	0	/* partition offset low word */
#define SETUP_PART_OFFSETHI	0	/* partition offset high word */
#define SYS_CAPS		0	/* no XT/AT capabilities */

#define SETUP_HEAPSIZE		1024	/* minimum kernel heap for now*/

#define CONFIG_8018X_FCPU	20
#define CONFIG_8018X_EB
#endif

/*
 * System capabilities - configurable for ROM or custom installations.
 * Normally, all capabilities will be set if arch_cpu > 5 (PC/AT),
 * except when SYS_CAPS is defined for custom installations or emulations.
 */
#define CAP_PC_AT       0x01      /* PC/AT capabilities */
#define CAP_DRIVE_PARMS	0x02      /* has INT 13h fn 8 drive parms */
#define CAP_KBD_LEDS    0x04      /* has keyboard LEDs */
#define CAP_HD_IDE      0x08      /* can do hard drive IDE probes */
#define CAP_IRQ8TO15    CAP_PC_AT /* has IRQ 8 through 15 */
#define CAP_IRQ2MAP9    CAP_PC_AT /* map IRQ 2 to 9 */
#define CAP_ALL         0xFF      /* all capabilities if PC/AT only */

/* Don't touch these, unless you really know what you are doing. */
#define DEF_INITSEG	0x0100	/* setup data, for netboot use 0x5000 */
#define DEF_SYSSEG	0x1000
#define DEF_SETUPSEG	DEF_INITSEG + 0x20
#define DEF_SYSSIZE	0x2F00

#ifdef CONFIG_ROMCODE
#define SETUP_DATA	CONFIG_ROM_SETUP_DATA

#ifdef CONFIG_BLK_DEV_BIOS    /* BIOS disk driver*/
#define DMASEG		0x80  /* 0x400 bytes floppy sector buffer */
#ifdef CONFIG_TRACK_CACHE     /* floppy track buffer in low mem */
#define DMASEGSZ 0x2400       /* SECTOR_SIZE * 18 (9216) */
#define KERNEL_DATA	0x2C0 /* kernel data segment */
#else
#define DMASEGSZ 0x040	      /* BLOCK_SIZE (1024) */
#define KERNEL_DATA	0x0C0 /* kernel data segment */
#endif
#else
#define KERNEL_DATA     0x80  /* kernel data segment */
#endif

#else
#define SETUP_DATA	REL_INITSEG

/* Define segment locations of low memory, must not overlap */
#ifdef CONFIG_ARCH_PC98
#define DEF_OPTSEG	0x60  /* 0x100 bytes boot options*/
#define REL_INITSEG	0x70  /* 0x200 bytes setup data */
#define DMASEG		0x90  /* 0x400 bytes floppy sector buffer */
#else
#define DEF_OPTSEG	0x50  /* 0x100 bytes boot options*/
#define REL_INITSEG	0x60  /* 0x200 bytes setup data */
#define DMASEG		0x80  /* 0x400 bytes floppy sector buffer */
#endif

#ifdef CONFIG_TRACK_CACHE     /* floppy track buffer in low mem */
#ifdef CONFIG_ARCH_PC98
#define DMASEGSZ 0x2000	      /* SECTOR_SIZE * 8 (8192) */
#define REL_SYSSEG	0x290 /* kernel code segment */
#else
#define DMASEGSZ 0x2400	      /* SECTOR_SIZE * 18 (9216) */
#define REL_SYSSEG	0x2C0 /* kernel code segment */
#endif
#else
#ifdef CONFIG_ARCH_PC98
#define DMASEGSZ 0x0400	      /* BLOCK_SIZE (1024) */
#define REL_SYSSEG	0x0D0 /* kernel code segment */
#else
#define DMASEGSZ 0x0400	      /* BLOCK_SIZE (1024) */
#define REL_SYSSEG	0x0C0 /* kernel code segment */
#endif
#endif

#endif /* !CONFIG_ROMCODE */

// DMASEG is a bouncing buffer of 1K (= BLOCKSIZE)
// below the first 64K boundary (= 0x1000:0)
// for use with the old 8237 DMA controller
// OR a floppy track buffer of 9K (18 512-byte sectors)

/*
 * Defines for what uname() should return.
 * The definitions for UTS_RELEASE and UTS_VERSION are now passed as
 * kernel compilation parameters, and should only be used by linux/version.c
 */
#define UTS_SYSNAME "ELKS"
#define UTS_MACHINE "i8086"
#define UTS_NODENAME "(none)"		/* set by sethostname() */

#endif
