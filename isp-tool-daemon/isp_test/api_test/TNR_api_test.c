
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

static int test_TNR_TNRAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_Enable(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_Enable);
}

static int test_TNR_TNRAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_enOpType(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_enOpType);
}

static int test_TNR_TNRAttr_updateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.updateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.updateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.updateInterval = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.updateInterval = 255;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.updateInterval = rand_range(0, 255);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.updateInterval = 1;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_updateInterval(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_updateInterval);
}

static int test_TNR_TNRAttr_DbgMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DbgMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.DbgMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DbgMode = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DbgMode = 8;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DbgMode = rand_range(0, 8);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DbgMode = rand_range(9, 255);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.DbgMode = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_DbgMode(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_DbgMode);
}

static int test_TNR_TNRAttr_DyBlurUVWt(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DyBlurUVWt != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.DyBlurUVWt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DyBlurUVWt = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DyBlurUVWt = 15;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DyBlurUVWt = rand_range(0, 15);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DyBlurUVWt = rand_range(16, 255);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.DyBlurUVWt = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_DyBlurUVWt(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_DyBlurUVWt);
}

static int test_TNR_TNRAttr_DyBlurJndTh(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DyBlurJndTh != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.DyBlurJndTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DyBlurJndTh = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DyBlurJndTh = 31;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DyBlurJndTh = rand_range(0, 31);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.DyBlurJndTh = rand_range(32, 255);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.DyBlurJndTh = 8;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_DyBlurJndTh(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_DyBlurJndTh);
}

static int test_TNR_TNRAttr_DyBlurStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DyBlurStr != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.DyBlurStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DyBlurStr = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.stManual.DyBlurStr = 3;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.stManual.DyBlurStr = rand_range(0, 3);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.stManual.DyBlurStr = rand_range(4, 255);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DyBlurStr = 1;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_DyBlurStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_DyBlurStr_manual);
}

static int test_TNR_TNRAttr_DyBlurStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_0, attr.stAuto.DyBlurStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurStr[j] = 0;
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
		attr.stAuto.DyBlurStr[j] = 3;
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
		attr.stAuto.DyBlurStr[j] = rand_range(0, 3);
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
		attr.stAuto.DyBlurStr[j] = rand_range(4, 255);
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.DyBlurStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_DyBlurStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_DyBlurStr_auto);
}

static int test_TNR_TNRAttr_DyBlurYWt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DyBlurYWt[0] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.DyBlurYWt[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.DyBlurYWt[1] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.DyBlurYWt[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.DyBlurYWt[2] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.DyBlurYWt[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DyBlurYWt[0] = 0;
	attr.stManual.DyBlurYWt[1] = 0;
	attr.stManual.DyBlurYWt[2] = 0;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.stManual.DyBlurYWt[0] = 15;
	attr.stManual.DyBlurYWt[1] = 15;
	attr.stManual.DyBlurYWt[2] = 15;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.stManual.DyBlurYWt[0] = rand_range(0, 15);
	attr.stManual.DyBlurYWt[1] = rand_range(0, 15);
	attr.stManual.DyBlurYWt[2] = rand_range(0, 15);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
	attr.stManual.DyBlurYWt[0] = rand_range(16, 255);
	attr.stManual.DyBlurYWt[1] = rand_range(16, 255);
	attr.stManual.DyBlurYWt[2] = rand_range(16, 255);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DyBlurYWt[0] = 1;
	attr.stManual.DyBlurYWt[1] = 1;
	attr.stManual.DyBlurYWt[2] = 1;
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_DyBlurYWt_manual(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_DyBlurYWt_manual);
}

static int test_TNR_TNRAttr_DyBlurYWt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_ATTR_S attr;

	ret = CVI_ISP_GetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_0, attr.stAuto.DyBlurYWt[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_1, attr.stAuto.DyBlurYWt[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_2, attr.stAuto.DyBlurYWt[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[2][j] = 0;
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
		attr.stAuto.DyBlurYWt[0][j] = 15;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[1][j] = 15;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[2][j] = 15;
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
		attr.stAuto.DyBlurYWt[0][j] = rand_range(0, 15);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[1][j] = rand_range(0, 15);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[2][j] = rand_range(0, 15);
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
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
		attr.stAuto.DyBlurYWt[0][j] = rand_range(16, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[1][j] = rand_range(16, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DyBlurYWt[2][j] = rand_range(16, 255);
	}
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.DyBlurYWt[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.DyBlurYWt[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.DyBlurYWt[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRAttr_DyBlurYWt_auto(void)
{
	reg_api_test_fn(test_TNR_TNRAttr_DyBlurYWt_auto);
}

static int test_TNR_TNRMvAttr_NullFlowEn(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.NullFlowEn != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.NullFlowEn);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.NullFlowEn = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.NullFlowEn = 1;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.NullFlowEn = rand_range(0, 1);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.NullFlowEn = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_NullFlowEn(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_NullFlowEn);
}

static int test_TNR_TNRMvAttr_StableIntensity_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.StableIntensity != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.StableIntensity);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.StableIntensity = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.StableIntensity = 8;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.StableIntensity = rand_range(0, 8);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.StableIntensity = rand_range(9, 255);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.StableIntensity = 4;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_StableIntensity_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_StableIntensity_manual);
}

static int test_TNR_TNRMvAttr_StableIntensity_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.StableIntensity, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.StableIntensity[j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.StableIntensity[j] = 8;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.StableIntensity[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.StableIntensity[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.StableIntensity, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_StableIntensity_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_StableIntensity_auto);
}

static int test_TNR_TNRMvAttr_L0YPSSBlur_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L0YPSSBlur != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.L0YPSSBlur);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L0YPSSBlur = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.L0YPSSBlur = 255;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.L0YPSSBlur = rand_range(0, 255);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.L0YPSSBlur = 255;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_L0YPSSBlur_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_L0YPSSBlur_manual);
}

static int test_TNR_TNRMvAttr_L0YPSSBlur_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_0, attr.stAuto.L0YPSSBlur, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YPSSBlur[j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.L0YPSSBlur[j] = 255;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.L0YPSSBlur[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.L0YPSSBlur, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_L0YPSSBlur_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_L0YPSSBlur_auto);
}

static int test_TNR_TNRMvAttr_AcbdSadAdj_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.AcbdSadAdj[0] != 100) {
		TEST_FAIL("test default value: 100 != %d\n", (int)attr.stManual.AcbdSadAdj[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.AcbdSadAdj[1] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.AcbdSadAdj[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.AcbdSadAdj[2] != 150) {
		TEST_FAIL("test default value: 150 != %d\n", (int)attr.stManual.AcbdSadAdj[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.AcbdSadAdj[3] != 200) {
		TEST_FAIL("test default value: 200 != %d\n", (int)attr.stManual.AcbdSadAdj[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.AcbdSadAdj[0] = 0;
	attr.stManual.AcbdSadAdj[1] = 0;
	attr.stManual.AcbdSadAdj[2] = 0;
	attr.stManual.AcbdSadAdj[3] = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.AcbdSadAdj[0] = 16383;
	attr.stManual.AcbdSadAdj[1] = 16383;
	attr.stManual.AcbdSadAdj[2] = 16383;
	attr.stManual.AcbdSadAdj[3] = 16383;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.AcbdSadAdj[0] = rand_range(0, 16383);
	attr.stManual.AcbdSadAdj[1] = rand_range(0, 16383);
	attr.stManual.AcbdSadAdj[2] = rand_range(0, 16383);
	attr.stManual.AcbdSadAdj[3] = rand_range(0, 16383);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.AcbdSadAdj[0] = rand_range(16384, 65535);
	attr.stManual.AcbdSadAdj[1] = rand_range(16384, 65535);
	attr.stManual.AcbdSadAdj[2] = rand_range(16384, 65535);
	attr.stManual.AcbdSadAdj[3] = rand_range(16384, 65535);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.AcbdSadAdj[0] = 100;
	attr.stManual.AcbdSadAdj[1] = 0;
	attr.stManual.AcbdSadAdj[2] = 150;
	attr.stManual.AcbdSadAdj[3] = 200;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_AcbdSadAdj_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_AcbdSadAdj_manual);
}

static int test_TNR_TNRMvAttr_AcbdSadAdj_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100 };
	if (memcmp(default_val_0, attr.stAuto.AcbdSadAdj[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_1, attr.stAuto.AcbdSadAdj[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	if (memcmp(default_val_2, attr.stAuto.AcbdSadAdj[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	if (memcmp(default_val_3, attr.stAuto.AcbdSadAdj[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[3][j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.AcbdSadAdj[0][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[1][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[2][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[3][j] = 16383;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.AcbdSadAdj[0][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[1][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[2][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[3][j] = rand_range(0, 16383);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.AcbdSadAdj[0][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[1][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[2][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdSadAdj[3][j] = rand_range(16384, 65535);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100 };
	memcpy(attr.stAuto.AcbdSadAdj[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.AcbdSadAdj[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	memcpy(attr.stAuto.AcbdSadAdj[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	memcpy(attr.stAuto.AcbdSadAdj[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_AcbdSadAdj_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_AcbdSadAdj_auto);
}

static int test_TNR_TNRMvAttr_AcbdEdgeAdj_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.AcbdEdgeAdj[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.AcbdEdgeAdj[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.AcbdEdgeAdj[1] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.AcbdEdgeAdj[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.AcbdEdgeAdj[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.AcbdEdgeAdj[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.AcbdEdgeAdj[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.AcbdEdgeAdj[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.AcbdEdgeAdj[0] = 0;
	attr.stManual.AcbdEdgeAdj[1] = 0;
	attr.stManual.AcbdEdgeAdj[2] = 0;
	attr.stManual.AcbdEdgeAdj[3] = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.AcbdEdgeAdj[0] = 16;
	attr.stManual.AcbdEdgeAdj[1] = 16;
	attr.stManual.AcbdEdgeAdj[2] = 16;
	attr.stManual.AcbdEdgeAdj[3] = 16;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.AcbdEdgeAdj[0] = rand_range(0, 16);
	attr.stManual.AcbdEdgeAdj[1] = rand_range(0, 16);
	attr.stManual.AcbdEdgeAdj[2] = rand_range(0, 16);
	attr.stManual.AcbdEdgeAdj[3] = rand_range(0, 16);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.AcbdEdgeAdj[0] = rand_range(17, 255);
	attr.stManual.AcbdEdgeAdj[1] = rand_range(17, 255);
	attr.stManual.AcbdEdgeAdj[2] = rand_range(17, 255);
	attr.stManual.AcbdEdgeAdj[3] = rand_range(17, 255);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.AcbdEdgeAdj[0] = 16;
	attr.stManual.AcbdEdgeAdj[1] = 0;
	attr.stManual.AcbdEdgeAdj[2] = 16;
	attr.stManual.AcbdEdgeAdj[3] = 16;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_AcbdEdgeAdj_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_AcbdEdgeAdj_manual);
}

static int test_TNR_TNRMvAttr_AcbdEdgeAdj_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.AcbdEdgeAdj[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_1, attr.stAuto.AcbdEdgeAdj[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.AcbdEdgeAdj[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.AcbdEdgeAdj[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[3][j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.AcbdEdgeAdj[0][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[1][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[2][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[3][j] = 16;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.AcbdEdgeAdj[0][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[1][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[2][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[3][j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.AcbdEdgeAdj[0][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[1][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[2][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AcbdEdgeAdj[3][j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.AcbdEdgeAdj[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.AcbdEdgeAdj[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.AcbdEdgeAdj[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.AcbdEdgeAdj[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_AcbdEdgeAdj_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_AcbdEdgeAdj_auto);
}

static int test_TNR_TNRMvAttr_LumaJndTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaJndTh[0] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.LumaJndTh[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaJndTh[1] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.LumaJndTh[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaJndTh[2] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.LumaJndTh[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaJndTh[3] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.LumaJndTh[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaJndTh[4] != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.LumaJndTh[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaJndTh[5] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.LumaJndTh[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaJndTh[6] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.LumaJndTh[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaJndTh[7] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.LumaJndTh[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaJndTh[0] = 0;
	attr.stManual.LumaJndTh[1] = 0;
	attr.stManual.LumaJndTh[2] = 0;
	attr.stManual.LumaJndTh[3] = 0;
	attr.stManual.LumaJndTh[4] = 0;
	attr.stManual.LumaJndTh[5] = 0;
	attr.stManual.LumaJndTh[6] = 0;
	attr.stManual.LumaJndTh[7] = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.LumaJndTh[0] = 255;
	attr.stManual.LumaJndTh[1] = 255;
	attr.stManual.LumaJndTh[2] = 255;
	attr.stManual.LumaJndTh[3] = 255;
	attr.stManual.LumaJndTh[4] = 255;
	attr.stManual.LumaJndTh[5] = 255;
	attr.stManual.LumaJndTh[6] = 255;
	attr.stManual.LumaJndTh[7] = 255;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.LumaJndTh[0] = rand_range(0, 255);
	attr.stManual.LumaJndTh[1] = rand_range(0, 255);
	attr.stManual.LumaJndTh[2] = rand_range(0, 255);
	attr.stManual.LumaJndTh[3] = rand_range(0, 255);
	attr.stManual.LumaJndTh[4] = rand_range(0, 255);
	attr.stManual.LumaJndTh[5] = rand_range(0, 255);
	attr.stManual.LumaJndTh[6] = rand_range(0, 255);
	attr.stManual.LumaJndTh[7] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.LumaJndTh[0] = 4;
	attr.stManual.LumaJndTh[1] = 4;
	attr.stManual.LumaJndTh[2] = 5;
	attr.stManual.LumaJndTh[3] = 5;
	attr.stManual.LumaJndTh[4] = 5;
	attr.stManual.LumaJndTh[5] = 6;
	attr.stManual.LumaJndTh[6] = 6;
	attr.stManual.LumaJndTh[7] = 6;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_LumaJndTh_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_LumaJndTh_manual);
}

static int test_TNR_TNRMvAttr_LumaJndTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.LumaJndTh[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_1, attr.stAuto.LumaJndTh[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_2, attr.stAuto.LumaJndTh[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_3, attr.stAuto.LumaJndTh[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_4, attr.stAuto.LumaJndTh[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_5, attr.stAuto.LumaJndTh[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_6, attr.stAuto.LumaJndTh[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_7, attr.stAuto.LumaJndTh[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.LumaJndTh[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[4][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[5][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[6][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[7][j] = 255;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.LumaJndTh[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[4][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[5][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[6][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndTh[7][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.LumaJndTh[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.LumaJndTh[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.LumaJndTh[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.LumaJndTh[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.LumaJndTh[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.LumaJndTh[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.LumaJndTh[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.LumaJndTh[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_LumaJndTh_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_LumaJndTh_auto);
}

static int test_TNR_TNRMvAttr_LumaJndRatio_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaJndRatio != 180) {
		TEST_FAIL("test default value: 180 != %d\n", (int)attr.stManual.LumaJndRatio);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaJndRatio = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.LumaJndRatio = 255;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.LumaJndRatio = rand_range(0, 255);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.LumaJndRatio = 180;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_LumaJndRatio_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_LumaJndRatio_manual);
}

static int test_TNR_TNRMvAttr_LumaJndRatio_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180 };
	if (memcmp(default_val_0, attr.stAuto.LumaJndRatio, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaJndRatio[j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.LumaJndRatio[j] = 255;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.LumaJndRatio[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180 };
	memcpy(attr.stAuto.LumaJndRatio, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_LumaJndRatio_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_LumaJndRatio_auto);
}

static int test_TNR_TNRMvAttr_JndSadWt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.JndSadWt != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.JndSadWt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.JndSadWt = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.JndSadWt = 15;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.JndSadWt = rand_range(0, 15);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.JndSadWt = rand_range(16, 255);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.JndSadWt = 8;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_JndSadWt_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_JndSadWt_manual);
}

static int test_TNR_TNRMvAttr_JndSadWt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.JndSadWt, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.JndSadWt[j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.JndSadWt[j] = 15;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.JndSadWt[j] = rand_range(0, 15);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.JndSadWt[j] = rand_range(16, 255);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.JndSadWt, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_JndSadWt_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_JndSadWt_auto);
}

static int test_TNR_TNRMvAttr_WarpB0_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.WarpB0[0] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpB0[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpB0[2] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpB0[3] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpB0[4] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpB0[5] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpB0[6] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpB0[7] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.WarpB0[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.WarpB0[0] = 0;
	attr.stManual.WarpB0[1] = 0;
	attr.stManual.WarpB0[2] = 0;
	attr.stManual.WarpB0[3] = 0;
	attr.stManual.WarpB0[4] = 0;
	attr.stManual.WarpB0[5] = 0;
	attr.stManual.WarpB0[6] = 0;
	attr.stManual.WarpB0[7] = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpB0[0] = 16383;
	attr.stManual.WarpB0[1] = 16383;
	attr.stManual.WarpB0[2] = 16383;
	attr.stManual.WarpB0[3] = 16383;
	attr.stManual.WarpB0[4] = 16383;
	attr.stManual.WarpB0[5] = 16383;
	attr.stManual.WarpB0[6] = 16383;
	attr.stManual.WarpB0[7] = 16383;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpB0[0] = rand_range(0, 16383);
	attr.stManual.WarpB0[1] = rand_range(0, 16383);
	attr.stManual.WarpB0[2] = rand_range(0, 16383);
	attr.stManual.WarpB0[3] = rand_range(0, 16383);
	attr.stManual.WarpB0[4] = rand_range(0, 16383);
	attr.stManual.WarpB0[5] = rand_range(0, 16383);
	attr.stManual.WarpB0[6] = rand_range(0, 16383);
	attr.stManual.WarpB0[7] = rand_range(0, 16383);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpB0[0] = rand_range(16384, 65535);
	attr.stManual.WarpB0[1] = rand_range(16384, 65535);
	attr.stManual.WarpB0[2] = rand_range(16384, 65535);
	attr.stManual.WarpB0[3] = rand_range(16384, 65535);
	attr.stManual.WarpB0[4] = rand_range(16384, 65535);
	attr.stManual.WarpB0[5] = rand_range(16384, 65535);
	attr.stManual.WarpB0[6] = rand_range(16384, 65535);
	attr.stManual.WarpB0[7] = rand_range(16384, 65535);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.WarpB0[0] = 64;
	attr.stManual.WarpB0[1] = 64;
	attr.stManual.WarpB0[2] = 64;
	attr.stManual.WarpB0[3] = 64;
	attr.stManual.WarpB0[4] = 64;
	attr.stManual.WarpB0[5] = 64;
	attr.stManual.WarpB0[6] = 64;
	attr.stManual.WarpB0[7] = 64;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpB0_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpB0_manual);
}

static int test_TNR_TNRMvAttr_WarpB0_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.WarpB0[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.WarpB0[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_2, attr.stAuto.WarpB0[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_3, attr.stAuto.WarpB0[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_4, attr.stAuto.WarpB0[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_5, attr.stAuto.WarpB0[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_6, attr.stAuto.WarpB0[6], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_7, attr.stAuto.WarpB0[7], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpB0[0][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[1][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[2][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[3][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[4][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[5][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[6][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[7][j] = 16383;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpB0[0][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[1][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[2][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[3][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[4][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[5][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[6][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[7][j] = rand_range(0, 16383);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpB0[0][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[1][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[2][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[3][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[4][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[5][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[6][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpB0[7][j] = rand_range(16384, 65535);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[6], restore_val_6, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.WarpB0[7], restore_val_7, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpB0_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpB0_auto);
}

static int test_TNR_TNRMvAttr_WarpEdge_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.WarpEdge[0] != 23) {
		TEST_FAIL("test default value: 23 != %d\n", (int)attr.stManual.WarpEdge[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpEdge[1] != 21) {
		TEST_FAIL("test default value: 21 != %d\n", (int)attr.stManual.WarpEdge[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpEdge[2] != 23) {
		TEST_FAIL("test default value: 23 != %d\n", (int)attr.stManual.WarpEdge[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpEdge[3] != 23) {
		TEST_FAIL("test default value: 23 != %d\n", (int)attr.stManual.WarpEdge[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpEdge[4] != 23) {
		TEST_FAIL("test default value: 23 != %d\n", (int)attr.stManual.WarpEdge[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpEdge[5] != 21) {
		TEST_FAIL("test default value: 21 != %d\n", (int)attr.stManual.WarpEdge[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpEdge[6] != 17) {
		TEST_FAIL("test default value: 17 != %d\n", (int)attr.stManual.WarpEdge[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpEdge[7] != 17) {
		TEST_FAIL("test default value: 17 != %d\n", (int)attr.stManual.WarpEdge[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.WarpEdge[0] = 0;
	attr.stManual.WarpEdge[1] = 0;
	attr.stManual.WarpEdge[2] = 0;
	attr.stManual.WarpEdge[3] = 0;
	attr.stManual.WarpEdge[4] = 0;
	attr.stManual.WarpEdge[5] = 0;
	attr.stManual.WarpEdge[6] = 0;
	attr.stManual.WarpEdge[7] = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpEdge[0] = 63;
	attr.stManual.WarpEdge[1] = 63;
	attr.stManual.WarpEdge[2] = 63;
	attr.stManual.WarpEdge[3] = 63;
	attr.stManual.WarpEdge[4] = 63;
	attr.stManual.WarpEdge[5] = 63;
	attr.stManual.WarpEdge[6] = 63;
	attr.stManual.WarpEdge[7] = 63;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpEdge[0] = rand_range(0, 63);
	attr.stManual.WarpEdge[1] = rand_range(0, 63);
	attr.stManual.WarpEdge[2] = rand_range(0, 63);
	attr.stManual.WarpEdge[3] = rand_range(0, 63);
	attr.stManual.WarpEdge[4] = rand_range(0, 63);
	attr.stManual.WarpEdge[5] = rand_range(0, 63);
	attr.stManual.WarpEdge[6] = rand_range(0, 63);
	attr.stManual.WarpEdge[7] = rand_range(0, 63);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpEdge[0] = rand_range(64, 255);
	attr.stManual.WarpEdge[1] = rand_range(64, 255);
	attr.stManual.WarpEdge[2] = rand_range(64, 255);
	attr.stManual.WarpEdge[3] = rand_range(64, 255);
	attr.stManual.WarpEdge[4] = rand_range(64, 255);
	attr.stManual.WarpEdge[5] = rand_range(64, 255);
	attr.stManual.WarpEdge[6] = rand_range(64, 255);
	attr.stManual.WarpEdge[7] = rand_range(64, 255);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.WarpEdge[0] = 23;
	attr.stManual.WarpEdge[1] = 21;
	attr.stManual.WarpEdge[2] = 23;
	attr.stManual.WarpEdge[3] = 23;
	attr.stManual.WarpEdge[4] = 23;
	attr.stManual.WarpEdge[5] = 21;
	attr.stManual.WarpEdge[6] = 17;
	attr.stManual.WarpEdge[7] = 17;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpEdge_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpEdge_manual);
}

static int test_TNR_TNRMvAttr_WarpEdge_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	if (memcmp(default_val_0, attr.stAuto.WarpEdge[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21 };
	if (memcmp(default_val_1, attr.stAuto.WarpEdge[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	if (memcmp(default_val_2, attr.stAuto.WarpEdge[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	if (memcmp(default_val_3, attr.stAuto.WarpEdge[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	if (memcmp(default_val_4, attr.stAuto.WarpEdge[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21 };
	if (memcmp(default_val_5, attr.stAuto.WarpEdge[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };
	if (memcmp(default_val_6, attr.stAuto.WarpEdge[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };
	if (memcmp(default_val_7, attr.stAuto.WarpEdge[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpEdge[0][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[1][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[2][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[3][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[4][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[5][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[6][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[7][j] = 63;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpEdge[0][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[1][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[2][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[3][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[4][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[5][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[6][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[7][j] = rand_range(0, 63);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpEdge[0][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[1][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[2][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[3][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[4][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[5][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[6][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpEdge[7][j] = rand_range(64, 255);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	memcpy(attr.stAuto.WarpEdge[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21 };
	memcpy(attr.stAuto.WarpEdge[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	memcpy(attr.stAuto.WarpEdge[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	memcpy(attr.stAuto.WarpEdge[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23 };
	memcpy(attr.stAuto.WarpEdge[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21 };
	memcpy(attr.stAuto.WarpEdge[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };
	memcpy(attr.stAuto.WarpEdge[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };
	memcpy(attr.stAuto.WarpEdge[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpEdge_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpEdge_auto);
}

static int test_TNR_TNRMvAttr_WarpBase_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.WarpBase[0] != 15) {
		TEST_FAIL("test default value: 15 != %d\n", (int)attr.stManual.WarpBase[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpBase[1] != 25) {
		TEST_FAIL("test default value: 25 != %d\n", (int)attr.stManual.WarpBase[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpBase[2] != 19) {
		TEST_FAIL("test default value: 19 != %d\n", (int)attr.stManual.WarpBase[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpBase[3] != 27) {
		TEST_FAIL("test default value: 27 != %d\n", (int)attr.stManual.WarpBase[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpBase[4] != 29) {
		TEST_FAIL("test default value: 29 != %d\n", (int)attr.stManual.WarpBase[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpBase[5] != 67) {
		TEST_FAIL("test default value: 67 != %d\n", (int)attr.stManual.WarpBase[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpBase[6] != 140) {
		TEST_FAIL("test default value: 140 != %d\n", (int)attr.stManual.WarpBase[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpBase[7] != 140) {
		TEST_FAIL("test default value: 140 != %d\n", (int)attr.stManual.WarpBase[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.WarpBase[0] = 0;
	attr.stManual.WarpBase[1] = 0;
	attr.stManual.WarpBase[2] = 0;
	attr.stManual.WarpBase[3] = 0;
	attr.stManual.WarpBase[4] = 0;
	attr.stManual.WarpBase[5] = 0;
	attr.stManual.WarpBase[6] = 0;
	attr.stManual.WarpBase[7] = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpBase[0] = 16383;
	attr.stManual.WarpBase[1] = 16383;
	attr.stManual.WarpBase[2] = 16383;
	attr.stManual.WarpBase[3] = 16383;
	attr.stManual.WarpBase[4] = 16383;
	attr.stManual.WarpBase[5] = 16383;
	attr.stManual.WarpBase[6] = 16383;
	attr.stManual.WarpBase[7] = 16383;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpBase[0] = rand_range(0, 16383);
	attr.stManual.WarpBase[1] = rand_range(0, 16383);
	attr.stManual.WarpBase[2] = rand_range(0, 16383);
	attr.stManual.WarpBase[3] = rand_range(0, 16383);
	attr.stManual.WarpBase[4] = rand_range(0, 16383);
	attr.stManual.WarpBase[5] = rand_range(0, 16383);
	attr.stManual.WarpBase[6] = rand_range(0, 16383);
	attr.stManual.WarpBase[7] = rand_range(0, 16383);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpBase[0] = rand_range(16384, 65535);
	attr.stManual.WarpBase[1] = rand_range(16384, 65535);
	attr.stManual.WarpBase[2] = rand_range(16384, 65535);
	attr.stManual.WarpBase[3] = rand_range(16384, 65535);
	attr.stManual.WarpBase[4] = rand_range(16384, 65535);
	attr.stManual.WarpBase[5] = rand_range(16384, 65535);
	attr.stManual.WarpBase[6] = rand_range(16384, 65535);
	attr.stManual.WarpBase[7] = rand_range(16384, 65535);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.WarpBase[0] = 15;
	attr.stManual.WarpBase[1] = 25;
	attr.stManual.WarpBase[2] = 19;
	attr.stManual.WarpBase[3] = 27;
	attr.stManual.WarpBase[4] = 29;
	attr.stManual.WarpBase[5] = 67;
	attr.stManual.WarpBase[6] = 140;
	attr.stManual.WarpBase[7] = 140;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpBase_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpBase_manual);
}

static int test_TNR_TNRMvAttr_WarpBase_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
	if (memcmp(default_val_0, attr.stAuto.WarpBase[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25 };
	if (memcmp(default_val_1, attr.stAuto.WarpBase[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19 };
	if (memcmp(default_val_2, attr.stAuto.WarpBase[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27 };
	if (memcmp(default_val_3, attr.stAuto.WarpBase[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29 };
	if (memcmp(default_val_4, attr.stAuto.WarpBase[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 67,67,67,67,67,67,67,67,67,67,67,67,67,67,67,67 };
	if (memcmp(default_val_5, attr.stAuto.WarpBase[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140 };
	if (memcmp(default_val_6, attr.stAuto.WarpBase[6], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140 };
	if (memcmp(default_val_7, attr.stAuto.WarpBase[7], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpBase[0][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[1][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[2][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[3][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[4][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[5][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[6][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[7][j] = 16383;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpBase[0][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[1][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[2][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[3][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[4][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[5][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[6][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[7][j] = rand_range(0, 16383);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpBase[0][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[1][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[2][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[3][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[4][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[5][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[6][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpBase[7][j] = rand_range(16384, 65535);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
	memcpy(attr.stAuto.WarpBase[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25 };
	memcpy(attr.stAuto.WarpBase[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19 };
	memcpy(attr.stAuto.WarpBase[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27 };
	memcpy(attr.stAuto.WarpBase[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29 };
	memcpy(attr.stAuto.WarpBase[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 67,67,67,67,67,67,67,67,67,67,67,67,67,67,67,67 };
	memcpy(attr.stAuto.WarpBase[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140 };
	memcpy(attr.stAuto.WarpBase[6], restore_val_6, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140 };
	memcpy(attr.stAuto.WarpBase[7], restore_val_7, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpBase_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpBase_auto);
}

static int test_TNR_TNRMvAttr_WarpStd_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.WarpStd[0] != 78) {
		TEST_FAIL("test default value: 78 != %d\n", (int)attr.stManual.WarpStd[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStd[1] != 80) {
		TEST_FAIL("test default value: 80 != %d\n", (int)attr.stManual.WarpStd[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStd[2] != 85) {
		TEST_FAIL("test default value: 85 != %d\n", (int)attr.stManual.WarpStd[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStd[3] != 79) {
		TEST_FAIL("test default value: 79 != %d\n", (int)attr.stManual.WarpStd[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStd[4] != 74) {
		TEST_FAIL("test default value: 74 != %d\n", (int)attr.stManual.WarpStd[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStd[5] != 61) {
		TEST_FAIL("test default value: 61 != %d\n", (int)attr.stManual.WarpStd[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStd[6] != 58) {
		TEST_FAIL("test default value: 58 != %d\n", (int)attr.stManual.WarpStd[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStd[7] != 58) {
		TEST_FAIL("test default value: 58 != %d\n", (int)attr.stManual.WarpStd[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.WarpStd[0] = 0;
	attr.stManual.WarpStd[1] = 0;
	attr.stManual.WarpStd[2] = 0;
	attr.stManual.WarpStd[3] = 0;
	attr.stManual.WarpStd[4] = 0;
	attr.stManual.WarpStd[5] = 0;
	attr.stManual.WarpStd[6] = 0;
	attr.stManual.WarpStd[7] = 0;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpStd[0] = 16383;
	attr.stManual.WarpStd[1] = 16383;
	attr.stManual.WarpStd[2] = 16383;
	attr.stManual.WarpStd[3] = 16383;
	attr.stManual.WarpStd[4] = 16383;
	attr.stManual.WarpStd[5] = 16383;
	attr.stManual.WarpStd[6] = 16383;
	attr.stManual.WarpStd[7] = 16383;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpStd[0] = rand_range(0, 16383);
	attr.stManual.WarpStd[1] = rand_range(0, 16383);
	attr.stManual.WarpStd[2] = rand_range(0, 16383);
	attr.stManual.WarpStd[3] = rand_range(0, 16383);
	attr.stManual.WarpStd[4] = rand_range(0, 16383);
	attr.stManual.WarpStd[5] = rand_range(0, 16383);
	attr.stManual.WarpStd[6] = rand_range(0, 16383);
	attr.stManual.WarpStd[7] = rand_range(0, 16383);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
	attr.stManual.WarpStd[0] = rand_range(16384, 65535);
	attr.stManual.WarpStd[1] = rand_range(16384, 65535);
	attr.stManual.WarpStd[2] = rand_range(16384, 65535);
	attr.stManual.WarpStd[3] = rand_range(16384, 65535);
	attr.stManual.WarpStd[4] = rand_range(16384, 65535);
	attr.stManual.WarpStd[5] = rand_range(16384, 65535);
	attr.stManual.WarpStd[6] = rand_range(16384, 65535);
	attr.stManual.WarpStd[7] = rand_range(16384, 65535);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.WarpStd[0] = 78;
	attr.stManual.WarpStd[1] = 80;
	attr.stManual.WarpStd[2] = 85;
	attr.stManual.WarpStd[3] = 79;
	attr.stManual.WarpStd[4] = 74;
	attr.stManual.WarpStd[5] = 61;
	attr.stManual.WarpStd[6] = 58;
	attr.stManual.WarpStd[7] = 58;
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpStd_manual(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpStd_manual);
}

static int test_TNR_TNRMvAttr_WarpStd_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_MV_ATTR_S attr;

	ret = CVI_ISP_GetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRMvAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78 };
	if (memcmp(default_val_0, attr.stAuto.WarpStd[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80 };
	if (memcmp(default_val_1, attr.stAuto.WarpStd[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85 };
	if (memcmp(default_val_2, attr.stAuto.WarpStd[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79 };
	if (memcmp(default_val_3, attr.stAuto.WarpStd[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74 };
	if (memcmp(default_val_4, attr.stAuto.WarpStd[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61 };
	if (memcmp(default_val_5, attr.stAuto.WarpStd[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58 };
	if (memcmp(default_val_6, attr.stAuto.WarpStd[6], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58 };
	if (memcmp(default_val_7, attr.stAuto.WarpStd[7], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpStd[0][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[1][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[2][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[3][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[4][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[5][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[6][j] = 16383;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[7][j] = 16383;
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpStd[0][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[1][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[2][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[3][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[4][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[5][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[6][j] = rand_range(0, 16383);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[7][j] = rand_range(0, 16383);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
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
		attr.stAuto.WarpStd[0][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[1][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[2][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[3][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[4][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[5][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[6][j] = rand_range(16384, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStd[7][j] = rand_range(16384, 65535);
	}
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78 };
	memcpy(attr.stAuto.WarpStd[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80 };
	memcpy(attr.stAuto.WarpStd[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85 };
	memcpy(attr.stAuto.WarpStd[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79 };
	memcpy(attr.stAuto.WarpStd[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74 };
	memcpy(attr.stAuto.WarpStd[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61 };
	memcpy(attr.stAuto.WarpStd[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58 };
	memcpy(attr.stAuto.WarpStd[6], restore_val_6, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58 };
	memcpy(attr.stAuto.WarpStd[7], restore_val_7, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRMvAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRMvAttr_WarpStd_auto(void)
{
	reg_api_test_fn(test_TNR_TNRMvAttr_WarpStd_auto);
}

static int test_TNR_TNRPsAttr_SadDebiasEN(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.SadDebiasEN != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.SadDebiasEN);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.SadDebiasEN = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.SadDebiasEN = 1;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.SadDebiasEN = rand_range(0, 1);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.SadDebiasEN = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_SadDebiasEN(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_SadDebiasEN);
}

static int test_TNR_TNRPsAttr_SadLpfMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.SadLpfMode != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.SadLpfMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.SadLpfMode = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.SadLpfMode = 3;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.SadLpfMode = rand_range(0, 3);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.SadLpfMode = rand_range(4, 255);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.SadLpfMode = 2;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_SadLpfMode(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_SadLpfMode);
}

static int test_TNR_TNRPsAttr_JndStdRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.JndStdRange[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.JndStdRange[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.JndStdRange[1] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.JndStdRange[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.JndStdRange[0] = 0;
	attr.stManual.JndStdRange[1] = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.JndStdRange[0] = 32;
	attr.stManual.JndStdRange[1] = 32;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.JndStdRange[0] = rand_range(0, 32);
	attr.stManual.JndStdRange[1] = rand_range(0, 32);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.JndStdRange[0] = rand_range(33, 255);
	attr.stManual.JndStdRange[1] = rand_range(33, 255);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.JndStdRange[0] = 0;
	attr.stManual.JndStdRange[1] = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_JndStdRange_manual(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_JndStdRange_manual);
}

static int test_TNR_TNRPsAttr_JndStdRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.JndStdRange[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_1, attr.stAuto.JndStdRange[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.JndStdRange[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.JndStdRange[1][j] = 0;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.JndStdRange[0][j] = 32;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.JndStdRange[1][j] = 32;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.JndStdRange[0][j] = rand_range(0, 32);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.JndStdRange[1][j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.JndStdRange[0][j] = rand_range(33, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.JndStdRange[1][j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.JndStdRange[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.JndStdRange[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_JndStdRange_auto(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_JndStdRange_auto);
}

static int test_TNR_TNRPsAttr_WarpStdRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.WarpStdRange[0] != 12) {
		TEST_FAIL("test default value: 12 != %d\n", (int)attr.stManual.WarpStdRange[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.WarpStdRange[1] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.WarpStdRange[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.WarpStdRange[0] = 0;
	attr.stManual.WarpStdRange[1] = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.WarpStdRange[0] = 32;
	attr.stManual.WarpStdRange[1] = 32;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.WarpStdRange[0] = rand_range(0, 32);
	attr.stManual.WarpStdRange[1] = rand_range(0, 32);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.WarpStdRange[0] = rand_range(33, 255);
	attr.stManual.WarpStdRange[1] = rand_range(33, 255);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.WarpStdRange[0] = 12;
	attr.stManual.WarpStdRange[1] = 20;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_WarpStdRange_manual(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_WarpStdRange_manual);
}

static int test_TNR_TNRPsAttr_WarpStdRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	if (memcmp(default_val_0, attr.stAuto.WarpStdRange[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_1, attr.stAuto.WarpStdRange[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStdRange[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStdRange[1][j] = 0;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.WarpStdRange[0][j] = 32;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStdRange[1][j] = 32;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.WarpStdRange[0][j] = rand_range(0, 32);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStdRange[1][j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.WarpStdRange[0][j] = rand_range(33, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.WarpStdRange[1][j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	memcpy(attr.stAuto.WarpStdRange[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.WarpStdRange[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_WarpStdRange_auto(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_WarpStdRange_auto);
}

static int test_TNR_TNRPsAttr_NbrBldRatio_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.NbrBldRatio[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.NbrBldRatio[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.NbrBldRatio[1] != 32) {
		TEST_FAIL("test default value: 32 != %d\n", (int)attr.stManual.NbrBldRatio[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.NbrBldRatio[0] = 0;
	attr.stManual.NbrBldRatio[1] = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.NbrBldRatio[0] = 255;
	attr.stManual.NbrBldRatio[1] = 255;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.NbrBldRatio[0] = rand_range(0, 255);
	attr.stManual.NbrBldRatio[1] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.NbrBldRatio[0] = 16;
	attr.stManual.NbrBldRatio[1] = 32;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_NbrBldRatio_manual(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_NbrBldRatio_manual);
}

static int test_TNR_TNRPsAttr_NbrBldRatio_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.NbrBldRatio[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	if (memcmp(default_val_1, attr.stAuto.NbrBldRatio[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NbrBldRatio[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NbrBldRatio[1][j] = 0;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.NbrBldRatio[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NbrBldRatio[1][j] = 255;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.NbrBldRatio[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NbrBldRatio[1][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.NbrBldRatio[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	memcpy(attr.stAuto.NbrBldRatio[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_NbrBldRatio_auto(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_NbrBldRatio_auto);
}

static int test_TNR_TNRPsAttr_StillRegionTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.StillRegionTh[0] != 32) {
		TEST_FAIL("test default value: 32 != %d\n", (int)attr.stManual.StillRegionTh[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.StillRegionTh[1] != 1024) {
		TEST_FAIL("test default value: 1024 != %d\n", (int)attr.stManual.StillRegionTh[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.StillRegionTh[0] = 0;
	attr.stManual.StillRegionTh[1] = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.StillRegionTh[0] = 65535;
	attr.stManual.StillRegionTh[1] = 65535;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.StillRegionTh[0] = rand_range(0, 65535);
	attr.stManual.StillRegionTh[1] = rand_range(0, 65535);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.StillRegionTh[0] = 32;
	attr.stManual.StillRegionTh[1] = 1024;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_StillRegionTh_manual(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_StillRegionTh_manual);
}

static int test_TNR_TNRPsAttr_StillRegionTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	if (memcmp(default_val_0, attr.stAuto.StillRegionTh[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024 };
	if (memcmp(default_val_1, attr.stAuto.StillRegionTh[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.StillRegionTh[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.StillRegionTh[1][j] = 0;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.StillRegionTh[0][j] = 65535;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.StillRegionTh[1][j] = 65535;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.StillRegionTh[0][j] = rand_range(0, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.StillRegionTh[1][j] = rand_range(0, 65535);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	memcpy(attr.stAuto.StillRegionTh[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024 };
	memcpy(attr.stAuto.StillRegionTh[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_StillRegionTh_auto(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_StillRegionTh_auto);
}

static int test_TNR_TNRPsAttr_FlowJndTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FlowJndTh != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.FlowJndTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FlowJndTh = 0;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.FlowJndTh = 63;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.FlowJndTh = rand_range(0, 63);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
	attr.stManual.FlowJndTh = rand_range(64, 255);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.FlowJndTh = 5;
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_FlowJndTh_manual(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_FlowJndTh_manual);
}

static int test_TNR_TNRPsAttr_FlowJndTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_PS_ATTR_S attr;

	ret = CVI_ISP_GetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRPsAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_0, attr.stAuto.FlowJndTh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FlowJndTh[j] = 0;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.FlowJndTh[j] = 63;
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.FlowJndTh[j] = rand_range(0, 63);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
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
		attr.stAuto.FlowJndTh[j] = rand_range(64, 255);
	}
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.FlowJndTh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRPsAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRPsAttr_FlowJndTh_auto(void)
{
	reg_api_test_fn(test_TNR_TNRPsAttr_FlowJndTh_auto);
}

static int test_TNR_TNRNrAttr_L12NrYEN(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.L12NrYEN != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.L12NrYEN);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.L12NrYEN = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L12NrYEN = 1;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L12NrYEN = rand_range(0, 1);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L12NrYEN = 1;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L12NrYEN(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L12NrYEN);
}

static int test_TNR_TNRNrAttr_L2UVNrEN(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.L2UVNrEN != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.L2UVNrEN);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.L2UVNrEN = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L2UVNrEN = 1;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L2UVNrEN = rand_range(0, 1);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L2UVNrEN = 1;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2UVNrEN(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2UVNrEN);
}

static int test_TNR_TNRNrAttr_L2UVMonoEN(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.L2UVMonoEN != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.L2UVMonoEN);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.L2UVMonoEN = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L2UVMonoEN = 1;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L2UVMonoEN = rand_range(0, 1);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.L2UVMonoEN = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2UVMonoEN(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2UVMonoEN);
}

static int test_TNR_TNRNrAttr_DcEn(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DcEn != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.DcEn);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DcEn = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.DcEn = 1;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.DcEn = rand_range(0, 1);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.DcEn = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_DcEn(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_DcEn);
}

static int test_TNR_TNRNrAttr_L0YBlurStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L0YBlurStr[0] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.L0YBlurStr[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L0YBlurStr[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.L0YBlurStr[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L0YBlurStr[2] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.L0YBlurStr[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L0YBlurStr[0] = 0;
	attr.stManual.L0YBlurStr[1] = 0;
	attr.stManual.L0YBlurStr[2] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YBlurStr[0] = 255;
	attr.stManual.L0YBlurStr[1] = 255;
	attr.stManual.L0YBlurStr[2] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YBlurStr[0] = rand_range(0, 255);
	attr.stManual.L0YBlurStr[1] = rand_range(0, 255);
	attr.stManual.L0YBlurStr[2] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YBlurStr[0] = 8;
	attr.stManual.L0YBlurStr[1] = 8;
	attr.stManual.L0YBlurStr[2] = 8;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YBlurStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YBlurStr_manual);
}

static int test_TNR_TNRNrAttr_L0YBlurStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.L0YBlurStr[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.L0YBlurStr[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_2, attr.stAuto.L0YBlurStr[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YBlurStr[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YBlurStr[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YBlurStr[2][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YBlurStr[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YBlurStr[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YBlurStr[2][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YBlurStr[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YBlurStr[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YBlurStr[2][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.L0YBlurStr[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.L0YBlurStr[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.L0YBlurStr[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YBlurStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YBlurStr_auto);
}

static int test_TNR_TNRNrAttr_L0YSigmaStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L0YSigmaStr[0] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L0YSigmaStr[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L0YSigmaStr[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L0YSigmaStr[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L0YSigmaStr[2] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L0YSigmaStr[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L0YSigmaStr[0] = 0;
	attr.stManual.L0YSigmaStr[1] = 0;
	attr.stManual.L0YSigmaStr[2] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YSigmaStr[0] = 255;
	attr.stManual.L0YSigmaStr[1] = 255;
	attr.stManual.L0YSigmaStr[2] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YSigmaStr[0] = rand_range(0, 255);
	attr.stManual.L0YSigmaStr[1] = rand_range(0, 255);
	attr.stManual.L0YSigmaStr[2] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YSigmaStr[0] = 64;
	attr.stManual.L0YSigmaStr[1] = 64;
	attr.stManual.L0YSigmaStr[2] = 64;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YSigmaStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YSigmaStr_manual);
}

static int test_TNR_TNRNrAttr_L0YSigmaStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.L0YSigmaStr[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.L0YSigmaStr[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_2, attr.stAuto.L0YSigmaStr[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YSigmaStr[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YSigmaStr[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YSigmaStr[2][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YSigmaStr[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YSigmaStr[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YSigmaStr[2][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YSigmaStr[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YSigmaStr[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YSigmaStr[2][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L0YSigmaStr[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L0YSigmaStr[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L0YSigmaStr[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YSigmaStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YSigmaStr_auto);
}

static int test_TNR_TNRNrAttr_L0UVSigmaStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L0UVSigmaStr[0] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.L0UVSigmaStr[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L0UVSigmaStr[1] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.L0UVSigmaStr[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L0UVSigmaStr[2] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.L0UVSigmaStr[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L0UVSigmaStr[0] = 0;
	attr.stManual.L0UVSigmaStr[1] = 0;
	attr.stManual.L0UVSigmaStr[2] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0UVSigmaStr[0] = 255;
	attr.stManual.L0UVSigmaStr[1] = 255;
	attr.stManual.L0UVSigmaStr[2] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0UVSigmaStr[0] = rand_range(0, 255);
	attr.stManual.L0UVSigmaStr[1] = rand_range(0, 255);
	attr.stManual.L0UVSigmaStr[2] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0UVSigmaStr[0] = 128;
	attr.stManual.L0UVSigmaStr[1] = 20;
	attr.stManual.L0UVSigmaStr[2] = 20;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0UVSigmaStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0UVSigmaStr_manual);
}

static int test_TNR_TNRNrAttr_L0UVSigmaStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.L0UVSigmaStr[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_1, attr.stAuto.L0UVSigmaStr[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_2, attr.stAuto.L0UVSigmaStr[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0UVSigmaStr[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0UVSigmaStr[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0UVSigmaStr[2][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0UVSigmaStr[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0UVSigmaStr[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0UVSigmaStr[2][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0UVSigmaStr[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0UVSigmaStr[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0UVSigmaStr[2][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.L0UVSigmaStr[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.L0UVSigmaStr[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.L0UVSigmaStr[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0UVSigmaStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0UVSigmaStr_auto);
}

static int test_TNR_TNRNrAttr_L0YGardBlurTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L0YGardBlurTh != 512) {
		TEST_FAIL("test default value: 512 != %d\n", (int)attr.stManual.L0YGardBlurTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L0YGardBlurTh = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YGardBlurTh = 65535;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YGardBlurTh = rand_range(0, 65535);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YGardBlurTh = 512;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YGardBlurTh_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YGardBlurTh_manual);
}

static int test_TNR_TNRNrAttr_L0YGardBlurTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512 };
	if (memcmp(default_val_0, attr.stAuto.L0YGardBlurTh, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YGardBlurTh[j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YGardBlurTh[j] = 65535;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YGardBlurTh[j] = rand_range(0, 65535);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512 };
	memcpy(attr.stAuto.L0YGardBlurTh, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YGardBlurTh_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YGardBlurTh_auto);
}

static int test_TNR_TNRNrAttr_L0YGardBlurStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L0YGardBlurStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.L0YGardBlurStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L0YGardBlurStr = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YGardBlurStr = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YGardBlurStr = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L0YGardBlurStr = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YGardBlurStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YGardBlurStr_manual);
}

static int test_TNR_TNRNrAttr_L0YGardBlurStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.L0YGardBlurStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L0YGardBlurStr[j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YGardBlurStr[j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L0YGardBlurStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.L0YGardBlurStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L0YGardBlurStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L0YGardBlurStr_auto);
}

static int test_TNR_TNRNrAttr_L1PFTStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1PFTStr[0] != 120) {
		TEST_FAIL("test default value: 120 != %d\n", (int)attr.stManual.L1PFTStr[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1PFTStr[1] != 130) {
		TEST_FAIL("test default value: 130 != %d\n", (int)attr.stManual.L1PFTStr[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1PFTStr[2] != 140) {
		TEST_FAIL("test default value: 140 != %d\n", (int)attr.stManual.L1PFTStr[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1PFTStr[3] != 150) {
		TEST_FAIL("test default value: 150 != %d\n", (int)attr.stManual.L1PFTStr[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1PFTStr[0] = 0;
	attr.stManual.L1PFTStr[1] = 0;
	attr.stManual.L1PFTStr[2] = 0;
	attr.stManual.L1PFTStr[3] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1PFTStr[0] = 255;
	attr.stManual.L1PFTStr[1] = 255;
	attr.stManual.L1PFTStr[2] = 255;
	attr.stManual.L1PFTStr[3] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1PFTStr[0] = rand_range(0, 255);
	attr.stManual.L1PFTStr[1] = rand_range(0, 255);
	attr.stManual.L1PFTStr[2] = rand_range(0, 255);
	attr.stManual.L1PFTStr[3] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1PFTStr[0] = 120;
	attr.stManual.L1PFTStr[1] = 130;
	attr.stManual.L1PFTStr[2] = 140;
	attr.stManual.L1PFTStr[3] = 150;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1PFTStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1PFTStr_manual);
}

static int test_TNR_TNRNrAttr_L1PFTStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120 };
	if (memcmp(default_val_0, attr.stAuto.L1PFTStr[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 130,130,130,130,130,130,130,130,130,130,130,130,130,130,130,130 };
	if (memcmp(default_val_1, attr.stAuto.L1PFTStr[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140 };
	if (memcmp(default_val_2, attr.stAuto.L1PFTStr[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	if (memcmp(default_val_3, attr.stAuto.L1PFTStr[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[3][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1PFTStr[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[3][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1PFTStr[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTStr[3][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120 };
	memcpy(attr.stAuto.L1PFTStr[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 130,130,130,130,130,130,130,130,130,130,130,130,130,130,130,130 };
	memcpy(attr.stAuto.L1PFTStr[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140 };
	memcpy(attr.stAuto.L1PFTStr[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	memcpy(attr.stAuto.L1PFTStr[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1PFTStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1PFTStr_auto);
}

static int test_TNR_TNRNrAttr_L1SpaGlobalStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1SpaGlobalStr != 1000) {
		TEST_FAIL("test default value: 1000 != %d\n", (int)attr.stManual.L1SpaGlobalStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1SpaGlobalStr = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1SpaGlobalStr = 65535;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1SpaGlobalStr = rand_range(0, 65535);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1SpaGlobalStr = 1000;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1SpaGlobalStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1SpaGlobalStr_manual);
}

static int test_TNR_TNRNrAttr_L1SpaGlobalStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000 };
	if (memcmp(default_val_0, attr.stAuto.L1SpaGlobalStr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1SpaGlobalStr[j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1SpaGlobalStr[j] = 65535;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1SpaGlobalStr[j] = rand_range(0, 65535);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000 };
	memcpy(attr.stAuto.L1SpaGlobalStr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1SpaGlobalStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1SpaGlobalStr_auto);
}

static int test_TNR_TNRNrAttr_L1TmpGlobalStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1TmpGlobalStr != 3000) {
		TEST_FAIL("test default value: 3000 != %d\n", (int)attr.stManual.L1TmpGlobalStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1TmpGlobalStr = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1TmpGlobalStr = 65535;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1TmpGlobalStr = rand_range(0, 65535);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1TmpGlobalStr = 3000;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1TmpGlobalStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1TmpGlobalStr_manual);
}

static int test_TNR_TNRNrAttr_L1TmpGlobalStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000 };
	if (memcmp(default_val_0, attr.stAuto.L1TmpGlobalStr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1TmpGlobalStr[j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1TmpGlobalStr[j] = 65535;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1TmpGlobalStr[j] = rand_range(0, 65535);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000 };
	memcpy(attr.stAuto.L1TmpGlobalStr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1TmpGlobalStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1TmpGlobalStr_auto);
}

static int test_TNR_TNRNrAttr_L1LumaAdj_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1LumaAdj[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1LumaAdj[1] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1LumaAdj[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1LumaAdj[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1LumaAdj[4] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1LumaAdj[5] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1LumaAdj[6] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1LumaAdj[7] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1LumaAdj[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1LumaAdj[0] = 0;
	attr.stManual.L1LumaAdj[1] = 0;
	attr.stManual.L1LumaAdj[2] = 0;
	attr.stManual.L1LumaAdj[3] = 0;
	attr.stManual.L1LumaAdj[4] = 0;
	attr.stManual.L1LumaAdj[5] = 0;
	attr.stManual.L1LumaAdj[6] = 0;
	attr.stManual.L1LumaAdj[7] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1LumaAdj[0] = 255;
	attr.stManual.L1LumaAdj[1] = 255;
	attr.stManual.L1LumaAdj[2] = 255;
	attr.stManual.L1LumaAdj[3] = 255;
	attr.stManual.L1LumaAdj[4] = 255;
	attr.stManual.L1LumaAdj[5] = 255;
	attr.stManual.L1LumaAdj[6] = 255;
	attr.stManual.L1LumaAdj[7] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1LumaAdj[0] = rand_range(0, 255);
	attr.stManual.L1LumaAdj[1] = rand_range(0, 255);
	attr.stManual.L1LumaAdj[2] = rand_range(0, 255);
	attr.stManual.L1LumaAdj[3] = rand_range(0, 255);
	attr.stManual.L1LumaAdj[4] = rand_range(0, 255);
	attr.stManual.L1LumaAdj[5] = rand_range(0, 255);
	attr.stManual.L1LumaAdj[6] = rand_range(0, 255);
	attr.stManual.L1LumaAdj[7] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1LumaAdj[0] = 16;
	attr.stManual.L1LumaAdj[1] = 16;
	attr.stManual.L1LumaAdj[2] = 16;
	attr.stManual.L1LumaAdj[3] = 16;
	attr.stManual.L1LumaAdj[4] = 16;
	attr.stManual.L1LumaAdj[5] = 16;
	attr.stManual.L1LumaAdj[6] = 16;
	attr.stManual.L1LumaAdj[7] = 16;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1LumaAdj_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1LumaAdj_manual);
}

static int test_TNR_TNRNrAttr_L1LumaAdj_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.L1LumaAdj[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_1, attr.stAuto.L1LumaAdj[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.L1LumaAdj[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.L1LumaAdj[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_4, attr.stAuto.L1LumaAdj[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_5, attr.stAuto.L1LumaAdj[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_6, attr.stAuto.L1LumaAdj[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_7, attr.stAuto.L1LumaAdj[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1LumaAdj[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[4][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[5][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[6][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[7][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1LumaAdj[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[4][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[5][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[6][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1LumaAdj[7][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1LumaAdj[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1LumaAdj_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1LumaAdj_auto);
}

static int test_TNR_TNRNrAttr_L1FSJndAdj_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1FSJndAdj[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.L1FSJndAdj[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FSJndAdj[1] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.L1FSJndAdj[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FSJndAdj[2] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.L1FSJndAdj[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1FSJndAdj[0] = 0;
	attr.stManual.L1FSJndAdj[1] = 0;
	attr.stManual.L1FSJndAdj[2] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FSJndAdj[0] = 255;
	attr.stManual.L1FSJndAdj[1] = 255;
	attr.stManual.L1FSJndAdj[2] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FSJndAdj[0] = rand_range(0, 255);
	attr.stManual.L1FSJndAdj[1] = rand_range(0, 255);
	attr.stManual.L1FSJndAdj[2] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FSJndAdj[0] = 0;
	attr.stManual.L1FSJndAdj[1] = 0;
	attr.stManual.L1FSJndAdj[2] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FSJndAdj_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FSJndAdj_manual);
}

static int test_TNR_TNRNrAttr_L1FSJndAdj_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.L1FSJndAdj[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_1, attr.stAuto.L1FSJndAdj[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_2, attr.stAuto.L1FSJndAdj[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSJndAdj[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSJndAdj[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSJndAdj[2][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FSJndAdj[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSJndAdj[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSJndAdj[2][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FSJndAdj[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSJndAdj[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSJndAdj[2][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.L1FSJndAdj[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.L1FSJndAdj[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.L1FSJndAdj[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FSJndAdj_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FSJndAdj_auto);
}

static int test_TNR_TNRNrAttr_L1FTJndAdj_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1FTJndAdj[0] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L1FTJndAdj[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FTJndAdj[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L1FTJndAdj[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FTJndAdj[2] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L1FTJndAdj[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1FTJndAdj[0] = 0;
	attr.stManual.L1FTJndAdj[1] = 0;
	attr.stManual.L1FTJndAdj[2] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTJndAdj[0] = 255;
	attr.stManual.L1FTJndAdj[1] = 255;
	attr.stManual.L1FTJndAdj[2] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTJndAdj[0] = rand_range(0, 255);
	attr.stManual.L1FTJndAdj[1] = rand_range(0, 255);
	attr.stManual.L1FTJndAdj[2] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTJndAdj[0] = 64;
	attr.stManual.L1FTJndAdj[1] = 64;
	attr.stManual.L1FTJndAdj[2] = 64;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FTJndAdj_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FTJndAdj_manual);
}

static int test_TNR_TNRNrAttr_L1FTJndAdj_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.L1FTJndAdj[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.L1FTJndAdj[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_2, attr.stAuto.L1FTJndAdj[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTJndAdj[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTJndAdj[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTJndAdj[2][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FTJndAdj[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTJndAdj[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTJndAdj[2][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FTJndAdj[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTJndAdj[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTJndAdj[2][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L1FTJndAdj[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L1FTJndAdj[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L1FTJndAdj[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FTJndAdj_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FTJndAdj_auto);
}

static int test_TNR_TNRNrAttr_L1FSFreAdj_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1FSFreAdj[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FSFreAdj[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FSFreAdj[1] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FSFreAdj[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FSFreAdj[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FSFreAdj[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FSFreAdj[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FSFreAdj[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1FSFreAdj[0] = 0;
	attr.stManual.L1FSFreAdj[1] = 0;
	attr.stManual.L1FSFreAdj[2] = 0;
	attr.stManual.L1FSFreAdj[3] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FSFreAdj[0] = 255;
	attr.stManual.L1FSFreAdj[1] = 255;
	attr.stManual.L1FSFreAdj[2] = 255;
	attr.stManual.L1FSFreAdj[3] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FSFreAdj[0] = rand_range(0, 255);
	attr.stManual.L1FSFreAdj[1] = rand_range(0, 255);
	attr.stManual.L1FSFreAdj[2] = rand_range(0, 255);
	attr.stManual.L1FSFreAdj[3] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FSFreAdj[0] = 16;
	attr.stManual.L1FSFreAdj[1] = 16;
	attr.stManual.L1FSFreAdj[2] = 16;
	attr.stManual.L1FSFreAdj[3] = 16;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FSFreAdj_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FSFreAdj_manual);
}

static int test_TNR_TNRNrAttr_L1FSFreAdj_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.L1FSFreAdj[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_1, attr.stAuto.L1FSFreAdj[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.L1FSFreAdj[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.L1FSFreAdj[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[3][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FSFreAdj[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[3][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FSFreAdj[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FSFreAdj[3][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FSFreAdj[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FSFreAdj[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FSFreAdj[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FSFreAdj[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FSFreAdj_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FSFreAdj_auto);
}

static int test_TNR_TNRNrAttr_L1FTFreAdj_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1FTFreAdj[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FTFreAdj[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FTFreAdj[1] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FTFreAdj[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FTFreAdj[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FTFreAdj[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1FTFreAdj[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L1FTFreAdj[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1FTFreAdj[0] = 0;
	attr.stManual.L1FTFreAdj[1] = 0;
	attr.stManual.L1FTFreAdj[2] = 0;
	attr.stManual.L1FTFreAdj[3] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTFreAdj[0] = 255;
	attr.stManual.L1FTFreAdj[1] = 255;
	attr.stManual.L1FTFreAdj[2] = 255;
	attr.stManual.L1FTFreAdj[3] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTFreAdj[0] = rand_range(0, 255);
	attr.stManual.L1FTFreAdj[1] = rand_range(0, 255);
	attr.stManual.L1FTFreAdj[2] = rand_range(0, 255);
	attr.stManual.L1FTFreAdj[3] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTFreAdj[0] = 16;
	attr.stManual.L1FTFreAdj[1] = 16;
	attr.stManual.L1FTFreAdj[2] = 16;
	attr.stManual.L1FTFreAdj[3] = 16;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FTFreAdj_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FTFreAdj_manual);
}

static int test_TNR_TNRNrAttr_L1FTFreAdj_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.L1FTFreAdj[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_1, attr.stAuto.L1FTFreAdj[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.L1FTFreAdj[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.L1FTFreAdj[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[3][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FTFreAdj[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[3][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FTFreAdj[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTFreAdj[3][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FTFreAdj[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FTFreAdj[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FTFreAdj[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L1FTFreAdj[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FTFreAdj_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FTFreAdj_auto);
}

static int test_TNR_TNRNrAttr_L1FTLumaStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1FTLumaStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.L1FTLumaStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1FTLumaStr = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTLumaStr = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTLumaStr = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1FTLumaStr = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FTLumaStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FTLumaStr_manual);
}

static int test_TNR_TNRNrAttr_L1FTLumaStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.L1FTLumaStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1FTLumaStr[j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FTLumaStr[j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1FTLumaStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.L1FTLumaStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1FTLumaStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1FTLumaStr_auto);
}

static int test_TNR_TNRNrAttr_L1PFTDamp_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L1PFTDamp[0] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.L1PFTDamp[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L1PFTDamp[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L1PFTDamp[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L1PFTDamp[0] = 0;
	attr.stManual.L1PFTDamp[1] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1PFTDamp[0] = 255;
	attr.stManual.L1PFTDamp[1] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1PFTDamp[0] = rand_range(0, 255);
	attr.stManual.L1PFTDamp[1] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L1PFTDamp[0] = 128;
	attr.stManual.L1PFTDamp[1] = 64;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1PFTDamp_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1PFTDamp_manual);
}

static int test_TNR_TNRNrAttr_L1PFTDamp_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.L1PFTDamp[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.L1PFTDamp[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTDamp[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTDamp[1][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1PFTDamp[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTDamp[1][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L1PFTDamp[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L1PFTDamp[1][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.L1PFTDamp[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L1PFTDamp[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L1PFTDamp_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L1PFTDamp_auto);
}

static int test_TNR_TNRNrAttr_L2YDampStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L2YDampStr[0] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.L2YDampStr[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YDampStr[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L2YDampStr[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L2YDampStr[0] = 0;
	attr.stManual.L2YDampStr[1] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2YDampStr[0] = 255;
	attr.stManual.L2YDampStr[1] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2YDampStr[0] = rand_range(0, 255);
	attr.stManual.L2YDampStr[1] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2YDampStr[0] = 128;
	attr.stManual.L2YDampStr[1] = 64;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2YDampStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2YDampStr_manual);
}

static int test_TNR_TNRNrAttr_L2YDampStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.L2YDampStr[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.L2YDampStr[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YDampStr[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YDampStr[1][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2YDampStr[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YDampStr[1][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2YDampStr[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YDampStr[1][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.L2YDampStr[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L2YDampStr[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2YDampStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2YDampStr_auto);
}

static int test_TNR_TNRNrAttr_L2YLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L2YLut[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.L2YLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YLut[1] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.L2YLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YLut[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L2YLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YLut[3] != 30) {
		TEST_FAIL("test default value: 30 != %d\n", (int)attr.stManual.L2YLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YLut[4] != 35) {
		TEST_FAIL("test default value: 35 != %d\n", (int)attr.stManual.L2YLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YLut[5] != 41) {
		TEST_FAIL("test default value: 41 != %d\n", (int)attr.stManual.L2YLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YLut[6] != 49) {
		TEST_FAIL("test default value: 49 != %d\n", (int)attr.stManual.L2YLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2YLut[7] != 57) {
		TEST_FAIL("test default value: 57 != %d\n", (int)attr.stManual.L2YLut[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L2YLut[0] = 0;
	attr.stManual.L2YLut[1] = 0;
	attr.stManual.L2YLut[2] = 0;
	attr.stManual.L2YLut[3] = 0;
	attr.stManual.L2YLut[4] = 0;
	attr.stManual.L2YLut[5] = 0;
	attr.stManual.L2YLut[6] = 0;
	attr.stManual.L2YLut[7] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2YLut[0] = 127;
	attr.stManual.L2YLut[1] = 127;
	attr.stManual.L2YLut[2] = 127;
	attr.stManual.L2YLut[3] = 127;
	attr.stManual.L2YLut[4] = 127;
	attr.stManual.L2YLut[5] = 127;
	attr.stManual.L2YLut[6] = 127;
	attr.stManual.L2YLut[7] = 127;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2YLut[0] = rand_range(0, 127);
	attr.stManual.L2YLut[1] = rand_range(0, 127);
	attr.stManual.L2YLut[2] = rand_range(0, 127);
	attr.stManual.L2YLut[3] = rand_range(0, 127);
	attr.stManual.L2YLut[4] = rand_range(0, 127);
	attr.stManual.L2YLut[5] = rand_range(0, 127);
	attr.stManual.L2YLut[6] = rand_range(0, 127);
	attr.stManual.L2YLut[7] = rand_range(0, 127);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2YLut[0] = rand_range(128, 255);
	attr.stManual.L2YLut[1] = rand_range(128, 255);
	attr.stManual.L2YLut[2] = rand_range(128, 255);
	attr.stManual.L2YLut[3] = rand_range(128, 255);
	attr.stManual.L2YLut[4] = rand_range(128, 255);
	attr.stManual.L2YLut[5] = rand_range(128, 255);
	attr.stManual.L2YLut[6] = rand_range(128, 255);
	attr.stManual.L2YLut[7] = rand_range(128, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.L2YLut[0] = 0;
	attr.stManual.L2YLut[1] = 4;
	attr.stManual.L2YLut[2] = 16;
	attr.stManual.L2YLut[3] = 30;
	attr.stManual.L2YLut[4] = 35;
	attr.stManual.L2YLut[5] = 41;
	attr.stManual.L2YLut[6] = 49;
	attr.stManual.L2YLut[7] = 57;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2YLut_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2YLut_manual);
}

static int test_TNR_TNRNrAttr_L2YLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.L2YLut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_1, attr.stAuto.L2YLut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.L2YLut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	if (memcmp(default_val_3, attr.stAuto.L2YLut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35 };
	if (memcmp(default_val_4, attr.stAuto.L2YLut[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41 };
	if (memcmp(default_val_5, attr.stAuto.L2YLut[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49 };
	if (memcmp(default_val_6, attr.stAuto.L2YLut[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57 };
	if (memcmp(default_val_7, attr.stAuto.L2YLut[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2YLut[0][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[1][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[2][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[3][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[4][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[5][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[6][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[7][j] = 127;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2YLut[0][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[1][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[2][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[3][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[4][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[5][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[6][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[7][j] = rand_range(0, 127);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2YLut[0][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[1][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[2][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[3][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[4][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[5][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[6][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2YLut[7][j] = rand_range(128, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.L2YLut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.L2YLut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L2YLut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	memcpy(attr.stAuto.L2YLut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35 };
	memcpy(attr.stAuto.L2YLut[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41 };
	memcpy(attr.stAuto.L2YLut[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49 };
	memcpy(attr.stAuto.L2YLut[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57 };
	memcpy(attr.stAuto.L2YLut[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2YLut_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2YLut_auto);
}

static int test_TNR_TNRNrAttr_L2UVDampStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L2UVDampStr[0] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.L2UVDampStr[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVDampStr[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.L2UVDampStr[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L2UVDampStr[0] = 0;
	attr.stManual.L2UVDampStr[1] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2UVDampStr[0] = 255;
	attr.stManual.L2UVDampStr[1] = 255;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2UVDampStr[0] = rand_range(0, 255);
	attr.stManual.L2UVDampStr[1] = rand_range(0, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2UVDampStr[0] = 128;
	attr.stManual.L2UVDampStr[1] = 64;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2UVDampStr_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2UVDampStr_manual);
}

static int test_TNR_TNRNrAttr_L2UVDampStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.L2UVDampStr[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.L2UVDampStr[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVDampStr[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVDampStr[1][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2UVDampStr[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVDampStr[1][j] = 255;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2UVDampStr[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVDampStr[1][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.L2UVDampStr[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.L2UVDampStr[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2UVDampStr_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2UVDampStr_auto);
}

static int test_TNR_TNRNrAttr_L2UVLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.L2UVLut[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.L2UVLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVLut[1] != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.L2UVLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVLut[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.L2UVLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVLut[3] != 30) {
		TEST_FAIL("test default value: 30 != %d\n", (int)attr.stManual.L2UVLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVLut[4] != 35) {
		TEST_FAIL("test default value: 35 != %d\n", (int)attr.stManual.L2UVLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVLut[5] != 41) {
		TEST_FAIL("test default value: 41 != %d\n", (int)attr.stManual.L2UVLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVLut[6] != 49) {
		TEST_FAIL("test default value: 49 != %d\n", (int)attr.stManual.L2UVLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.L2UVLut[7] != 57) {
		TEST_FAIL("test default value: 57 != %d\n", (int)attr.stManual.L2UVLut[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.L2UVLut[0] = 0;
	attr.stManual.L2UVLut[1] = 0;
	attr.stManual.L2UVLut[2] = 0;
	attr.stManual.L2UVLut[3] = 0;
	attr.stManual.L2UVLut[4] = 0;
	attr.stManual.L2UVLut[5] = 0;
	attr.stManual.L2UVLut[6] = 0;
	attr.stManual.L2UVLut[7] = 0;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2UVLut[0] = 127;
	attr.stManual.L2UVLut[1] = 127;
	attr.stManual.L2UVLut[2] = 127;
	attr.stManual.L2UVLut[3] = 127;
	attr.stManual.L2UVLut[4] = 127;
	attr.stManual.L2UVLut[5] = 127;
	attr.stManual.L2UVLut[6] = 127;
	attr.stManual.L2UVLut[7] = 127;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2UVLut[0] = rand_range(0, 127);
	attr.stManual.L2UVLut[1] = rand_range(0, 127);
	attr.stManual.L2UVLut[2] = rand_range(0, 127);
	attr.stManual.L2UVLut[3] = rand_range(0, 127);
	attr.stManual.L2UVLut[4] = rand_range(0, 127);
	attr.stManual.L2UVLut[5] = rand_range(0, 127);
	attr.stManual.L2UVLut[6] = rand_range(0, 127);
	attr.stManual.L2UVLut[7] = rand_range(0, 127);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
	attr.stManual.L2UVLut[0] = rand_range(128, 255);
	attr.stManual.L2UVLut[1] = rand_range(128, 255);
	attr.stManual.L2UVLut[2] = rand_range(128, 255);
	attr.stManual.L2UVLut[3] = rand_range(128, 255);
	attr.stManual.L2UVLut[4] = rand_range(128, 255);
	attr.stManual.L2UVLut[5] = rand_range(128, 255);
	attr.stManual.L2UVLut[6] = rand_range(128, 255);
	attr.stManual.L2UVLut[7] = rand_range(128, 255);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.L2UVLut[0] = 0;
	attr.stManual.L2UVLut[1] = 7;
	attr.stManual.L2UVLut[2] = 16;
	attr.stManual.L2UVLut[3] = 30;
	attr.stManual.L2UVLut[4] = 35;
	attr.stManual.L2UVLut[5] = 41;
	attr.stManual.L2UVLut[6] = 49;
	attr.stManual.L2UVLut[7] = 57;
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2UVLut_manual(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2UVLut_manual);
}

static int test_TNR_TNRNrAttr_L2UVLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_TNR_NR_ATTR_S attr;

	ret = CVI_ISP_GetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetTNRNrAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.L2UVLut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_1, attr.stAuto.L2UVLut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.L2UVLut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	if (memcmp(default_val_3, attr.stAuto.L2UVLut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35 };
	if (memcmp(default_val_4, attr.stAuto.L2UVLut[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41 };
	if (memcmp(default_val_5, attr.stAuto.L2UVLut[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49 };
	if (memcmp(default_val_6, attr.stAuto.L2UVLut[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57 };
	if (memcmp(default_val_7, attr.stAuto.L2UVLut[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[7][j] = 0;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2UVLut[0][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[1][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[2][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[3][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[4][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[5][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[6][j] = 127;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[7][j] = 127;
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2UVLut[0][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[1][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[2][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[3][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[4][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[5][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[6][j] = rand_range(0, 127);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[7][j] = rand_range(0, 127);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
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
		attr.stAuto.L2UVLut[0][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[1][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[2][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[3][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[4][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[5][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[6][j] = rand_range(128, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.L2UVLut[7][j] = rand_range(128, 255);
	}
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.L2UVLut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.L2UVLut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.L2UVLut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	memcpy(attr.stAuto.L2UVLut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35 };
	memcpy(attr.stAuto.L2UVLut[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41 };
	memcpy(attr.stAuto.L2UVLut[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49 };
	memcpy(attr.stAuto.L2UVLut[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57 };
	memcpy(attr.stAuto.L2UVLut[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetTNRNrAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_TNR_TNRNrAttr_L2UVLut_auto(void)
{
	reg_api_test_fn(test_TNR_TNRNrAttr_L2UVLut_auto);
}
