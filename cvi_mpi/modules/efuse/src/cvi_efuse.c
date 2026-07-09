#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/prctl.h>

#include <cvi_sys.h>
#include <cvi_efuse.h>
#include <cvi_errno.h>
#include <errno.h>

#define CVI_EFUSE_CHIP_SN_SIZE 8
#define CVI_EFUSE_CHIP_SN_ADDR 0x0C

CVI_S32 CVI_MISC_GetChipSNSize(CVI_U32 *pu32SNSize)
{
        if (pu32SNSize)
                *pu32SNSize = CVI_EFUSE_CHIP_SN_SIZE;

        return CVI_SUCCESS;
}

CVI_S32 CVI_MISC_GetChipSN(CVI_U8 *pu8SN, CVI_U32 u32SNSize)
{
        FILE *fp;

        if (!pu8SN)
                return CVI_ERR_SYS_ILLEGAL_PARAM;

        fp = fopen("/sys/class/cvi-efuse/base_efuse_shadow", "r");
        if (!fp) {
                CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open efuse file.\n");
                return CVI_ERR_SYS_NOTREADY;
        }

        if (u32SNSize > CVI_EFUSE_CHIP_SN_SIZE)
                u32SNSize = CVI_EFUSE_CHIP_SN_SIZE;

        fseek(fp, CVI_EFUSE_CHIP_SN_ADDR, SEEK_SET);
        if (fread(pu8SN, 1, u32SNSize, fp) != u32SNSize) {
                CVI_TRACE_SYS(CVI_DBG_ERR, "fread failed\n");
                return CVI_FAILURE;
        }

        fclose(fp);

        return CVI_SUCCESS;
}

// ===========================================================================
// EFUSE API
// ===========================================================================
static struct _CVI_EFUSE_AREA_S {
	CVI_U32 addr;
	CVI_U32 size;
} cvi_efuse_area[] = { [CVI_EFUSE_AREA_USER] = { 0x40, 40 },
		       [CVI_EFUSE_AREA_DEVICE_ID] = { 0x8c, 8 },
		       [CVI_EFUSE_AREA_HASH0_PUBLIC] = { 0xA8, 32 },
		       [CVI_EFUSE_AREA_LOADER_EK] = { 0xD8, 16 },
		       [CVI_EFUSE_AREA_DEVICE_EK] = { 0xE8, 16 },
		       [CVI_EFUSE_AREA_CHIP_SN] = { 0x0C, 8 } };

static struct _CVI_EFUSE_LOCK_S {
	CVI_U32 wlock_shift;
	CVI_U32 rlock_shift;
} cvi_efuse_lock[] = { [CVI_EFUSE_LOCK_HASH0_PUBLIC] = { 0, 8 },
		       [CVI_EFUSE_LOCK_LOADER_EK] = { 4, 12 },
		       [CVI_EFUSE_LOCK_DEVICE_EK] = { 6, 14 } };

static struct _CVI_EFUSE_USER_S {
	CVI_U32 addr;
	CVI_U32 size;
} cvi_efuse_user[] = {
	{ 0x40, 4 },
	{ 0x48, 4 },
	{ 0x50, 4 },
	{ 0x58, 4 },
	{ 0x60, 4 },
	{ 0x68, 4 },
	{ 0x70, 4 },
	{ 0x78, 4 },
	{ 0x80, 4 },
	{ 0x88, 4 },
};

#define CVI_EFUSE_TOTAL_SIZE 0x100

#define CVI_EFUSE_LOCK_ADDR 0xF8
#define CVI_EFUSE_SECURE_CONF_ADDR 0xA0
#define CVI_EFUSE_SCS_ENABLE_SHIFT 0
#define CVI_EFUSE_SW_INFO 0x2C
#define CVI_EFUSE_CUSTOMER_ADDR 0x4

#define CVI_EFUSE_PATH_PROG "/sys/class/cvi-efuse/base_efuse_prog"
#define CVI_EFUSE_PATH_SHADOW "/sys/class/cvi-efuse/base_efuse_shadow"

CVI_S32 CVI_EFUSE_GetSize(CVI_EFUSE_AREA_E area, CVI_U32 *size)
{
	if (area >= ARRAY_SIZE(cvi_efuse_area) ||
	    cvi_efuse_area[area].size == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "area (%d) is not found\n", area);
		return CVI_ERR_SYS_NOMEM;
	}

	if (size)
		*size = cvi_efuse_area[area].size;

	return 0;
}

static CVI_S32 _CVI_EFUSE_Read(CVI_U32 addr, void *buf, CVI_U32 buf_size)
{
	CVI_S32 ret = -1;

	CVI_TRACE_SYS(CVI_DBG_DEBUG, "addr=0x%02x\n", addr);

	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	FILE *fp = fopen(CVI_EFUSE_PATH_SHADOW, "r");

	if (!fp) {
		ret = errno;
		CVI_TRACE_SYS(CVI_DBG_ERR, "fopen(%s)\n",
			      CVI_EFUSE_PATH_SHADOW);
		return ret;
	}

	fseek(fp, addr, SEEK_SET);
	ret = fread(buf, buf_size, 1, fp);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ret=%d\n", ret);
	}
	fclose(fp);

	return ret;
}

static CVI_S32 _CVI_EFUSE_Write(CVI_U32 addr, const void *buf, CVI_U32 buf_size)
{
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "addr=0x%02x\n", addr);

	char cmd[64];
	CVI_U32 value;
	CVI_U32 aligned_addr = addr;
	CVI_U32 aligned_size = buf_size;
	void *aligned_buf = NULL;
	size_t i;

	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	if (aligned_addr % 4) {
		aligned_addr -= aligned_addr % 4;
	}

	if (aligned_size % 4) {
		aligned_size += 4 - aligned_size % 4;
	}

	aligned_buf = malloc(aligned_size);
	memset(aligned_buf, 0, aligned_size);
	memcpy((CVI_U8 *)aligned_buf + (addr - aligned_addr), buf, buf_size);

	for (i = 0; i < aligned_size; i += 4) {
		memcpy(&value, (CVI_U8 *)aligned_buf + i, sizeof(value));
		snprintf(cmd, sizeof(cmd), "0x%04zx=0x%08x", addr + i, value);
		CVI_TRACE_SYS(CVI_DBG_DEBUG, "cmd=%s\n", cmd);

		FILE *fp = fopen(CVI_EFUSE_PATH_PROG, "w");
		int ret;

		if (!fp) {
			ret = errno;
			CVI_TRACE_SYS(CVI_DBG_ERR, "fopen(%s)\n",
				      CVI_EFUSE_PATH_PROG);
			return ret;
		}

		ret = fwrite(cmd, strlen(cmd), 1, fp);
		if (ret < 0) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "ret=%d\n", ret);
		}
		fclose(fp);
	}

	return 0;
}

CVI_S32 CVI_EFUSE_Read(CVI_EFUSE_AREA_E area, CVI_U8 *buf, CVI_U32 buf_size)
{
	CVI_U32 user_size = cvi_efuse_area[CVI_EFUSE_AREA_USER].size;
	CVI_U8 user[user_size], *p;
	CVI_S32 ret;
	size_t i;

	if (area >= ARRAY_SIZE(cvi_efuse_area) ||
	    cvi_efuse_area[area].size == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "area (%d) is not found\n", area);
		return CVI_ERR_SYS_NOMEM;
	}

	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	memset(buf, 0, buf_size);

	if (buf_size > cvi_efuse_area[area].size)
		buf_size = cvi_efuse_area[area].size;

	if (area != CVI_EFUSE_AREA_USER)
		return _CVI_EFUSE_Read(cvi_efuse_area[area].addr, buf,
				       buf_size);

	memset(user, 0, user_size);

	p = user;
	for (i = 0; i < ARRAY_SIZE(cvi_efuse_user); i++) {
		ret = _CVI_EFUSE_Read(cvi_efuse_user[i].addr, p,
				      cvi_efuse_user[i].size);
		if (ret < 0)
			return ret;
		p += cvi_efuse_user[i].size;
	}

	memcpy(buf, user, buf_size);

	return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_Write(CVI_EFUSE_AREA_E area, const CVI_U8 *buf,
			CVI_U32 buf_size)
{
	CVI_U32 user_size = cvi_efuse_area[CVI_EFUSE_AREA_USER].size;
	CVI_U8 user[user_size], *p;
	CVI_S32 ret;
	size_t i;

	if (area >= ARRAY_SIZE(cvi_efuse_area) ||
	    cvi_efuse_area[area].size == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "area (%d) is not found\n", area);
		return CVI_ERR_SYS_NOMEM;
	}
	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	if (buf_size > cvi_efuse_area[area].size)
		buf_size = cvi_efuse_area[area].size;

	if (area != CVI_EFUSE_AREA_USER) {
		return _CVI_EFUSE_Write(cvi_efuse_area[area].addr, buf,
					buf_size);
	}

	memset(user, 0, user_size);
	memcpy(user, buf, buf_size);

	p = user;
	for (i = 0; i < ARRAY_SIZE(cvi_efuse_user); i++) {
		ret = _CVI_EFUSE_Write(cvi_efuse_user[i].addr, p,
				       cvi_efuse_user[i].size);
		if (ret < 0)
			return ret;
		p += cvi_efuse_user[i].size;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_EnableSecureBoot(void)
{
	CVI_U32 value = 0x3 << CVI_EFUSE_SCS_ENABLE_SHIFT;

	return _CVI_EFUSE_Write(CVI_EFUSE_SECURE_CONF_ADDR, &value,
				sizeof(value));
}

CVI_S32 CVI_EFUSE_IsSecureBootEnabled(void)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;

	ret = _CVI_EFUSE_Read(CVI_EFUSE_SECURE_CONF_ADDR, &value,
			      sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	value &= 0x3 << CVI_EFUSE_SCS_ENABLE_SHIFT;
	return !!value;
}

/**
 * @brief Get Chip ID by reading register via devmem.
 *
 * @return CVI_U32, the chip id.
 */
static unsigned int CVI_MISC_GetChipIdFromDevmem(void)
{
	CVI_U32 chip = 0;
	FILE *fp = popen("devmem 0x0300008c", "r");

	if (fp) {
		char buf[64];

		if (fgets(buf, sizeof(buf), fp)) {
			chip = strtoul(buf, NULL, 0);
		}
		pclose(fp);
	}
	return chip;
}

CVI_S32 CVI_EFUSE_EnableFastBoot(void)
{
	CVI_U32 value = 0, data;
	CVI_S32 ret = 0;
	CVI_U32 chip = 0;

	if (CVI_EFUSE_IsFastBootEnabled() == CVI_SUCCESS) {
		printf("Fast Boot is already enabled.\n");
		return CVI_SUCCESS;
	}

	chip = CVI_MISC_GetChipIdFromDevmem();
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "chip id=%x\n", chip);

	ret = _CVI_EFUSE_Read(CVI_EFUSE_SW_INFO, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	data = (value & (0x3 << 22)) >> 22;
	if (data > 0x1)
		return CVI_FAILURE;

	data = (value & (0x3 << 24)) >> 24;
	if (data > 0x1)
		return CVI_FAILURE;

	data = (value & (0x3 << 26)) >> 26;
	if (data > 0x1)
		return CVI_FAILURE;

	ret = _CVI_EFUSE_Read(CVI_EFUSE_CUSTOMER_ADDR, &value, sizeof(value));
        CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
        if (ret < 0)
                return ret;

	if (((chip & 0xFFF0F) == 0x1810C || (chip & 0xFFF0F) == 0x1840C) && ((chip >> 4) & 0xF) <= 3) { // 181XC AND 184XC (X <= 3)
		value |= 0x1E1E64; // CV181X-AUX0 AND CV184X-AUX0
		if (value != 0x1E1E64) {
			printf("CUSTOMER value=%u\n", value);
			return CVI_FAILURE;
		}
	} else if (((chip & 0xFFF0F) == 0x1800C || (chip & 0xFFF0F) == 0x1800B)
					&& ((chip >> 4) & 0xF) <= 3) { // CV180X (X <= 3)
		value |= 0x1E1564; // CV180X-AUX0
		if (value != 0x1E1564) {
			printf("CUSTOMER value=%u\n", value);
			return CVI_FAILURE;
		}
	} else {
		value |= 0x1; // USB_ID
		if (value != 0x1) {
			printf("CUSTOMER value=%u\n", value);
			return CVI_FAILURE;
		}
	}

	ret = _CVI_EFUSE_Write(CVI_EFUSE_CUSTOMER_ADDR, &value, sizeof(value));
	if (ret < 0)
		return ret;

	// set sd dl button
	value = (0x1 << 22);
	value |= (0x1 << 24);
	value |= (0x1 << 26);

	return _CVI_EFUSE_Write(CVI_EFUSE_SW_INFO, &value, sizeof(value));
}

CVI_S32 CVI_EFUSE_IsFastBootEnabled(void)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;
	CVI_U32 chip = 0;

	chip = CVI_MISC_GetChipIdFromDevmem();
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "chip id=%x\n", chip);

	ret = _CVI_EFUSE_Read(CVI_EFUSE_SW_INFO, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	if (((value & (0x3 << 22)) != (0x1 << 22))
		&& ((value & (0x3 << 24)) != (0x1 << 24))
		&& ((value & (0x3 << 26)) != (0x1 << 26))) {
		CVI_TRACE_SYS(CVI_DBG_DEBUG, "sw_info isn't fastboot config\n");
		return CVI_FAILURE;
	}

	ret = _CVI_EFUSE_Read(CVI_EFUSE_CUSTOMER_ADDR, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	if (((chip & 0xFFF0F) == 0x1810C || (chip & 0xFFF0F) == 0x1840C) && ((chip >> 4) & 0xF) <= 3) { // 181XC AND 184XC (X <= 3)
		if (value == 0x1E1E64)
			return CVI_SUCCESS; // CV181X-AUX0 AND CV184X-AUX0
		else
			return CVI_FAILURE;
	}  else {
		if (value == 0x1)
			return CVI_SUCCESS; // USB_ID
		else
			return CVI_FAILURE;
	}

	return CVI_FAILURE;
}

CVI_S32 CVI_EFUSE_Lock(CVI_EFUSE_LOCK_E lock)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	value = 0x3 << cvi_efuse_lock[lock].wlock_shift;
	ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	if (ret < 0)
		return ret;

	value = 0x3 << cvi_efuse_lock[lock].rlock_shift;
	ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	return ret;
}

CVI_S32 CVI_EFUSE_IsLocked(CVI_EFUSE_LOCK_E lock)
{
	CVI_S32 ret = 0;
	CVI_U32 value = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	ret = _CVI_EFUSE_Read(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	value &= 0x3 << cvi_efuse_lock[lock].wlock_shift;
	return !!value;
}

CVI_S32 CVI_EFUSE_LockWrite(CVI_EFUSE_LOCK_E lock)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	value = 0x3 << cvi_efuse_lock[lock].wlock_shift;
	ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	return ret;
}

CVI_S32 CVI_EFUSE_IsWriteLocked(CVI_EFUSE_LOCK_E lock)
{
	CVI_S32 ret = 0;
	CVI_U32 value = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	ret = _CVI_EFUSE_Read(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	value &= 0x3 << cvi_efuse_lock[lock].wlock_shift;
	return !!value;
}


