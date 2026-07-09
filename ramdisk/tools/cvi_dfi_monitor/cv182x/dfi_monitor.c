#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "dfi_monitor.h"
#include "devmem.h"
#include "argvtoint.h"

#define DDR_TOP_BASE		0x0800A000
#define DDRC_BASE		0x08004000
#define CLKMON_MEANS_CLKIN	0x03000140

#define ERR	printf
#define DEBUG	//printf
#define INFO	printf

uint64_t dfi_r0_mw_cnt;		//dfi_r0_mw_cnt[31:0]
uint64_t dfi_r0_wr2_cnt;	//dfi_r0_wr2_cnt[31:0]
uint64_t dfi_r0_wr_cnt;		//dfi_r0_wr_cnt[31:0]
uint64_t dfi_r0_rd2_cnt;	//dfi_r0_rd2_cnt[31:0]
uint64_t dfi_r0_rd_cnt;		//dfi_r0_rd_cnt[31:0]
uint64_t dfi_r0_act_cnt;	//dfi_r0_act_cnt[31:0]
uint64_t dfi_w2r_cnt;		//dfi_w2r_cnt[31:0]
uint64_t dfi_r0_preab_cnt;	//dfi_r0_preab_cnt[31:0]
// uint64_t dfi_r1_preab_cnt;	//dfi_r1_preab_cnt[31:0]
uint64_t dfi_r0_prepb_cnt;	//dfi_r0_prepb_cnt[31:0]
// uint64_t dfi_r1_prepb_cnt;	//dfi_r1_prepb_cnt[31:0]
uint64_t dfi_rpt_cycle_cnt;	//dfi_rpt_cycle_cnt[31:0]

static uint32_t get_ddr_type(void)
{
	uint32_t reg_mstr;

	// read MSTR for ddr type
	reg_mstr = devmem_readl(DDRC_BASE + 0x00);
	if (reg_mstr & 0x1) {
		DEBUG("DDR3\n");
		return 0; // DDR3
	} else if (reg_mstr & 0x10) {
		DEBUG("DDR4\n");
		return 1; // DDR4
	} else if (reg_mstr & 0x20) {
		DEBUG("LPDDR4\n");
		return 2; // LPDDR4
	}

	ERR("Wrong DDR type (0x%08X)\n", reg_mstr);
	return 0;
}

static uint32_t get_ddr_bus_width(void)
{
	uint32_t reg_mstr;

	// read MSTR
	reg_mstr = devmem_readl(DDRC_BASE + 0x00);

	if (((reg_mstr >> 12) & 0x3) == 0) {
		DEBUG("Bus width is X32\n");
		return 32;
	} else if (((reg_mstr >> 12) & 0x3) == 1) {
		DEBUG("Bus width is X16\n");
		return 16;
	}
	ERR("Wrong DDR bus width (0x%08X)\n", reg_mstr);
	return 0;
}

static void dfi_monitor_start(uint32_t ddr_type)
{
	// dfi_mon_en[0], dfi_mon_tphy_wrdata[7:4], dfi_mon_mem_type[9:8]
	// dfi_mon_mem_type[9:8] = 0 (DDR3)
	// dfi_mon_mem_type[9:8] = 1 (DDR4)
	// dfi_mon_mem_type[9:8] = 2 (LPDDR4)
	devmem_writel(DDR_TOP_BASE + 0x40, 0x001 | (ddr_type << 8));
}

static void dfi_monitor_end(void)
{
	devmem_writel(DDR_TOP_BASE + 0x40, 0x0); //dfi_mon_en[0], dfi_mon_tphy_wrdata[7:4], dfi_mon_mem_type[9:8]
}

static void dfi_monitor_rpt_trig(void)
{
	devmem_writel(DDR_TOP_BASE + 0x44, 0x1); //dfi_mon_rpt_trig[0]
}

static void dfi_monitor_readrpt(void)
{
	dfi_r0_mw_cnt += devmem_readl(DDR_TOP_BASE + 0x68); //dfi_r0_mw_cnt[31:0]
	dfi_r0_wr2_cnt += devmem_readl(DDR_TOP_BASE + 0x60); //dfi_r0_wr2_cnt[31:0]
	dfi_r0_wr_cnt += devmem_readl(DDR_TOP_BASE + 0x58); //dfi_r0_wr_cnt[31:0]
	dfi_r0_rd2_cnt += devmem_readl(DDR_TOP_BASE + 0x50); //dfi_r0_rd2_cnt[31:0]
	dfi_r0_rd_cnt += devmem_readl(DDR_TOP_BASE + 0x48); //dfi_r0_rd_cnt[31:0]
	dfi_r0_act_cnt += devmem_readl(DDR_TOP_BASE + 0x38); //dfi_r0_act_cnt[31:0]
	dfi_w2r_cnt += devmem_readl(DDR_TOP_BASE + 0xB8); //dfi_w2r_cnt[31:0]
	dfi_r0_preab_cnt += devmem_readl(DDR_TOP_BASE + 0x70); //dfi_r0_preab_cnt[31:0]
	// dfi_r1_preab_cnt += devmem_readl(DDR_TOP_BASE + 0x74); //dfi_r1_preab_cnt[31:0]
	dfi_r0_prepb_cnt += devmem_readl(DDR_TOP_BASE + 0x78); //dfi_r0_prepb_cnt[31:0]
	// dfi_r1_prepb_cnt += devmem_readl(DDR_TOP_BASE + 0x7C); //dfi_r1_prepb_cnt[31:0]
	dfi_rpt_cycle_cnt += devmem_readl(DDR_TOP_BASE + 0xBC); //dfi_rpt_cycle_cnt[31:0]

	DEBUG("dfi_r0_mw_cnt=0x%08X\n", dfi_r0_mw_cnt);
	DEBUG("dfi_r0_wr2_cnt=0x%08X\n", dfi_r0_wr2_cnt);
	DEBUG("dfi_r0_wr_cnt=0x%08X\n", dfi_r0_wr_cnt);
	DEBUG("dfi_r0_rd2_cnt=0x%08X\n", dfi_r0_rd2_cnt);
	DEBUG("dfi_r0_rd_cnt=0x%08X\n", dfi_r0_rd_cnt);
	DEBUG("dfi_r0_act_cnt=0x%08X\n", dfi_r0_act_cnt);
	DEBUG("dfi_w2r_cnt=0x%08X\n", dfi_w2r_cnt);
	DEBUG("dfi_r0_preab_cnt=0x%08X\n", dfi_r0_preab_cnt);
	// DEBUG("dfi_r1_preab_cnt=0x%08X\n", dfi_r1_preab_cnt);
	DEBUG("dfi_r0_prepb_cnt=0x%08X\n", dfi_r0_prepb_cnt);
	// DEBUG("dfi_r1_prepb_cnt=0x%08X\n", dfi_r1_prepb_cnt);
	DEBUG("dfi_rpt_cycle_cnt=0x%08X\n", dfi_rpt_cycle_cnt);
}

static uint32_t meas_clk_in_mhz(uint32_t meas_sel)
{
	uint32_t clk_meas;
	uint32_t meas_count;
	uint32_t clk_in_mhz;

	// meas_en[0] = 0
	devmem_writel(CLKMON_MEANS_CLKIN, 0 | meas_sel << 3);
	// meas_en[0] = 1
	devmem_writel(CLKMON_MEANS_CLKIN, 1 | meas_sel << 3);

	clk_meas = devmem_readl(CLKMON_MEANS_CLKIN);
	meas_count = (clk_meas >> 8) & 0x3FFFF;
	clk_in_mhz = meas_count / 10 * 2;

	// disable clk meas
	devmem_writel(CLKMON_MEANS_CLKIN, 0);

	return clk_in_mhz;
}

static void dfi_calc_bw(uint32_t meas_time_ms,
			uint32_t ddr_type,
			uint32_t ddr_bus_width)
{
	uint32_t ddr_clk;
	uint64_t byte_count;
	uint64_t cycle_count;
	double bw;
	double ideal_bw;
	double efficiency;

	ddr_clk = meas_clk_in_mhz(8);
	DEBUG("ddr_clk = %d MHz\n", ddr_clk);

	// byte count
	byte_count = dfi_r0_wr_cnt;
	byte_count += dfi_r0_rd_cnt;
	byte_count += dfi_r0_wr2_cnt;
	byte_count += dfi_r0_rd2_cnt;
	byte_count *= (ddr_bus_width / 8);
	byte_count *= (ddr_type == 2 ? 16 : 8);
	DEBUG("byte_count = %u\n", byte_count);

	// cycle count
	cycle_count = dfi_rpt_cycle_cnt;
	DEBUG("cycle_count = %u\n", cycle_count);

	// bw
	bw =  (float) (byte_count * ddr_clk / 2) / (float) cycle_count;
	INFO("bw = %.2f MB/s\n", bw);

	// ideal bw
	ideal_bw = (cycle_count * 4);
	ideal_bw *= (ddr_bus_width / 8);
	ideal_bw /= (meas_time_ms * 1000);
	INFO("ideal_bw = %.2f MB/s\n", ideal_bw);

	// efficiency(%)
	efficiency = bw * 100 / ideal_bw;
	INFO("efficiency = %.2f %%\n", efficiency);
}

int main(int argc, char *argv[])
{
	uint32_t meas_time_ms;
	uint32_t remain;
	uint32_t ddr_type;
	uint32_t ddr_bus_width;

	if (argc >= 2) {
		meas_time_ms = StrToInt(argv[1]);
		if (meas_time_ms == 0)
			meas_time_ms = 1000;
	} else {
		meas_time_ms = 1000; //1 sec
	}
	INFO("measure time = %d ms\n", meas_time_ms);

	ddr_type = get_ddr_type();
	INFO("DDR Type = %d (0:DDR3, 1:DDR4, 2:LPDDR4)\n", ddr_type);

	ddr_bus_width = get_ddr_bus_width();
	INFO("Bus width = %d\n", ddr_bus_width);

	remain = meas_time_ms;
	while (remain) {
		dfi_monitor_start(ddr_type);
		usleep(1000 * (remain > 500 ? 500 : remain));
		dfi_monitor_end();
		dfi_monitor_rpt_trig();
		dfi_monitor_readrpt();
		remain -= (remain > 500 ? 500 : remain);
	}

	dfi_calc_bw(meas_time_ms, ddr_type, ddr_bus_width);

	return 0;
}
