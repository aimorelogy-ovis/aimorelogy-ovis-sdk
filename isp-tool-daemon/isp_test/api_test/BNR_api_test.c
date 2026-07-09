
// auto generate code, don't modify
// auto generate code, don't modify
// auto generate code, don't modify

#include <stdlib.h>
#include "api_test_main.h"

#define TEST_FAIL(fmt, arg...) printf("[ISP_TEST_FAIL] %s,%d: " fmt, __FUNCTION__, __LINE__, ##arg)

static long long rand_range(long long a, long long b)
{
	return rand() % (b - a + 1) + a;
}

static int test_BNR_BNRAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.Enable = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.Enable = rand_range(0, 1);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// restore default value
	attr.Enable = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_Enable(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_Enable);
}

static int test_BNR_BNRAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.enOpType = OP_TYPE_MANUAL;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// restore default value
	attr.enOpType = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_enOpType(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_enOpType);
}

static int test_BNR_BNRAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.UpdateInterval = 255;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.UpdateInterval = rand_range(0, 255);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// restore default value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_UpdateInterval);
}

static int test_BNR_BNRAttr_PreFilterEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.PreFilterEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.PreFilterEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.PreFilterEnable = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.PreFilterEnable = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.PreFilterEnable = rand_range(0, 1);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// restore default value
	attr.PreFilterEnable = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_PreFilterEnable(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_PreFilterEnable);
}

static int test_BNR_BNRAttr_PreFilterModeSel(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.PreFilterModeSel != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.PreFilterModeSel);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.PreFilterModeSel = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.PreFilterModeSel = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.PreFilterModeSel = rand_range(0, 1);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// restore default value
	attr.PreFilterModeSel = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_PreFilterModeSel(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_PreFilterModeSel);
}

static int test_BNR_BNRAttr_FilterKsize_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FilterKsize != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.FilterKsize);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FilterKsize = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.FilterKsize = 1;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.FilterKsize = rand_range(0, 1);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.FilterKsize = rand_range(2, 255);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.FilterKsize = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_FilterKsize_manual(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_FilterKsize_manual);
}

static int test_BNR_BNRAttr_FilterKsize_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.FilterKsize, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FilterKsize[j] = 0;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FilterKsize[j] = 1;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FilterKsize[j] = rand_range(0, 1);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FilterKsize[j] = rand_range(2, 255);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.FilterKsize, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_FilterKsize_auto(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_FilterKsize_auto);
}

static int test_BNR_BNRAttr_PreFilterStdThr1_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.PreFilterStdThr1 != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.PreFilterStdThr1);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.PreFilterStdThr1 = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.PreFilterStdThr1 = 1023;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.PreFilterStdThr1 = rand_range(0, 1023);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.PreFilterStdThr1 = rand_range(1024, 65535);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.PreFilterStdThr1 = 4;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_PreFilterStdThr1_manual(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_PreFilterStdThr1_manual);
}

static int test_BNR_BNRAttr_PreFilterStdThr1_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.PreFilterStdThr1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr1[j] = 0;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr1[j] = 1023;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr1[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr1[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.PreFilterStdThr1, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_PreFilterStdThr1_auto(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_PreFilterStdThr1_auto);
}

static int test_BNR_BNRAttr_PreFilterStdThr2_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.PreFilterStdThr2 != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.PreFilterStdThr2);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.PreFilterStdThr2 = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.PreFilterStdThr2 = 1023;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.PreFilterStdThr2 = rand_range(0, 1023);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.PreFilterStdThr2 = rand_range(1024, 65535);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.PreFilterStdThr2 = 16;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_PreFilterStdThr2_manual(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_PreFilterStdThr2_manual);
}

static int test_BNR_BNRAttr_PreFilterStdThr2_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.PreFilterStdThr2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr2[j] = 0;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr2[j] = 1023;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr2[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.PreFilterStdThr2[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.PreFilterStdThr2, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_PreFilterStdThr2_auto(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_PreFilterStdThr2_auto);
}

static int test_BNR_BNRAttr_GussianKernelWt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.GussianKernelWt[0] != 31) {
		TEST_FAIL("test default value: 31 != %d\n", (int)attr.stManual.GussianKernelWt[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[1] != 23) {
		TEST_FAIL("test default value: 23 != %d\n", (int)attr.stManual.GussianKernelWt[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[2] != 17) {
		TEST_FAIL("test default value: 17 != %d\n", (int)attr.stManual.GussianKernelWt[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[3] != 9) {
		TEST_FAIL("test default value: 9 != %d\n", (int)attr.stManual.GussianKernelWt[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[4] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.GussianKernelWt[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[5] != 3) {
		TEST_FAIL("test default value: 3 != %d\n", (int)attr.stManual.GussianKernelWt[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[6] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.GussianKernelWt[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[7] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.GussianKernelWt[7]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[8] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.GussianKernelWt[8]);
		return CVI_FAILURE;
	}
	if (attr.stManual.GussianKernelWt[9] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.GussianKernelWt[9]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.GussianKernelWt[0] = 0;
	attr.stManual.GussianKernelWt[1] = 0;
	attr.stManual.GussianKernelWt[2] = 0;
	attr.stManual.GussianKernelWt[3] = 0;
	attr.stManual.GussianKernelWt[4] = 0;
	attr.stManual.GussianKernelWt[5] = 0;
	attr.stManual.GussianKernelWt[6] = 0;
	attr.stManual.GussianKernelWt[7] = 0;
	attr.stManual.GussianKernelWt[8] = 0;
	attr.stManual.GussianKernelWt[9] = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.GussianKernelWt[0] = 31;
	attr.stManual.GussianKernelWt[1] = 31;
	attr.stManual.GussianKernelWt[2] = 31;
	attr.stManual.GussianKernelWt[3] = 31;
	attr.stManual.GussianKernelWt[4] = 31;
	attr.stManual.GussianKernelWt[5] = 31;
	attr.stManual.GussianKernelWt[6] = 31;
	attr.stManual.GussianKernelWt[7] = 31;
	attr.stManual.GussianKernelWt[8] = 31;
	attr.stManual.GussianKernelWt[9] = 31;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.GussianKernelWt[0] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[1] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[2] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[3] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[4] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[5] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[6] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[7] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[8] = rand_range(0, 31);
	attr.stManual.GussianKernelWt[9] = rand_range(0, 31);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.GussianKernelWt[0] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[1] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[2] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[3] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[4] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[5] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[6] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[7] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[8] = rand_range(32, 255);
	attr.stManual.GussianKernelWt[9] = rand_range(32, 255);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.GussianKernelWt[0] = 31;
	attr.stManual.GussianKernelWt[1] = 23;
	attr.stManual.GussianKernelWt[2] = 17;
	attr.stManual.GussianKernelWt[3] = 9;
	attr.stManual.GussianKernelWt[4] = 7;
	attr.stManual.GussianKernelWt[5] = 3;
	attr.stManual.GussianKernelWt[6] = 0;
	attr.stManual.GussianKernelWt[7] = 0;
	attr.stManual.GussianKernelWt[8] = 0;
	attr.stManual.GussianKernelWt[9] = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_GussianKernelWt_manual(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_GussianKernelWt_manual);
}

static int test_BNR_BNRAttr_GussianKernelWt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31 };
	if (memcmp(default_val_0, attr.stAuto.GussianKernelWt[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	if (memcmp(default_val_1, attr.stAuto.GussianKernelWt[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };
	if (memcmp(default_val_2, attr.stAuto.GussianKernelWt[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	if (memcmp(default_val_3, attr.stAuto.GussianKernelWt[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_4, attr.stAuto.GussianKernelWt[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	if (memcmp(default_val_5, attr.stAuto.GussianKernelWt[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_6, attr.stAuto.GussianKernelWt[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_7, attr.stAuto.GussianKernelWt[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_8, attr.stAuto.GussianKernelWt[8], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_9, attr.stAuto.GussianKernelWt[9], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[7][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[8][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[9][j] = 0;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[0][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[1][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[2][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[3][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[4][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[5][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[6][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[7][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[8][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[9][j] = 31;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[0][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[1][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[2][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[3][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[4][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[5][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[6][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[7][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[8][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[9][j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[0][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[1][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[2][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[3][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[4][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[5][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[6][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[7][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[8][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.GussianKernelWt[9][j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31 };
	memcpy(attr.stAuto.GussianKernelWt[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	memcpy(attr.stAuto.GussianKernelWt[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };
	memcpy(attr.stAuto.GussianKernelWt[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	memcpy(attr.stAuto.GussianKernelWt[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.GussianKernelWt[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	memcpy(attr.stAuto.GussianKernelWt[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.GussianKernelWt[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.GussianKernelWt[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.GussianKernelWt[8], restore_val_8, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.GussianKernelWt[9], restore_val_9, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_GussianKernelWt_auto(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_GussianKernelWt_auto);
}

static int test_BNR_BNRAttr_NrBlendWt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.NrBlendWt != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.NrBlendWt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.NrBlendWt = 0;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.NrBlendWt = 16;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.NrBlendWt = rand_range(0, 16);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.NrBlendWt = rand_range(17, 255);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.NrBlendWt = 8;
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_NrBlendWt_manual(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_NrBlendWt_manual);
}

static int test_BNR_BNRAttr_NrBlendWt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_ATTR_S attr;

	ret = CVI_ISP_GetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.NrBlendWt, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NrBlendWt[j] = 0;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NrBlendWt[j] = 16;
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NrBlendWt[j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NrBlendWt[j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.NrBlendWt, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRAttr_NrBlendWt_auto(void)
{
	reg_api_test_fn(test_BNR_BNRAttr_NrBlendWt_auto);
}

static int test_BNR_BNRFilterAttr_DebugMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DebugMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.DebugMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DebugMode = 0;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.DebugMode = 3;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.DebugMode = rand_range(0, 3);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.DebugMode = rand_range(4, 255);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.DebugMode = 0;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_DebugMode(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_DebugMode);
}

static int test_BNR_BNRFilterAttr_Filter1GaussianCurve_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.Filter1GaussianCurve[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.Filter1GaussianCurve[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1GaussianCurve[1] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.Filter1GaussianCurve[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1GaussianCurve[2] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.Filter1GaussianCurve[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1GaussianCurve[3] != 3) {
		TEST_FAIL("test default value: 3 != %d\n", (int)attr.stManual.Filter1GaussianCurve[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1GaussianCurve[4] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.Filter1GaussianCurve[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1GaussianCurve[5] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.Filter1GaussianCurve[5]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.Filter1GaussianCurve[0] = 0;
	attr.stManual.Filter1GaussianCurve[1] = 0;
	attr.stManual.Filter1GaussianCurve[2] = 0;
	attr.stManual.Filter1GaussianCurve[3] = 0;
	attr.stManual.Filter1GaussianCurve[4] = 0;
	attr.stManual.Filter1GaussianCurve[5] = 0;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.Filter1GaussianCurve[0] = 1023;
	attr.stManual.Filter1GaussianCurve[1] = 1023;
	attr.stManual.Filter1GaussianCurve[2] = 1023;
	attr.stManual.Filter1GaussianCurve[3] = 1023;
	attr.stManual.Filter1GaussianCurve[4] = 1023;
	attr.stManual.Filter1GaussianCurve[5] = 1023;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.Filter1GaussianCurve[0] = rand_range(0, 1023);
	attr.stManual.Filter1GaussianCurve[1] = rand_range(0, 1023);
	attr.stManual.Filter1GaussianCurve[2] = rand_range(0, 1023);
	attr.stManual.Filter1GaussianCurve[3] = rand_range(0, 1023);
	attr.stManual.Filter1GaussianCurve[4] = rand_range(0, 1023);
	attr.stManual.Filter1GaussianCurve[5] = rand_range(0, 1023);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.Filter1GaussianCurve[0] = rand_range(1024, 65535);
	attr.stManual.Filter1GaussianCurve[1] = rand_range(1024, 65535);
	attr.stManual.Filter1GaussianCurve[2] = rand_range(1024, 65535);
	attr.stManual.Filter1GaussianCurve[3] = rand_range(1024, 65535);
	attr.stManual.Filter1GaussianCurve[4] = rand_range(1024, 65535);
	attr.stManual.Filter1GaussianCurve[5] = rand_range(1024, 65535);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.Filter1GaussianCurve[0] = 0;
	attr.stManual.Filter1GaussianCurve[1] = 1;
	attr.stManual.Filter1GaussianCurve[2] = 2;
	attr.stManual.Filter1GaussianCurve[3] = 3;
	attr.stManual.Filter1GaussianCurve[4] = 4;
	attr.stManual.Filter1GaussianCurve[5] = 5;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter1GaussianCurve_manual(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter1GaussianCurve_manual);
}

static int test_BNR_BNRFilterAttr_Filter1GaussianCurve_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.Filter1GaussianCurve[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_1, attr.stAuto.Filter1GaussianCurve[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_2, attr.stAuto.Filter1GaussianCurve[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	if (memcmp(default_val_3, attr.stAuto.Filter1GaussianCurve[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_4, attr.stAuto.Filter1GaussianCurve[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_5, attr.stAuto.Filter1GaussianCurve[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[5][j] = 0;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[0][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[1][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[2][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[3][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[4][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[5][j] = 1023;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[0][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[1][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[2][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[3][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[4][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[5][j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[0][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[1][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[2][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[3][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[4][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1GaussianCurve[5][j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.Filter1GaussianCurve[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.Filter1GaussianCurve[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.Filter1GaussianCurve[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	memcpy(attr.stAuto.Filter1GaussianCurve[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.Filter1GaussianCurve[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.Filter1GaussianCurve[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter1GaussianCurve_auto(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter1GaussianCurve_auto);
}

static int test_BNR_BNRFilterAttr_Filter1LumaOffset_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.Filter1LumaOffset[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.Filter1LumaOffset[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[1] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.Filter1LumaOffset[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[2] != 3) {
		TEST_FAIL("test default value: 3 != %d\n", (int)attr.stManual.Filter1LumaOffset[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[3] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.Filter1LumaOffset[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[4] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.Filter1LumaOffset[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[5] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.Filter1LumaOffset[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[6] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.Filter1LumaOffset[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[7] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.Filter1LumaOffset[7]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[8] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.Filter1LumaOffset[8]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[9] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.Filter1LumaOffset[9]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[10] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.Filter1LumaOffset[10]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[11] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.Filter1LumaOffset[11]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[12] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.Filter1LumaOffset[12]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[13] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.Filter1LumaOffset[13]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[14] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.Filter1LumaOffset[14]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[15] != 9) {
		TEST_FAIL("test default value: 9 != %d\n", (int)attr.stManual.Filter1LumaOffset[15]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter1LumaOffset[16] != 9) {
		TEST_FAIL("test default value: 9 != %d\n", (int)attr.stManual.Filter1LumaOffset[16]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.Filter1LumaOffset[0] = 0;
	attr.stManual.Filter1LumaOffset[1] = 0;
	attr.stManual.Filter1LumaOffset[2] = 0;
	attr.stManual.Filter1LumaOffset[3] = 0;
	attr.stManual.Filter1LumaOffset[4] = 0;
	attr.stManual.Filter1LumaOffset[5] = 0;
	attr.stManual.Filter1LumaOffset[6] = 0;
	attr.stManual.Filter1LumaOffset[7] = 0;
	attr.stManual.Filter1LumaOffset[8] = 0;
	attr.stManual.Filter1LumaOffset[9] = 0;
	attr.stManual.Filter1LumaOffset[10] = 0;
	attr.stManual.Filter1LumaOffset[11] = 0;
	attr.stManual.Filter1LumaOffset[12] = 0;
	attr.stManual.Filter1LumaOffset[13] = 0;
	attr.stManual.Filter1LumaOffset[14] = 0;
	attr.stManual.Filter1LumaOffset[15] = 0;
	attr.stManual.Filter1LumaOffset[16] = 0;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.Filter1LumaOffset[0] = 1023;
	attr.stManual.Filter1LumaOffset[1] = 1023;
	attr.stManual.Filter1LumaOffset[2] = 1023;
	attr.stManual.Filter1LumaOffset[3] = 1023;
	attr.stManual.Filter1LumaOffset[4] = 1023;
	attr.stManual.Filter1LumaOffset[5] = 1023;
	attr.stManual.Filter1LumaOffset[6] = 1023;
	attr.stManual.Filter1LumaOffset[7] = 1023;
	attr.stManual.Filter1LumaOffset[8] = 1023;
	attr.stManual.Filter1LumaOffset[9] = 1023;
	attr.stManual.Filter1LumaOffset[10] = 1023;
	attr.stManual.Filter1LumaOffset[11] = 1023;
	attr.stManual.Filter1LumaOffset[12] = 1023;
	attr.stManual.Filter1LumaOffset[13] = 1023;
	attr.stManual.Filter1LumaOffset[14] = 1023;
	attr.stManual.Filter1LumaOffset[15] = 1023;
	attr.stManual.Filter1LumaOffset[16] = 1023;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.Filter1LumaOffset[0] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[1] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[2] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[3] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[4] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[5] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[6] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[7] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[8] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[9] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[10] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[11] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[12] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[13] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[14] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[15] = rand_range(0, 1023);
	attr.stManual.Filter1LumaOffset[16] = rand_range(0, 1023);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.Filter1LumaOffset[0] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[1] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[2] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[3] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[4] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[5] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[6] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[7] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[8] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[9] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[10] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[11] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[12] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[13] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[14] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[15] = rand_range(1024, 65535);
	attr.stManual.Filter1LumaOffset[16] = rand_range(1024, 65535);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.Filter1LumaOffset[0] = 0;
	attr.stManual.Filter1LumaOffset[1] = 2;
	attr.stManual.Filter1LumaOffset[2] = 3;
	attr.stManual.Filter1LumaOffset[3] = 4;
	attr.stManual.Filter1LumaOffset[4] = 4;
	attr.stManual.Filter1LumaOffset[5] = 5;
	attr.stManual.Filter1LumaOffset[6] = 5;
	attr.stManual.Filter1LumaOffset[7] = 6;
	attr.stManual.Filter1LumaOffset[8] = 6;
	attr.stManual.Filter1LumaOffset[9] = 7;
	attr.stManual.Filter1LumaOffset[10] = 7;
	attr.stManual.Filter1LumaOffset[11] = 7;
	attr.stManual.Filter1LumaOffset[12] = 8;
	attr.stManual.Filter1LumaOffset[13] = 8;
	attr.stManual.Filter1LumaOffset[14] = 8;
	attr.stManual.Filter1LumaOffset[15] = 9;
	attr.stManual.Filter1LumaOffset[16] = 9;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter1LumaOffset_manual(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter1LumaOffset_manual);
}

static int test_BNR_BNRFilterAttr_Filter1LumaOffset_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.Filter1LumaOffset[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_1, attr.stAuto.Filter1LumaOffset[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	if (memcmp(default_val_2, attr.stAuto.Filter1LumaOffset[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_3, attr.stAuto.Filter1LumaOffset[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_4, attr.stAuto.Filter1LumaOffset[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_5, attr.stAuto.Filter1LumaOffset[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_6, attr.stAuto.Filter1LumaOffset[6], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_7, attr.stAuto.Filter1LumaOffset[7], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_8, attr.stAuto.Filter1LumaOffset[8], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_9, attr.stAuto.Filter1LumaOffset[9], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_10, attr.stAuto.Filter1LumaOffset[10], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_11, attr.stAuto.Filter1LumaOffset[11], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_12, attr.stAuto.Filter1LumaOffset[12], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_13, attr.stAuto.Filter1LumaOffset[13], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_14, attr.stAuto.Filter1LumaOffset[14], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	if (memcmp(default_val_15, attr.stAuto.Filter1LumaOffset[15], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	if (memcmp(default_val_16, attr.stAuto.Filter1LumaOffset[16], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[7][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[8][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[9][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[10][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[11][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[12][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[13][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[14][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[15][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[16][j] = 0;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[0][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[1][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[2][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[3][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[4][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[5][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[6][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[7][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[8][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[9][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[10][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[11][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[12][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[13][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[14][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[15][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[16][j] = 1023;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[0][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[1][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[2][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[3][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[4][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[5][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[6][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[7][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[8][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[9][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[10][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[11][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[12][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[13][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[14][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[15][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[16][j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[0][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[1][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[2][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[3][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[4][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[5][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[6][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[7][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[8][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[9][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[10][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[11][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[12][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[13][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[14][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[15][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter1LumaOffset[16][j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.Filter1LumaOffset[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.Filter1LumaOffset[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	memcpy(attr.stAuto.Filter1LumaOffset[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.Filter1LumaOffset[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.Filter1LumaOffset[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.Filter1LumaOffset[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.Filter1LumaOffset[6], restore_val_6, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.Filter1LumaOffset[7], restore_val_7, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.Filter1LumaOffset[8], restore_val_8, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.Filter1LumaOffset[9], restore_val_9, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.Filter1LumaOffset[10], restore_val_10, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.Filter1LumaOffset[11], restore_val_11, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.Filter1LumaOffset[12], restore_val_12, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.Filter1LumaOffset[13], restore_val_13, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.Filter1LumaOffset[14], restore_val_14, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	memcpy(attr.stAuto.Filter1LumaOffset[15], restore_val_15, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	memcpy(attr.stAuto.Filter1LumaOffset[16], restore_val_16, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter1LumaOffset_auto(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter1LumaOffset_auto);
}

static int test_BNR_BNRFilterAttr_Filter2GaussianCurve_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.Filter2GaussianCurve[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.Filter2GaussianCurve[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2GaussianCurve[1] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.Filter2GaussianCurve[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2GaussianCurve[2] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.Filter2GaussianCurve[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2GaussianCurve[3] != 3) {
		TEST_FAIL("test default value: 3 != %d\n", (int)attr.stManual.Filter2GaussianCurve[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2GaussianCurve[4] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.Filter2GaussianCurve[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2GaussianCurve[5] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.Filter2GaussianCurve[5]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.Filter2GaussianCurve[0] = 0;
	attr.stManual.Filter2GaussianCurve[1] = 0;
	attr.stManual.Filter2GaussianCurve[2] = 0;
	attr.stManual.Filter2GaussianCurve[3] = 0;
	attr.stManual.Filter2GaussianCurve[4] = 0;
	attr.stManual.Filter2GaussianCurve[5] = 0;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.Filter2GaussianCurve[0] = 1023;
	attr.stManual.Filter2GaussianCurve[1] = 1023;
	attr.stManual.Filter2GaussianCurve[2] = 1023;
	attr.stManual.Filter2GaussianCurve[3] = 1023;
	attr.stManual.Filter2GaussianCurve[4] = 1023;
	attr.stManual.Filter2GaussianCurve[5] = 1023;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.Filter2GaussianCurve[0] = rand_range(0, 1023);
	attr.stManual.Filter2GaussianCurve[1] = rand_range(0, 1023);
	attr.stManual.Filter2GaussianCurve[2] = rand_range(0, 1023);
	attr.stManual.Filter2GaussianCurve[3] = rand_range(0, 1023);
	attr.stManual.Filter2GaussianCurve[4] = rand_range(0, 1023);
	attr.stManual.Filter2GaussianCurve[5] = rand_range(0, 1023);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.Filter2GaussianCurve[0] = rand_range(1024, 65535);
	attr.stManual.Filter2GaussianCurve[1] = rand_range(1024, 65535);
	attr.stManual.Filter2GaussianCurve[2] = rand_range(1024, 65535);
	attr.stManual.Filter2GaussianCurve[3] = rand_range(1024, 65535);
	attr.stManual.Filter2GaussianCurve[4] = rand_range(1024, 65535);
	attr.stManual.Filter2GaussianCurve[5] = rand_range(1024, 65535);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.Filter2GaussianCurve[0] = 0;
	attr.stManual.Filter2GaussianCurve[1] = 1;
	attr.stManual.Filter2GaussianCurve[2] = 2;
	attr.stManual.Filter2GaussianCurve[3] = 3;
	attr.stManual.Filter2GaussianCurve[4] = 4;
	attr.stManual.Filter2GaussianCurve[5] = 5;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter2GaussianCurve_manual(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter2GaussianCurve_manual);
}

static int test_BNR_BNRFilterAttr_Filter2GaussianCurve_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.Filter2GaussianCurve[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_1, attr.stAuto.Filter2GaussianCurve[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_2, attr.stAuto.Filter2GaussianCurve[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	if (memcmp(default_val_3, attr.stAuto.Filter2GaussianCurve[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_4, attr.stAuto.Filter2GaussianCurve[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_5, attr.stAuto.Filter2GaussianCurve[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[5][j] = 0;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[0][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[1][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[2][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[3][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[4][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[5][j] = 1023;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[0][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[1][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[2][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[3][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[4][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[5][j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[0][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[1][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[2][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[3][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[4][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2GaussianCurve[5][j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.Filter2GaussianCurve[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.Filter2GaussianCurve[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.Filter2GaussianCurve[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	memcpy(attr.stAuto.Filter2GaussianCurve[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.Filter2GaussianCurve[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.Filter2GaussianCurve[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter2GaussianCurve_auto(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter2GaussianCurve_auto);
}

static int test_BNR_BNRFilterAttr_Filter2LumaOffset_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.Filter2LumaOffset[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.Filter2LumaOffset[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[1] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.Filter2LumaOffset[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[2] != 3) {
		TEST_FAIL("test default value: 3 != %d\n", (int)attr.stManual.Filter2LumaOffset[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[3] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.Filter2LumaOffset[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[4] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.Filter2LumaOffset[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[5] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.Filter2LumaOffset[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[6] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.Filter2LumaOffset[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[7] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.Filter2LumaOffset[7]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[8] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.Filter2LumaOffset[8]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[9] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.Filter2LumaOffset[9]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[10] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.Filter2LumaOffset[10]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[11] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.Filter2LumaOffset[11]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[12] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.Filter2LumaOffset[12]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[13] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.Filter2LumaOffset[13]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[14] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.Filter2LumaOffset[14]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[15] != 9) {
		TEST_FAIL("test default value: 9 != %d\n", (int)attr.stManual.Filter2LumaOffset[15]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Filter2LumaOffset[16] != 9) {
		TEST_FAIL("test default value: 9 != %d\n", (int)attr.stManual.Filter2LumaOffset[16]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.Filter2LumaOffset[0] = 0;
	attr.stManual.Filter2LumaOffset[1] = 0;
	attr.stManual.Filter2LumaOffset[2] = 0;
	attr.stManual.Filter2LumaOffset[3] = 0;
	attr.stManual.Filter2LumaOffset[4] = 0;
	attr.stManual.Filter2LumaOffset[5] = 0;
	attr.stManual.Filter2LumaOffset[6] = 0;
	attr.stManual.Filter2LumaOffset[7] = 0;
	attr.stManual.Filter2LumaOffset[8] = 0;
	attr.stManual.Filter2LumaOffset[9] = 0;
	attr.stManual.Filter2LumaOffset[10] = 0;
	attr.stManual.Filter2LumaOffset[11] = 0;
	attr.stManual.Filter2LumaOffset[12] = 0;
	attr.stManual.Filter2LumaOffset[13] = 0;
	attr.stManual.Filter2LumaOffset[14] = 0;
	attr.stManual.Filter2LumaOffset[15] = 0;
	attr.stManual.Filter2LumaOffset[16] = 0;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.Filter2LumaOffset[0] = 1023;
	attr.stManual.Filter2LumaOffset[1] = 1023;
	attr.stManual.Filter2LumaOffset[2] = 1023;
	attr.stManual.Filter2LumaOffset[3] = 1023;
	attr.stManual.Filter2LumaOffset[4] = 1023;
	attr.stManual.Filter2LumaOffset[5] = 1023;
	attr.stManual.Filter2LumaOffset[6] = 1023;
	attr.stManual.Filter2LumaOffset[7] = 1023;
	attr.stManual.Filter2LumaOffset[8] = 1023;
	attr.stManual.Filter2LumaOffset[9] = 1023;
	attr.stManual.Filter2LumaOffset[10] = 1023;
	attr.stManual.Filter2LumaOffset[11] = 1023;
	attr.stManual.Filter2LumaOffset[12] = 1023;
	attr.stManual.Filter2LumaOffset[13] = 1023;
	attr.stManual.Filter2LumaOffset[14] = 1023;
	attr.stManual.Filter2LumaOffset[15] = 1023;
	attr.stManual.Filter2LumaOffset[16] = 1023;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.Filter2LumaOffset[0] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[1] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[2] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[3] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[4] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[5] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[6] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[7] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[8] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[9] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[10] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[11] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[12] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[13] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[14] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[15] = rand_range(0, 1023);
	attr.stManual.Filter2LumaOffset[16] = rand_range(0, 1023);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.Filter2LumaOffset[0] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[1] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[2] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[3] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[4] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[5] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[6] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[7] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[8] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[9] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[10] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[11] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[12] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[13] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[14] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[15] = rand_range(1024, 65535);
	attr.stManual.Filter2LumaOffset[16] = rand_range(1024, 65535);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.Filter2LumaOffset[0] = 0;
	attr.stManual.Filter2LumaOffset[1] = 2;
	attr.stManual.Filter2LumaOffset[2] = 3;
	attr.stManual.Filter2LumaOffset[3] = 4;
	attr.stManual.Filter2LumaOffset[4] = 4;
	attr.stManual.Filter2LumaOffset[5] = 5;
	attr.stManual.Filter2LumaOffset[6] = 5;
	attr.stManual.Filter2LumaOffset[7] = 6;
	attr.stManual.Filter2LumaOffset[8] = 6;
	attr.stManual.Filter2LumaOffset[9] = 7;
	attr.stManual.Filter2LumaOffset[10] = 7;
	attr.stManual.Filter2LumaOffset[11] = 7;
	attr.stManual.Filter2LumaOffset[12] = 8;
	attr.stManual.Filter2LumaOffset[13] = 8;
	attr.stManual.Filter2LumaOffset[14] = 8;
	attr.stManual.Filter2LumaOffset[15] = 9;
	attr.stManual.Filter2LumaOffset[16] = 9;
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter2LumaOffset_manual(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter2LumaOffset_manual);
}

static int test_BNR_BNRFilterAttr_Filter2LumaOffset_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_BNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetBNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.Filter2LumaOffset[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_1, attr.stAuto.Filter2LumaOffset[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	if (memcmp(default_val_2, attr.stAuto.Filter2LumaOffset[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_3, attr.stAuto.Filter2LumaOffset[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_4, attr.stAuto.Filter2LumaOffset[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_5, attr.stAuto.Filter2LumaOffset[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_6, attr.stAuto.Filter2LumaOffset[6], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_7, attr.stAuto.Filter2LumaOffset[7], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_8, attr.stAuto.Filter2LumaOffset[8], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_9, attr.stAuto.Filter2LumaOffset[9], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_10, attr.stAuto.Filter2LumaOffset[10], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_11, attr.stAuto.Filter2LumaOffset[11], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_12, attr.stAuto.Filter2LumaOffset[12], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_13, attr.stAuto.Filter2LumaOffset[13], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_14, attr.stAuto.Filter2LumaOffset[14], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	if (memcmp(default_val_15, attr.stAuto.Filter2LumaOffset[15], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	if (memcmp(default_val_16, attr.stAuto.Filter2LumaOffset[16], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[7][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[8][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[9][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[10][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[11][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[12][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[13][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[14][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[15][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[16][j] = 0;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[0][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[1][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[2][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[3][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[4][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[5][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[6][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[7][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[8][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[9][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[10][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[11][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[12][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[13][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[14][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[15][j] = 1023;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[16][j] = 1023;
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[0][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[1][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[2][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[3][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[4][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[5][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[6][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[7][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[8][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[9][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[10][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[11][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[12][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[13][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[14][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[15][j] = rand_range(0, 1023);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[16][j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[0][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[1][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[2][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[3][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[4][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[5][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[6][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[7][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[8][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[9][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[10][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[11][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[12][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[13][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[14][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[15][j] = rand_range(1024, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Filter2LumaOffset[16][j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.Filter2LumaOffset[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.Filter2LumaOffset[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
	memcpy(attr.stAuto.Filter2LumaOffset[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.Filter2LumaOffset[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.Filter2LumaOffset[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.Filter2LumaOffset[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.Filter2LumaOffset[6], restore_val_6, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.Filter2LumaOffset[7], restore_val_7, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.Filter2LumaOffset[8], restore_val_8, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.Filter2LumaOffset[9], restore_val_9, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.Filter2LumaOffset[10], restore_val_10, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.Filter2LumaOffset[11], restore_val_11, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.Filter2LumaOffset[12], restore_val_12, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.Filter2LumaOffset[13], restore_val_13, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.Filter2LumaOffset[14], restore_val_14, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	memcpy(attr.stAuto.Filter2LumaOffset[15], restore_val_15, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };
	memcpy(attr.stAuto.Filter2LumaOffset[16], restore_val_16, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetBNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_BNR_BNRFilterAttr_Filter2LumaOffset_auto(void)
{
	reg_api_test_fn(test_BNR_BNRFilterAttr_Filter2LumaOffset_auto);
}
