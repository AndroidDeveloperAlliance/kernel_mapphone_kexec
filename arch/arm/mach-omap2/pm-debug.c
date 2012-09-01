/*
 * OMAP Power Management debug routines
 *
 * Copyright (C) 2005 Texas Instruments, Inc.
 * Copyright (C) 2006-2008 Nokia Corporation
 *
 * Written by:
 * Richard Woodruff <r-woodruff2@ti.com>
 * Tony Lindgren
 * Juha Yrjola
 * Amit Kucheria <amit.kucheria@nokia.com>
 * Igor Stoppa <igor.stoppa@nokia.com>
 * Jouni Hogander
 *
 * Based on pm.c for omap2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <plat/clock.h>
#include <plat/board.h>
#include "powerdomain.h"
#include "clockdomain.h"
#include <plat/dmtimer.h>
#include <plat/omap-pm.h>

#include "cm2xxx_3xxx.h"
#include "prm2xxx_3xxx.h"
#include "pm.h"

#define PM_DEBUG_MAX_SAVED_REGS	64
#define PM_DEBUG_PRM_MIN	0x4A306000
#define PM_DEBUG_PRM_MAX	(0x4A307F00 + (PM_DEBUG_MAX_SAVED_REGS * 4) - 1)
#define PM_DEBUG_CM1_MIN	0x4A004000
#define PM_DEBUG_CM1_MAX	(0x4A004F00 + (PM_DEBUG_MAX_SAVED_REGS * 4) - 1)
#define PM_DEBUG_CM2_MIN	0x4A008000
#define PM_DEBUG_CM2_MAX	(0x4A009F00 + (PM_DEBUG_MAX_SAVED_REGS * 4) - 1)

int omap2_pm_debug;
u32 enable_off_mode;
u32 sleep_while_idle;
u32 wakeup_timer_seconds;
u32 wakeup_timer_milliseconds;
u32 omap4_device_off_counter = 0;
u32 enable_regulator_dump;

#ifdef CONFIG_PM_ADVANCED_DEBUG
static u32 saved_reg_num;
static u32 saved_reg_num_used;
static u32 saved_reg_addr;
static u32 saved_reg_buff[2][PM_DEBUG_MAX_SAVED_REGS];
#endif

#define DUMP_PRM_MOD_REG(mod, reg)    \
	regs[reg_count].name = #mod "." #reg; \
	regs[reg_count++].val = omap2_prm_read_mod_reg(mod, reg)
#define DUMP_CM_MOD_REG(mod, reg)     \
	regs[reg_count].name = #mod "." #reg; \
	regs[reg_count++].val = omap2_cm_read_mod_reg(mod, reg)
#define DUMP_PRM_REG(reg) \
	regs[reg_count].name = #reg; \
	regs[reg_count++].val = __raw_readl(reg)
#define DUMP_CM_REG(reg) \
	regs[reg_count].name = #reg; \
	regs[reg_count++].val = __raw_readl(reg)
#define DUMP_INTC_REG(reg, off) \
	regs[reg_count].name = #reg; \
	regs[reg_count++].val = \
			 __raw_readl(OMAP2_L4_IO_ADDRESS(0x480fe000 + (off)))

void omap2_pm_dump(int mode, int resume, unsigned int us)
{
	struct reg {
		const char *name;
		u32 val;
	} regs[32];
	int reg_count = 0, i;
	const char *s1 = NULL, *s2 = NULL;

	if (!resume) {
#if 0
		/* MPU */
		DUMP_PRM_MOD_REG(OCP_MOD, OMAP2_PRM_IRQENABLE_MPU_OFFSET);
		DUMP_CM_MOD_REG(MPU_MOD, OMAP2_CM_CLKSTCTRL);
		DUMP_PRM_MOD_REG(MPU_MOD, OMAP2_PM_PWSTCTRL);
		DUMP_PRM_MOD_REG(MPU_MOD, OMAP2_PM_PWSTST);
		DUMP_PRM_MOD_REG(MPU_MOD, PM_WKDEP);
#endif
#if 0
		/* INTC */
		DUMP_INTC_REG(INTC_MIR0, 0x0084);
		DUMP_INTC_REG(INTC_MIR1, 0x00a4);
		DUMP_INTC_REG(INTC_MIR2, 0x00c4);
#endif
#if 0
		DUMP_CM_MOD_REG(CORE_MOD, CM_FCLKEN1);
		if (cpu_is_omap24xx()) {
			DUMP_CM_MOD_REG(CORE_MOD, OMAP24XX_CM_FCLKEN2);
			DUMP_PRM_MOD_REG(OMAP24XX_GR_MOD,
					OMAP2_PRCM_CLKEMUL_CTRL_OFFSET);
			DUMP_PRM_MOD_REG(OMAP24XX_GR_MOD,
					OMAP2_PRCM_CLKSRC_CTRL_OFFSET);
		}
		DUMP_CM_MOD_REG(WKUP_MOD, CM_FCLKEN);
		DUMP_CM_MOD_REG(CORE_MOD, CM_ICLKEN1);
		DUMP_CM_MOD_REG(CORE_MOD, CM_ICLKEN2);
		DUMP_CM_MOD_REG(WKUP_MOD, CM_ICLKEN);
		DUMP_CM_MOD_REG(PLL_MOD, CM_CLKEN);
		DUMP_CM_MOD_REG(PLL_MOD, CM_AUTOIDLE);
		DUMP_PRM_MOD_REG(CORE_MOD, OMAP2_PM_PWSTST);
#endif
#if 0
		/* DSP */
		if (cpu_is_omap24xx()) {
			DUMP_CM_MOD_REG(OMAP24XX_DSP_MOD, CM_FCLKEN);
			DUMP_CM_MOD_REG(OMAP24XX_DSP_MOD, CM_ICLKEN);
			DUMP_CM_MOD_REG(OMAP24XX_DSP_MOD, CM_IDLEST);
			DUMP_CM_MOD_REG(OMAP24XX_DSP_MOD, CM_AUTOIDLE);
			DUMP_CM_MOD_REG(OMAP24XX_DSP_MOD, CM_CLKSEL);
			DUMP_CM_MOD_REG(OMAP24XX_DSP_MOD, OMAP2_CM_CLKSTCTRL);
			DUMP_PRM_MOD_REG(OMAP24XX_DSP_MOD, OMAP2_RM_RSTCTRL);
			DUMP_PRM_MOD_REG(OMAP24XX_DSP_MOD, OMAP2_RM_RSTST);
			DUMP_PRM_MOD_REG(OMAP24XX_DSP_MOD, OMAP2_PM_PWSTCTRL);
			DUMP_PRM_MOD_REG(OMAP24XX_DSP_MOD, OMAP2_PM_PWSTST);
		}
#endif
	} else {
		DUMP_PRM_MOD_REG(CORE_MOD, PM_WKST1);
		if (cpu_is_omap24xx())
			DUMP_PRM_MOD_REG(CORE_MOD, OMAP24XX_PM_WKST2);
		DUMP_PRM_MOD_REG(WKUP_MOD, PM_WKST);
		DUMP_PRM_MOD_REG(OCP_MOD, OMAP2_PRCM_IRQSTATUS_MPU_OFFSET);
#if 1
		DUMP_INTC_REG(INTC_PENDING_IRQ0, 0x0098);
		DUMP_INTC_REG(INTC_PENDING_IRQ1, 0x00b8);
		DUMP_INTC_REG(INTC_PENDING_IRQ2, 0x00d8);
#endif
	}

	switch (mode) {
	case 0:
		s1 = "full";
		s2 = "retention";
		break;
	case 1:
		s1 = "MPU";
		s2 = "retention";
		break;
	case 2:
		s1 = "MPU";
		s2 = "idle";
		break;
	}

	if (!resume)
#ifdef CONFIG_NO_HZ
		printk(KERN_INFO
		       "--- Going to %s %s (next timer after %u ms)\n", s1, s2,
		       jiffies_to_msecs(get_next_timer_interrupt(jiffies) -
					jiffies));
#else
		printk(KERN_INFO "--- Going to %s %s\n", s1, s2);
#endif
	else
		printk(KERN_INFO "--- Woke up (slept for %u.%03u ms)\n",
			us / 1000, us % 1000);

	for (i = 0; i < reg_count; i++)
		printk(KERN_INFO "%-20s: 0x%08x\n", regs[i].name, regs[i].val);
}

void omap2_pm_wakeup_on_timer(u32 seconds, u32 milliseconds)
{
	u32 tick_rate, cycles;

	if (!seconds && !milliseconds)
		return;

	tick_rate = clk_get_rate(omap_dm_timer_get_fclk(gptimer_wakeup));
	cycles = tick_rate * seconds + tick_rate * milliseconds / 1000;
	omap_dm_timer_stop(gptimer_wakeup);
	omap_dm_timer_set_load_start(gptimer_wakeup, 0, 0xffffffff - cycles);

	pr_info("PM: Resume timer in %u.%03u secs"
		" (%d ticks at %d ticks/sec.)\n",
		seconds, milliseconds, cycles, tick_rate);
}

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>

static void pm_dbg_regset_store(u32 *ptr);

static struct dentry *pm_dbg_dir;

static int pm_dbg_init_done;

static int pm_dbg_init(void);

enum {
	DEBUG_FILE_COUNTERS = 0,
	DEBUG_FILE_TIMERS,
	DEBUG_FILE_LAST_COUNTERS,
	DEBUG_FILE_LAST_TIMERS,
};

struct pm_module_def {
	char name[8]; /* Name of the module */
	short type; /* CM or PRM */
	unsigned short offset;
	int low; /* First register address on this module */
	int high; /* Last register address on this module */
};

#define MOD_CM 0
#define MOD_PRM 1

static const struct pm_module_def *pm_dbg_reg_modules;
static const struct pm_module_def omap3_pm_reg_modules[] = {
	{ "IVA2", MOD_CM, OMAP3430_IVA2_MOD, 0, 0x4c },
	{ "OCP", MOD_CM, OCP_MOD, 0, 0x10 },
	{ "MPU", MOD_CM, MPU_MOD, 4, 0x4c },
	{ "CORE", MOD_CM, CORE_MOD, 0, 0x4c },
	{ "SGX", MOD_CM, OMAP3430ES2_SGX_MOD, 0, 0x4c },
	{ "WKUP", MOD_CM, WKUP_MOD, 0, 0x40 },
	{ "CCR", MOD_CM, PLL_MOD, 0, 0x70 },
	{ "DSS", MOD_CM, OMAP3430_DSS_MOD, 0, 0x4c },
	{ "CAM", MOD_CM, OMAP3430_CAM_MOD, 0, 0x4c },
	{ "PER", MOD_CM, OMAP3430_PER_MOD, 0, 0x4c },
	{ "EMU", MOD_CM, OMAP3430_EMU_MOD, 0x40, 0x54 },
	{ "NEON", MOD_CM, OMAP3430_NEON_MOD, 0x20, 0x48 },
	{ "USB", MOD_CM, OMAP3430ES2_USBHOST_MOD, 0, 0x4c },

	{ "IVA2", MOD_PRM, OMAP3430_IVA2_MOD, 0x50, 0xfc },
	{ "OCP", MOD_PRM, OCP_MOD, 4, 0x1c },
	{ "MPU", MOD_PRM, MPU_MOD, 0x58, 0xe8 },
	{ "CORE", MOD_PRM, CORE_MOD, 0x58, 0xf8 },
	{ "SGX", MOD_PRM, OMAP3430ES2_SGX_MOD, 0x58, 0xe8 },
	{ "WKUP", MOD_PRM, WKUP_MOD, 0xa0, 0xb0 },
	{ "CCR", MOD_PRM, PLL_MOD, 0x40, 0x70 },
	{ "DSS", MOD_PRM, OMAP3430_DSS_MOD, 0x58, 0xe8 },
	{ "CAM", MOD_PRM, OMAP3430_CAM_MOD, 0x58, 0xe8 },
	{ "PER", MOD_PRM, OMAP3430_PER_MOD, 0x58, 0xe8 },
	{ "EMU", MOD_PRM, OMAP3430_EMU_MOD, 0x58, 0xe4 },
	{ "GLBL", MOD_PRM, OMAP3430_GR_MOD, 0x20, 0xe4 },
	{ "NEON", MOD_PRM, OMAP3430_NEON_MOD, 0x58, 0xe8 },
	{ "USB", MOD_PRM, OMAP3430ES2_USBHOST_MOD, 0x58, 0xe8 },
	{ "", 0, 0, 0, 0 },
};

#define PM_DBG_MAX_REG_SETS 4

static void *pm_dbg_reg_set[PM_DBG_MAX_REG_SETS];

static int pm_dbg_get_regset_size(void)
{
	static int regset_size;

	if (regset_size == 0) {
		int i = 0;

		while (pm_dbg_reg_modules[i].name[0] != 0) {
			regset_size += pm_dbg_reg_modules[i].high +
				4 - pm_dbg_reg_modules[i].low;
			i++;
		}
	}
	return regset_size;
}

static int pm_dbg_show_regs(struct seq_file *s, void *unused)
{
	int i, j;
	unsigned long val;
	int reg_set = (int)s->private;
	u32 *ptr;
	void *store = NULL;
	int regs;
	int linefeed;

	if (reg_set == 0) {
		store = kmalloc(pm_dbg_get_regset_size(), GFP_KERNEL);
		ptr = store;
		pm_dbg_regset_store(ptr);
	} else {
		ptr = pm_dbg_reg_set[reg_set - 1];
	}

	i = 0;

	while (pm_dbg_reg_modules[i].name[0] != 0) {
		regs = 0;
		linefeed = 0;
		if (pm_dbg_reg_modules[i].type == MOD_CM)
			seq_printf(s, "MOD: CM_%s (%08x)\n",
				pm_dbg_reg_modules[i].name,
				(u32)(OMAP3430_CM_BASE +
				pm_dbg_reg_modules[i].offset));
		else
			seq_printf(s, "MOD: PRM_%s (%08x)\n",
				pm_dbg_reg_modules[i].name,
				(u32)(OMAP3430_PRM_BASE +
				pm_dbg_reg_modules[i].offset));

		for (j = pm_dbg_reg_modules[i].low;
			j <= pm_dbg_reg_modules[i].high; j += 4) {
			val = *(ptr++);
			if (val != 0) {
				regs++;
				if (linefeed) {
					seq_printf(s, "\n");
					linefeed = 0;
				}
				seq_printf(s, "  %02x => %08lx", j, val);
				if (regs % 4 == 0)
					linefeed = 1;
			}
		}
		seq_printf(s, "\n");
		i++;
	}

	if (store != NULL)
		kfree(store);

	return 0;
}

static void pm_dbg_regset_store(u32 *ptr)
{
	int i, j;
	u32 val;

	i = 0;

	while (pm_dbg_reg_modules[i].name[0] != 0) {
		for (j = pm_dbg_reg_modules[i].low;
			j <= pm_dbg_reg_modules[i].high; j += 4) {
			if (pm_dbg_reg_modules[i].type == MOD_CM)
				val = omap2_cm_read_mod_reg(
					pm_dbg_reg_modules[i].offset, j);
			else
				val = omap2_prm_read_mod_reg(
					pm_dbg_reg_modules[i].offset, j);
			*(ptr++) = val;
		}
		i++;
	}
}

int pm_dbg_regset_save(int reg_set)
{
	if (pm_dbg_reg_set[reg_set-1] == NULL)
		return -EINVAL;

	pm_dbg_regset_store(pm_dbg_reg_set[reg_set-1]);

	return 0;
}

static const char pwrdm_state_names[][PWRDM_MAX_PWRSTS] = {
	"OFF",
	"RET",
	"INA",
	"ON"
};

void pm_dbg_update_time(struct powerdomain *pwrdm, int prev)
{
	s64 t;

	if (!pm_dbg_init_done)
		return ;

	/* Update timer for previous state */
	t = sched_clock();

	pwrdm->time.state[prev] += t - pwrdm->timer;

	pwrdm->timer = t;
}

static int clkdm_dbg_show_counter(struct clockdomain *clkdm, void *user)
{
	struct seq_file *s = (struct seq_file *)user;

	if (strcmp(clkdm->name, "emu_clkdm") == 0 ||
		strcmp(clkdm->name, "wkup_clkdm") == 0 ||
		strncmp(clkdm->name, "dpll", 4) == 0)
		return 0;

	seq_printf(s, "%s->%s (%d)", clkdm->name,
			clkdm->pwrdm.ptr->name,
			atomic_read(&clkdm->usecount));
	seq_printf(s, "\n");

	return 0;
}

static int pwrdm_dbg_show_count_stats(struct powerdomain *pwrdm,
	struct powerdomain_count_stats *stats, struct seq_file *s)
{
	int i;

	seq_printf(s, "%s (%s)", pwrdm->name,
			pwrdm_state_names[pwrdm->state]);

	for (i = 0; i < PWRDM_MAX_PWRSTS; i++)
		seq_printf(s, ",%s:%d", pwrdm_state_names[i],
			stats->state[i]);

	seq_printf(s, ",RET-LOGIC-OFF:%d", stats->ret_logic_off);
	for (i = 0; i < pwrdm->banks; i++)
		seq_printf(s, ",RET-MEMBANK%d-OFF:%d", i + 1,
				stats->ret_mem_off[i]);

	seq_printf(s, "\n");

	return 0;
}

static int pwrdm_dbg_show_time_stats(struct powerdomain *pwrdm,
	struct powerdomain_time_stats *stats, struct seq_file *s)
{
	int i;
	u64 total = 0;

	seq_printf(s, "%s (%s)", pwrdm->name,
		pwrdm_state_names[pwrdm->state]);

	for (i = 0; i < 4; i++)
		total += stats->state[i];

	for (i = 0; i < 4; i++)
		seq_printf(s, ",%s:%lld (%lld%%)", pwrdm_state_names[i],
			stats->state[i],
			total ? div64_u64(stats->state[i] * 100, total) : 0);

	seq_printf(s, "\n");

	return 0;
}

static int pwrdm_dbg_show_counter(struct powerdomain *pwrdm, void *user)
{
	struct seq_file *s = (struct seq_file *)user;

	if (strcmp(pwrdm->name, "emu_pwrdm") == 0 ||
		strcmp(pwrdm->name, "wkup_pwrdm") == 0 ||
		strncmp(pwrdm->name, "dpll", 4) == 0)
		return 0;

	if (pwrdm->state != pwrdm_read_pwrst(pwrdm))
		printk(KERN_ERR "pwrdm state mismatch(%s) %d != %d\n",
			pwrdm->name, pwrdm->state, pwrdm_read_pwrst(pwrdm));

	pwrdm_dbg_show_count_stats(pwrdm, &pwrdm->count, s);

	return 0;
}

static int pwrdm_dbg_show_timer(struct powerdomain *pwrdm, void *user)
{
	struct seq_file *s = (struct seq_file *)user;

	if (strcmp(pwrdm->name, "emu_pwrdm") == 0 ||
		strcmp(pwrdm->name, "wkup_pwrdm") == 0 ||
		strncmp(pwrdm->name, "dpll", 4) == 0)
		return 0;

	pwrdm_state_switch(pwrdm);

	pwrdm_dbg_show_time_stats(pwrdm, &pwrdm->time, s);

	return 0;
}

static int pwrdm_dbg_show_last_counter(struct powerdomain *pwrdm, void *user)
{
	struct seq_file *s = (struct seq_file *)user;
	struct powerdomain_count_stats stats;
	int i;

	if (strcmp(pwrdm->name, "emu_pwrdm") == 0 ||
		strcmp(pwrdm->name, "wkup_pwrdm") == 0 ||
		strncmp(pwrdm->name, "dpll", 4) == 0)
		return 0;

	stats = pwrdm->count;
	for (i = 0; i < PWRDM_MAX_PWRSTS; i++)
		stats.state[i] -= pwrdm->last_count.state[i];
	for (i = 0; i < PWRDM_MAX_MEM_BANKS; i++)
		stats.ret_mem_off[i] -= pwrdm->last_count.ret_mem_off[i];
	stats.ret_logic_off -= pwrdm->last_count.ret_logic_off;

	pwrdm->last_count = pwrdm->count;

	pwrdm_dbg_show_count_stats(pwrdm, &stats, s);

	return 0;
}

static int pwrdm_dbg_show_last_timer(struct powerdomain *pwrdm, void *user)
{
	struct seq_file *s = (struct seq_file *)user;
	struct powerdomain_time_stats stats;
	int i;

	stats = pwrdm->time;
	for (i = 0; i < PWRDM_MAX_PWRSTS; i++)
		stats.state[i] -= pwrdm->last_time.state[i];

	pwrdm->last_time = pwrdm->time;

	pwrdm_dbg_show_time_stats(pwrdm, &stats, s);

	return 0;
}

#ifdef CONFIG_PM_FOOTPRINT
static ssize_t time_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	struct seq_file s;
	s.buf = buf;
	s.count = 0;
	s.size = 4096;
	pwrdm_for_each(pwrdm_dbg_show_timer, &s);
	return s.count;
}

static struct kobj_attribute time_attr =
	__ATTR(time, 0444, time_show, NULL);

#endif

static int pm_dbg_show_counters(struct seq_file *s, void *unused)
{
	pwrdm_for_each(pwrdm_dbg_show_counter, s);
	if (cpu_is_omap44xx())
		seq_printf(s, "DEVICE-OFF:%d\n", omap4_device_off_counter);
	clkdm_for_each(clkdm_dbg_show_counter, s);

	return 0;
}

static int pm_dbg_show_timers(struct seq_file *s, void *unused)
{
	pwrdm_for_each(pwrdm_dbg_show_timer, s);
	return 0;
}

static int pm_dbg_show_last_counters(struct seq_file *s, void *unused)
{
	pwrdm_for_each(pwrdm_dbg_show_last_counter, s);
	return 0;
}

static int pm_dbg_show_last_timers(struct seq_file *s, void *unused)
{
	pwrdm_for_each(pwrdm_dbg_show_last_timer, s);
	return 0;
}

static int pm_dbg_open(struct inode *inode, struct file *file)
{
	switch ((int)inode->i_private) {
	case DEBUG_FILE_COUNTERS:
		return single_open(file, pm_dbg_show_counters,
			&inode->i_private);
	case DEBUG_FILE_TIMERS:
		return single_open(file, pm_dbg_show_timers,
			&inode->i_private);
	case DEBUG_FILE_LAST_COUNTERS:
		return single_open(file, pm_dbg_show_last_counters,
			&inode->i_private);
	case DEBUG_FILE_LAST_TIMERS:
	default:
		return single_open(file, pm_dbg_show_last_timers,
			&inode->i_private);
	};
}

static int pm_dbg_reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, pm_dbg_show_regs, inode->i_private);
}

static const struct file_operations debug_fops = {
	.open           = pm_dbg_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static const struct file_operations debug_reg_fops = {
	.open           = pm_dbg_reg_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

int __init pm_dbg_regset_init(int reg_set)
{
	char name[2];

	if (!pm_dbg_init_done)
		pm_dbg_init();

	if (reg_set < 1 || reg_set > PM_DBG_MAX_REG_SETS ||
		pm_dbg_reg_set[reg_set-1] != NULL)
		return -EINVAL;

	pm_dbg_reg_set[reg_set-1] =
		kmalloc(pm_dbg_get_regset_size(), GFP_KERNEL);

	if (pm_dbg_reg_set[reg_set-1] == NULL)
		return -ENOMEM;

	if (pm_dbg_dir != NULL) {
		sprintf(name, "%d", reg_set);

		(void) debugfs_create_file(name, S_IRUGO,
			pm_dbg_dir, (void *)reg_set, &debug_reg_fops);
	}

	return 0;
}

static int pwrdm_suspend_get(void *data, u64 *val)
{
	int ret = -EINVAL;

	if (cpu_is_omap34xx())
		ret = omap3_pm_get_suspend_state((struct powerdomain *)data);
	*val = ret;

	if (ret >= 0)
		return 0;
	return *val;
}

static int pwrdm_suspend_set(void *data, u64 val)
{
	if (cpu_is_omap34xx())
		return omap3_pm_set_suspend_state(
			(struct powerdomain *)data, (int)val);
	return -EINVAL;
}

DEFINE_SIMPLE_ATTRIBUTE(pwrdm_suspend_fops, pwrdm_suspend_get,
			pwrdm_suspend_set, "%llu\n");

#ifdef CONFIG_PM_ADVANCED_DEBUG
static bool is_addr_valid()
{
	int saved_reg_addr_max = 0;
	/* Only for OMAP4 for the timebeing */
	if (!cpu_is_omap44xx())
		return false;

	saved_reg_num = (saved_reg_num > PM_DEBUG_MAX_SAVED_REGS) ?
		PM_DEBUG_MAX_SAVED_REGS : saved_reg_num;

	saved_reg_addr_max = saved_reg_addr + (saved_reg_num * 4) - 1;

	if (saved_reg_addr >= PM_DEBUG_PRM_MIN &&
		saved_reg_addr_max <= PM_DEBUG_PRM_MAX)
			return true;
	if (saved_reg_addr >= PM_DEBUG_CM1_MIN &&
		saved_reg_addr_max <= PM_DEBUG_CM1_MAX)
			return true;
	if (saved_reg_addr >= PM_DEBUG_CM2_MIN &&
		saved_reg_addr_max <= PM_DEBUG_CM2_MAX)
			return true;
	return false;
}

void omap4_pm_suspend_save_regs()
{
	int i = 0;
	if (!saved_reg_num || !is_addr_valid())
		return;

	saved_reg_num_used = saved_reg_num;

	for (i = 0; i < saved_reg_num; i++) {
		saved_reg_buff[1][i] = omap_readl(saved_reg_addr + (i*4));
		saved_reg_buff[0][i] = saved_reg_addr + (i*4);
	}
	return;
}
#endif

static int __init pwrdms_setup(struct powerdomain *pwrdm, void *dir)
{
	int i;
	s64 t;
	struct dentry *d;

	t = sched_clock();

	for (i = 0; i < 4; i++)
		pwrdm->time.state[i] = 0;

	pwrdm->timer = t;

	if (strncmp(pwrdm->name, "dpll", 4) == 0)
		return 0;

	d = debugfs_create_dir(pwrdm->name, (struct dentry *)dir);

	(void) debugfs_create_file("suspend", S_IRUGO|S_IWUSR, d,
			(void *)pwrdm, &pwrdm_suspend_fops);

	return 0;
}

static int option_get(void *data, u64 *val)
{
	u32 *option = data;

	if (option == &enable_off_mode) {
		enable_off_mode = off_mode_enabled;
	}

	*val = *option;
#ifdef CONFIG_PM_ADVANCED_DEBUG
	if (option == &saved_reg_addr) {
		int i;
		for (i = 0; i < saved_reg_num_used; i++)
			pr_info(" %x = %x\n", saved_reg_buff[0][i],
				saved_reg_buff[1][i]);
	}
#endif

	return 0;
}

static int option_set(void *data, u64 val)
{
	u32 *option = data;

	if (option == &wakeup_timer_milliseconds && val >= 1000)
		return -EINVAL;

	if (cpu_is_omap443x() && omap_type() == OMAP2_DEVICE_TYPE_GP)
		*option = 0;
	else
		*option = val;

	if (option == &enable_off_mode) {
		if (val)
			omap_pm_enable_off_mode();
		else
			omap_pm_disable_off_mode();
		if (cpu_is_omap34xx())
			omap3_pm_off_mode_enable(val);
	}

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pm_dbg_option_fops, option_get, option_set, "%llu\n");

static int __init pm_dbg_init(void)
{
	int i;
	struct dentry *d;
	char name[2];
#ifdef CONFIG_PM_FOOTPRINT
	int error;
#endif

	if (pm_dbg_init_done)
		return 0;

	if (cpu_is_omap34xx()) {
		pm_dbg_reg_modules = omap3_pm_reg_modules;
	} else if (cpu_is_omap44xx()) {
		/* Allow pm_dbg_init on OMAP4. */
	} else {
		printk(KERN_ERR "%s: only OMAP3 supported\n", __func__);
		return -ENODEV;
	}

	d = debugfs_create_dir("pm_debug", NULL);
	if (IS_ERR(d))
		return PTR_ERR(d);

	(void) debugfs_create_file("count", S_IRUGO,
		d, (void *)DEBUG_FILE_COUNTERS, &debug_fops);
	(void) debugfs_create_file("time", S_IRUGO,
		d, (void *)DEBUG_FILE_TIMERS, &debug_fops);
	(void) debugfs_create_file("last_count", S_IRUGO,
		d, (void *)DEBUG_FILE_LAST_COUNTERS, &debug_fops);
	(void) debugfs_create_file("last_time", S_IRUGO,
		d, (void *)DEBUG_FILE_LAST_TIMERS, &debug_fops);

	pwrdm_for_each(pwrdms_setup, (void *)d);

	if (cpu_is_omap44xx())
		goto skip_reg_debufs;

	pm_dbg_dir = debugfs_create_dir("registers", d);
	if (IS_ERR(pm_dbg_dir))
		return PTR_ERR(pm_dbg_dir);

	(void) debugfs_create_file("current", S_IRUGO,
		pm_dbg_dir, (void *)0, &debug_reg_fops);

	for (i = 0; i < PM_DBG_MAX_REG_SETS; i++)
		if (pm_dbg_reg_set[i] != NULL) {
			sprintf(name, "%d", i+1);
			(void) debugfs_create_file(name, S_IRUGO,
				pm_dbg_dir, (void *)(i+1), &debug_reg_fops);

		}

	(void) debugfs_create_file("sleep_while_idle", S_IRUGO | S_IWUSR, d,
				   &sleep_while_idle, &pm_dbg_option_fops);

skip_reg_debufs:
#ifdef CONFIG_OMAP_ALLOW_OSWR
	(void) debugfs_create_file("enable_off_mode", S_IRUGO | S_IWUSR, d,
				   &enable_off_mode, &pm_dbg_option_fops);
#endif
	(void) debugfs_create_file("wakeup_timer_seconds", S_IRUGO | S_IWUSR, d,
				   &wakeup_timer_seconds, &pm_dbg_option_fops);
	(void) debugfs_create_file("wakeup_timer_milliseconds",
			S_IRUGO | S_IWUSR, d, &wakeup_timer_milliseconds,
			&pm_dbg_option_fops);
	(void) debugfs_create_file("enable_regulator_dump", S_IRUGO | S_IWUSR,
			d, &enable_regulator_dump, &pm_dbg_option_fops);

#ifdef CONFIG_PM_ADVANCED_DEBUG
	(void) debugfs_create_file("saved_reg_show",
			S_IRUGO | S_IWUSR, d, &saved_reg_addr,
			&pm_dbg_option_fops);
	debugfs_create_u32("saved_reg_addr",  S_IRUGO | S_IWUGO, d,
				&saved_reg_addr);
	debugfs_create_u32("saved_reg_num",  S_IRUGO | S_IWUGO, d,
				 &saved_reg_num);
#endif
	pm_dbg_init_done = 1;

#ifdef CONFIG_PM_FOOTPRINT
	error = sysfs_create_file(power_kobj, &time_attr.attr);
	if (error)
		printk(KERN_ERR "sysfs_create_file failed: %d\n", error);
#endif

	return 0;
}
arch_initcall(pm_dbg_init);

#endif
