
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

static int test_CNR_CNRAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_Enable(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_Enable);
}

static int test_CNR_CNRAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_enOpType(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_enOpType);
}

static int test_CNR_CNRAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.UpdateInterval = rand_range(1, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.UpdateInterval = rand_range(0, 0);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_UpdateInterval);
}

static int test_CNR_CNRAttr_MedianFltEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MedianFltEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.MedianFltEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MedianFltEnable = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.MedianFltEnable = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.MedianFltEnable = rand_range(0, 1);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.MedianFltEnable = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_MedianFltEnable(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_MedianFltEnable);
}

static int test_CNR_CNRAttr_CnrEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CnrEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.CnrEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CnrEnable = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrEnable = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrEnable = rand_range(0, 1);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrEnable = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrEnable(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrEnable);
}

static int test_CNR_CNRAttr_CnrSatBldEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CnrSatBldEnable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.CnrSatBldEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CnrSatBldEnable = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrSatBldEnable = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrSatBldEnable = rand_range(0, 1);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrSatBldEnable = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatBldEnable(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatBldEnable);
}

static int test_CNR_CNRAttr_SubSclSel(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.SubSclSel != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.SubSclSel);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.SubSclSel = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.SubSclSel = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.SubSclSel = rand_range(0, 1);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.SubSclSel = rand_range(2, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.SubSclSel = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_SubSclSel(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_SubSclSel);
}

static int test_CNR_CNRAttr_CnrDebugMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CnrDebugMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.CnrDebugMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CnrDebugMode = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrDebugMode = 6;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrDebugMode = rand_range(0, 6);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrDebugMode = rand_range(7, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.CnrDebugMode = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrDebugMode(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrDebugMode);
}

static int test_CNR_CNRAttr_CnrBypass1(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CnrBypass1 != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.CnrBypass1);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CnrBypass1 = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrBypass1 = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrBypass1 = rand_range(0, 1);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrBypass1 = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrBypass1(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrBypass1);
}

static int test_CNR_CNRAttr_CnrBypass2(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CnrBypass2 != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.CnrBypass2);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CnrBypass2 = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrBypass2 = 1;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrBypass2 = rand_range(0, 1);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.CnrBypass2 = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrBypass2(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrBypass2);
}

static int test_CNR_CNRAttr_SubImgSclFactor(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.SubImgSclFactor != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.SubImgSclFactor);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.SubImgSclFactor = SCALE_FACTOR_4;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.SubImgSclFactor = SCALE_FACTOR_16;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.SubImgSclFactor = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_SubImgSclFactor(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_SubImgSclFactor);
}

static int test_CNR_CNRAttr_MedianFltKsize_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.MedianFltKsize != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.MedianFltKsize);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.MedianFltKsize = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.MedianFltKsize = 2;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.MedianFltKsize = rand_range(0, 2);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.MedianFltKsize = rand_range(3, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.MedianFltKsize = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_MedianFltKsize_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_MedianFltKsize_manual);
}

static int test_CNR_CNRAttr_MedianFltKsize_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.MedianFltKsize, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.MedianFltKsize[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.MedianFltKsize[j] = 2;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.MedianFltKsize[j] = rand_range(0, 2);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.MedianFltKsize[j] = rand_range(3, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.MedianFltKsize, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_MedianFltKsize_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_MedianFltKsize_auto);
}

static int test_CNR_CNRAttr_CnrByMotion_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrByMotion[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrByMotion[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.CnrByMotion[1] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrByMotion[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.CnrByMotion[2] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrByMotion[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrByMotion[0] = 0;
	attr.stManual.CnrByMotion[1] = 0;
	attr.stManual.CnrByMotion[2] = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrByMotion[0] = 31;
	attr.stManual.CnrByMotion[1] = 31;
	attr.stManual.CnrByMotion[2] = 31;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrByMotion[0] = rand_range(0, 31);
	attr.stManual.CnrByMotion[1] = rand_range(0, 31);
	attr.stManual.CnrByMotion[2] = rand_range(0, 31);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrByMotion[0] = rand_range(32, 255);
	attr.stManual.CnrByMotion[1] = rand_range(32, 255);
	attr.stManual.CnrByMotion[2] = rand_range(32, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrByMotion[0] = 0;
	attr.stManual.CnrByMotion[1] = 0;
	attr.stManual.CnrByMotion[2] = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrByMotion_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrByMotion_manual);
}

static int test_CNR_CNRAttr_CnrByMotion_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrByMotion[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_1, attr.stAuto.CnrByMotion[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_2, attr.stAuto.CnrByMotion[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[2][j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrByMotion[0][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[1][j] = 31;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[2][j] = 31;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrByMotion[0][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[1][j] = rand_range(0, 31);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[2][j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrByMotion[0][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[1][j] = rand_range(32, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrByMotion[2][j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrByMotion[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrByMotion[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrByMotion[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrByMotion_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrByMotion_auto);
}

static int test_CNR_CNRAttr_CnrRefYStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrRefYStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrRefYStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrRefYStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrRefYStr = 31;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrRefYStr = rand_range(0, 31);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrRefYStr = rand_range(32, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrRefYStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrRefYStr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrRefYStr_manual);
}

static int test_CNR_CNRAttr_CnrRefYStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrRefYStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrRefYStr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrRefYStr[j] = 31;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrRefYStr[j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrRefYStr[j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrRefYStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrRefYStr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrRefYStr_auto);
}

static int test_CNR_CNRAttr_CnrRefUVStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrRefUVStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrRefUVStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrRefUVStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrRefUVStr = 31;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrRefUVStr = rand_range(0, 31);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrRefUVStr = rand_range(32, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrRefUVStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrRefUVStr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrRefUVStr_manual);
}

static int test_CNR_CNRAttr_CnrRefUVStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrRefUVStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrRefUVStr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrRefUVStr[j] = 31;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrRefUVStr[j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrRefUVStr[j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrRefUVStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrRefUVStr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrRefUVStr_auto);
}

static int test_CNR_CNRAttr_CnrYRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrYRange != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.CnrYRange);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrYRange = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYRange = 3;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYRange = rand_range(0, 3);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYRange = rand_range(4, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrYRange = 2;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYRange_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYRange_manual);
}

static int test_CNR_CNRAttr_CnrYRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.CnrYRange, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrYRange[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYRange[j] = 3;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYRange[j] = rand_range(0, 3);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYRange[j] = rand_range(4, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.CnrYRange, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYRange_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYRange_auto);
}

static int test_CNR_CNRAttr_CnrYThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrYThr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrYThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrYThr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYThr = 31;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYThr = rand_range(0, 31);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYThr = rand_range(32, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrYThr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYThr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYThr_manual);
}

static int test_CNR_CNRAttr_CnrYThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrYThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrYThr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYThr[j] = 31;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYThr[j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYThr[j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrYThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYThr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYThr_auto);
}

static int test_CNR_CNRAttr_CnrYPrcStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrYPrcStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrYPrcStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrYPrcStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYPrcStr = 2;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYPrcStr = rand_range(0, 2);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYPrcStr = rand_range(3, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrYPrcStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYPrcStr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYPrcStr_manual);
}

static int test_CNR_CNRAttr_CnrYPrcStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrYPrcStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrYPrcStr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYPrcStr[j] = 2;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYPrcStr[j] = rand_range(0, 2);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYPrcStr[j] = rand_range(3, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrYPrcStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYPrcStr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYPrcStr_auto);
}

static int test_CNR_CNRAttr_CnrYStrH_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrYStrH != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.CnrYStrH);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrYStrH = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYStrH = 32;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYStrH = rand_range(0, 32);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYStrH = rand_range(33, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrYStrH = 16;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYStrH_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYStrH_manual);
}

static int test_CNR_CNRAttr_CnrYStrH_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.CnrYStrH, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrYStrH[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYStrH[j] = 32;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYStrH[j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYStrH[j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.CnrYStrH, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYStrH_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYStrH_auto);
}

static int test_CNR_CNRAttr_CnrYStrT_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrYStrT != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrYStrT);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrYStrT = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYStrT = 32;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYStrT = rand_range(0, 32);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrYStrT = rand_range(33, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrYStrT = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYStrT_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYStrT_manual);
}

static int test_CNR_CNRAttr_CnrYStrT_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrYStrT, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrYStrT[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYStrT[j] = 32;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYStrT[j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrYStrT[j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrYStrT, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrYStrT_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrYStrT_auto);
}

static int test_CNR_CNRAttr_CnrUVRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrUVRange != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.CnrUVRange);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrUVRange = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVRange = 4;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVRange = rand_range(0, 4);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVRange = rand_range(5, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrUVRange = 2;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVRange_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVRange_manual);
}

static int test_CNR_CNRAttr_CnrUVRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.CnrUVRange, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrUVRange[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVRange[j] = 4;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVRange[j] = rand_range(0, 4);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVRange[j] = rand_range(5, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.CnrUVRange, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVRange_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVRange_auto);
}

static int test_CNR_CNRAttr_CnrUVThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrUVThr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrUVThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrUVThr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVThr = 32;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVThr = rand_range(0, 32);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVThr = rand_range(33, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrUVThr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVThr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVThr_manual);
}

static int test_CNR_CNRAttr_CnrUVThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrUVThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrUVThr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVThr[j] = 32;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVThr[j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVThr[j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrUVThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVThr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVThr_auto);
}

static int test_CNR_CNRAttr_CnrUVPrcStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrUVPrcStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrUVPrcStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrUVPrcStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVPrcStr = 3;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVPrcStr = rand_range(0, 3);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVPrcStr = rand_range(4, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrUVPrcStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVPrcStr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVPrcStr_manual);
}

static int test_CNR_CNRAttr_CnrUVPrcStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrUVPrcStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrUVPrcStr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVPrcStr[j] = 3;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVPrcStr[j] = rand_range(0, 3);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVPrcStr[j] = rand_range(4, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrUVPrcStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVPrcStr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVPrcStr_auto);
}

static int test_CNR_CNRAttr_CnrUVStrH_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrUVStrH != 32) {
		TEST_FAIL("test default value: 32 != %d\n", (int)attr.stManual.CnrUVStrH);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrUVStrH = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVStrH = 64;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVStrH = rand_range(0, 64);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVStrH = rand_range(65, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrUVStrH = 32;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVStrH_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVStrH_manual);
}

static int test_CNR_CNRAttr_CnrUVStrH_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	if (memcmp(default_val_0, attr.stAuto.CnrUVStrH, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrUVStrH[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVStrH[j] = 64;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVStrH[j] = rand_range(0, 64);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVStrH[j] = rand_range(65, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	memcpy(attr.stAuto.CnrUVStrH, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVStrH_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVStrH_auto);
}

static int test_CNR_CNRAttr_CnrUVStrT_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrUVStrT != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrUVStrT);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrUVStrT = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVStrT = 64;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVStrT = rand_range(0, 64);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrUVStrT = rand_range(65, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrUVStrT = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVStrT_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVStrT_manual);
}

static int test_CNR_CNRAttr_CnrUVStrT_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrUVStrT, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrUVStrT[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVStrT[j] = 64;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVStrT[j] = rand_range(0, 64);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrUVStrT[j] = rand_range(65, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrUVStrT, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrUVStrT_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrUVStrT_auto);
}

static int test_CNR_CNRAttr_CnrSatRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrSatRange != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.CnrSatRange);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrSatRange = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatRange = 3;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatRange = rand_range(0, 3);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatRange = rand_range(4, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrSatRange = 2;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatRange_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatRange_manual);
}

static int test_CNR_CNRAttr_CnrSatRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.CnrSatRange, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrSatRange[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatRange[j] = 3;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatRange[j] = rand_range(0, 3);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatRange[j] = rand_range(4, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.CnrSatRange, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatRange_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatRange_auto);
}

static int test_CNR_CNRAttr_CnrSatPrtThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrSatPrtThr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrSatPrtThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrSatPrtThr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatPrtThr = 31;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatPrtThr = rand_range(0, 31);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatPrtThr = rand_range(32, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrSatPrtThr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatPrtThr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatPrtThr_manual);
}

static int test_CNR_CNRAttr_CnrSatPrtThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrSatPrtThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrSatPrtThr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatPrtThr[j] = 31;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatPrtThr[j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatPrtThr[j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrSatPrtThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatPrtThr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatPrtThr_auto);
}

static int test_CNR_CNRAttr_CnrSatPrcStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrSatPrcStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrSatPrcStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrSatPrcStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatPrcStr = 2;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatPrcStr = rand_range(0, 2);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatPrcStr = rand_range(3, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrSatPrcStr = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatPrcStr_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatPrcStr_manual);
}

static int test_CNR_CNRAttr_CnrSatPrcStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrSatPrcStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrSatPrcStr[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatPrcStr[j] = 2;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatPrcStr[j] = rand_range(0, 2);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatPrcStr[j] = rand_range(3, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrSatPrcStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatPrcStr_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatPrcStr_auto);
}

static int test_CNR_CNRAttr_CnrSatStrH_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrSatStrH != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.CnrSatStrH);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrSatStrH = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatStrH = 32;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatStrH = rand_range(0, 32);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatStrH = rand_range(33, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrSatStrH = 16;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatStrH_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatStrH_manual);
}

static int test_CNR_CNRAttr_CnrSatStrH_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.CnrSatStrH, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrSatStrH[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatStrH[j] = 32;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatStrH[j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatStrH[j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.CnrSatStrH, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatStrH_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatStrH_auto);
}

static int test_CNR_CNRAttr_CnrSatStrT_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrSatStrT != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrSatStrT);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrSatStrT = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatStrT = 32;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatStrT = rand_range(0, 32);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatStrT = rand_range(33, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrSatStrT = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatStrT_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatStrT_manual);
}

static int test_CNR_CNRAttr_CnrSatStrT_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrSatStrT, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrSatStrT[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatStrT[j] = 32;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatStrT[j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatStrT[j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrSatStrT, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatStrT_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatStrT_auto);
}

static int test_CNR_CNRAttr_CnrSatBldCoring_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CnrSatBldCoring != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.CnrSatBldCoring);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CnrSatBldCoring = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatBldCoring = 32;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatBldCoring = rand_range(0, 32);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
	attr.stManual.CnrSatBldCoring = rand_range(33, 255);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CnrSatBldCoring = 0;
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatBldCoring_manual(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatBldCoring_manual);
}

static int test_CNR_CNRAttr_CnrSatBldCoring_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_ATTR_S attr;

	ret = CVI_ISP_GetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.CnrSatBldCoring, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CnrSatBldCoring[j] = 0;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatBldCoring[j] = 32;
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatBldCoring[j] = rand_range(0, 32);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
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
		attr.stAuto.CnrSatBldCoring[j] = rand_range(33, 255);
	}
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.CnrSatBldCoring, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRAttr_CnrSatBldCoring_auto(void)
{
	reg_api_test_fn(test_CNR_CNRAttr_CnrSatBldCoring_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.SubImgNrEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.SubImgNrEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.SubImgNrEnable = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.SubImgNrEnable = 1;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.SubImgNrEnable = rand_range(0, 1);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.SubImgNrEnable = 1;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrEnable(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrEnable);
}

static int test_CNR_CNRFilterAttr_YNrEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.YNrEnable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.YNrEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.YNrEnable = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.YNrEnable = 1;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.YNrEnable = rand_range(0, 1);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.YNrEnable = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_YNrEnable(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_YNrEnable);
}

static int test_CNR_CNRFilterAttr_EdgeDebugEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.EdgeDebugEnable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.EdgeDebugEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.EdgeDebugEnable = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.EdgeDebugEnable = 1;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.EdgeDebugEnable = rand_range(0, 1);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.EdgeDebugEnable = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_EdgeDebugEnable(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_EdgeDebugEnable);
}

static int test_CNR_CNRFilterAttr_RefCenterDebugEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.RefCenterDebugEnable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.RefCenterDebugEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.RefCenterDebugEnable = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.RefCenterDebugEnable = 1;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.RefCenterDebugEnable = rand_range(0, 1);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.RefCenterDebugEnable = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_RefCenterDebugEnable(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_RefCenterDebugEnable);
}

static int test_CNR_CNRFilterAttr_SubImgNrFltKsize_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrFltKsize != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SubImgNrFltKsize);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrFltKsize = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrFltKsize = 3;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrFltKsize = rand_range(0, 3);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrFltKsize = rand_range(4, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrFltKsize = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrFltKsize_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrFltKsize_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrFltKsize_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrFltKsize, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrFltKsize[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrFltKsize[j] = 3;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrFltKsize[j] = rand_range(0, 3);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrFltKsize[j] = rand_range(4, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SubImgNrFltKsize, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrFltKsize_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrFltKsize_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrEdgeDirPN_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrEdgeDirPN != 11) {
		TEST_FAIL("test default value: 11 != %d\n", (int)attr.stManual.SubImgNrEdgeDirPN);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrEdgeDirPN = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeDirPN = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeDirPN = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeDirPN = 11;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrEdgeDirPN_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrEdgeDirPN_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrEdgeDirPN_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrEdgeDirPN, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrEdgeDirPN[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrEdgeDirPN[j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrEdgeDirPN[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11 };
	memcpy(attr.stAuto.SubImgNrEdgeDirPN, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrEdgeDirPN_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrEdgeDirPN_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrEdgeDirHV_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrEdgeDirHV != 11) {
		TEST_FAIL("test default value: 11 != %d\n", (int)attr.stManual.SubImgNrEdgeDirHV);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrEdgeDirHV = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeDirHV = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeDirHV = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeDirHV = 11;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrEdgeDirHV_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrEdgeDirHV_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrEdgeDirHV_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrEdgeDirHV, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrEdgeDirHV[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrEdgeDirHV[j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrEdgeDirHV[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11 };
	memcpy(attr.stAuto.SubImgNrEdgeDirHV, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrEdgeDirHV_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrEdgeDirHV_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrEdgeKsize_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrEdgeKsize != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SubImgNrEdgeKsize);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrEdgeKsize = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeKsize = 1;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeKsize = rand_range(0, 1);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrEdgeKsize = rand_range(2, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrEdgeKsize = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrEdgeKsize_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrEdgeKsize_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrEdgeKsize_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrEdgeKsize, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrEdgeKsize[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrEdgeKsize[j] = 1;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrEdgeKsize[j] = rand_range(0, 1);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrEdgeKsize[j] = rand_range(2, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SubImgNrEdgeKsize, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrEdgeKsize_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrEdgeKsize_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrYCRef_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrYCRef != 15) {
		TEST_FAIL("test default value: 15 != %d\n", (int)attr.stManual.SubImgNrYCRef);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrYCRef = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrYCRef = 31;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrYCRef = rand_range(0, 31);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrYCRef = rand_range(32, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrYCRef = 15;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrYCRef_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrYCRef_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrYCRef_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrYCRef, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrYCRef[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrYCRef[j] = 31;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrYCRef[j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrYCRef[j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
	memcpy(attr.stAuto.SubImgNrYCRef, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrYCRef_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrYCRef_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrUVCRef_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrUVCRef != 15) {
		TEST_FAIL("test default value: 15 != %d\n", (int)attr.stManual.SubImgNrUVCRef);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrUVCRef = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrUVCRef = 31;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrUVCRef = rand_range(0, 31);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrUVCRef = rand_range(32, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrUVCRef = 15;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrUVCRef_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrUVCRef_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrUVCRef_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrUVCRef, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrUVCRef[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrUVCRef[j] = 31;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrUVCRef[j] = rand_range(0, 31);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrUVCRef[j] = rand_range(32, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
	memcpy(attr.stAuto.SubImgNrUVCRef, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrUVCRef_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrUVCRef_auto);
}

static int test_CNR_CNRFilterAttr_OutierThY_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.OutierThY != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.OutierThY);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.OutierThY = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.OutierThY = 7;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.OutierThY = rand_range(0, 7);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.OutierThY = rand_range(8, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.OutierThY = 7;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_OutierThY_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_OutierThY_manual);
}

static int test_CNR_CNRFilterAttr_OutierThY_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_0, attr.stAuto.OutierThY, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.OutierThY[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.OutierThY[j] = 7;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.OutierThY[j] = rand_range(0, 7);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.OutierThY[j] = rand_range(8, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.OutierThY, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_OutierThY_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_OutierThY_auto);
}

static int test_CNR_CNRFilterAttr_OutierThUV_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.OutierThUV != 7) {
		TEST_FAIL("test default value: 7 != %d\n", (int)attr.stManual.OutierThUV);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.OutierThUV = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.OutierThUV = 7;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.OutierThUV = rand_range(0, 7);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.OutierThUV = rand_range(8, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.OutierThUV = 7;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_OutierThUV_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_OutierThUV_manual);
}

static int test_CNR_CNRFilterAttr_OutierThUV_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	if (memcmp(default_val_0, attr.stAuto.OutierThUV, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.OutierThUV[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.OutierThUV[j] = 7;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.OutierThUV[j] = rand_range(0, 7);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.OutierThUV[j] = rand_range(8, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
	memcpy(attr.stAuto.OutierThUV, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_OutierThUV_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_OutierThUV_auto);
}

static int test_CNR_CNRFilterAttr_YOutierDifTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.YOutierDifTh != 24) {
		TEST_FAIL("test default value: 24 != %d\n", (int)attr.stManual.YOutierDifTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.YOutierDifTh = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.YOutierDifTh = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.YOutierDifTh = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.YOutierDifTh = 24;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_YOutierDifTh_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_YOutierDifTh_manual);
}

static int test_CNR_CNRFilterAttr_YOutierDifTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	if (memcmp(default_val_0, attr.stAuto.YOutierDifTh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YOutierDifTh[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.YOutierDifTh[j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.YOutierDifTh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	memcpy(attr.stAuto.YOutierDifTh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_YOutierDifTh_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_YOutierDifTh_auto);
}

static int test_CNR_CNRFilterAttr_UOutierDifTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UOutierDifTh != 24) {
		TEST_FAIL("test default value: 24 != %d\n", (int)attr.stManual.UOutierDifTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UOutierDifTh = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.UOutierDifTh = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.UOutierDifTh = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.UOutierDifTh = 24;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_UOutierDifTh_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_UOutierDifTh_manual);
}

static int test_CNR_CNRFilterAttr_UOutierDifTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	if (memcmp(default_val_0, attr.stAuto.UOutierDifTh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UOutierDifTh[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.UOutierDifTh[j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.UOutierDifTh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	memcpy(attr.stAuto.UOutierDifTh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_UOutierDifTh_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_UOutierDifTh_auto);
}

static int test_CNR_CNRFilterAttr_VOutierDifTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.VOutierDifTh != 24) {
		TEST_FAIL("test default value: 24 != %d\n", (int)attr.stManual.VOutierDifTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.VOutierDifTh = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.VOutierDifTh = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.VOutierDifTh = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.VOutierDifTh = 24;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_VOutierDifTh_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_VOutierDifTh_manual);
}

static int test_CNR_CNRFilterAttr_VOutierDifTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	if (memcmp(default_val_0, attr.stAuto.VOutierDifTh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.VOutierDifTh[j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.VOutierDifTh[j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.VOutierDifTh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	memcpy(attr.stAuto.VOutierDifTh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_VOutierDifTh_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_VOutierDifTh_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCThrYDif_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrRefCThrYDif[0] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.SubImgNrRefCThrYDif[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCThrYDif[1] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.SubImgNrRefCThrYDif[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCThrYDif[2] != 10) {
		TEST_FAIL("test default value: 10 != %d\n", (int)attr.stManual.SubImgNrRefCThrYDif[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrRefCThrYDif[0] = 0;
	attr.stManual.SubImgNrRefCThrYDif[1] = 0;
	attr.stManual.SubImgNrRefCThrYDif[2] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCThrYDif[0] = 255;
	attr.stManual.SubImgNrRefCThrYDif[1] = 255;
	attr.stManual.SubImgNrRefCThrYDif[2] = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCThrYDif[0] = rand_range(0, 255);
	attr.stManual.SubImgNrRefCThrYDif[1] = rand_range(0, 255);
	attr.stManual.SubImgNrRefCThrYDif[2] = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCThrYDif[0] = 2;
	attr.stManual.SubImgNrRefCThrYDif[1] = 6;
	attr.stManual.SubImgNrRefCThrYDif[2] = 10;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCThrYDif_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCThrYDif_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCThrYDif_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrRefCThrYDif[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrRefCThrYDif[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrRefCThrYDif[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrYDif[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrYDif[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrYDif[2][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCThrYDif[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrYDif[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrYDif[2][j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCThrYDif[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrYDif[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrYDif[2][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.SubImgNrRefCThrYDif[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.SubImgNrRefCThrYDif[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	memcpy(attr.stAuto.SubImgNrRefCThrYDif[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCThrYDif_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCThrYDif_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCStrY_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrRefCStrY[0] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrRefCStrY[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCStrY[1] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrRefCStrY[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCStrY[2] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.SubImgNrRefCStrY[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCStrY[3] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SubImgNrRefCStrY[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrRefCStrY[0] = 0;
	attr.stManual.SubImgNrRefCStrY[1] = 0;
	attr.stManual.SubImgNrRefCStrY[2] = 0;
	attr.stManual.SubImgNrRefCStrY[3] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCStrY[0] = 8;
	attr.stManual.SubImgNrRefCStrY[1] = 8;
	attr.stManual.SubImgNrRefCStrY[2] = 8;
	attr.stManual.SubImgNrRefCStrY[3] = 8;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCStrY[0] = rand_range(0, 8);
	attr.stManual.SubImgNrRefCStrY[1] = rand_range(0, 8);
	attr.stManual.SubImgNrRefCStrY[2] = rand_range(0, 8);
	attr.stManual.SubImgNrRefCStrY[3] = rand_range(0, 8);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCStrY[0] = rand_range(9, 255);
	attr.stManual.SubImgNrRefCStrY[1] = rand_range(9, 255);
	attr.stManual.SubImgNrRefCStrY[2] = rand_range(9, 255);
	attr.stManual.SubImgNrRefCStrY[3] = rand_range(9, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrRefCStrY[0] = 8;
	attr.stManual.SubImgNrRefCStrY[1] = 4;
	attr.stManual.SubImgNrRefCStrY[2] = 2;
	attr.stManual.SubImgNrRefCStrY[3] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCStrY_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCStrY_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCStrY_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrRefCStrY[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrRefCStrY[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrRefCStrY[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrRefCStrY[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[3][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCStrY[0][j] = 8;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[1][j] = 8;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[2][j] = 8;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[3][j] = 8;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCStrY[0][j] = rand_range(0, 8);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[1][j] = rand_range(0, 8);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[2][j] = rand_range(0, 8);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[3][j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCStrY[0][j] = rand_range(9, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[1][j] = rand_range(9, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[2][j] = rand_range(9, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrY[3][j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrRefCStrY[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.SubImgNrRefCStrY[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.SubImgNrRefCStrY[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SubImgNrRefCStrY[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCStrY_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCStrY_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCThrUVDif_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrRefCThrUVDif[0] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.SubImgNrRefCThrUVDif[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCThrUVDif[1] != 6) {
		TEST_FAIL("test default value: 6 != %d\n", (int)attr.stManual.SubImgNrRefCThrUVDif[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCThrUVDif[2] != 10) {
		TEST_FAIL("test default value: 10 != %d\n", (int)attr.stManual.SubImgNrRefCThrUVDif[2]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrRefCThrUVDif[0] = 0;
	attr.stManual.SubImgNrRefCThrUVDif[1] = 0;
	attr.stManual.SubImgNrRefCThrUVDif[2] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCThrUVDif[0] = 255;
	attr.stManual.SubImgNrRefCThrUVDif[1] = 255;
	attr.stManual.SubImgNrRefCThrUVDif[2] = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCThrUVDif[0] = rand_range(0, 255);
	attr.stManual.SubImgNrRefCThrUVDif[1] = rand_range(0, 255);
	attr.stManual.SubImgNrRefCThrUVDif[2] = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCThrUVDif[0] = 2;
	attr.stManual.SubImgNrRefCThrUVDif[1] = 6;
	attr.stManual.SubImgNrRefCThrUVDif[2] = 10;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCThrUVDif_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCThrUVDif_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCThrUVDif_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrRefCThrUVDif[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrRefCThrUVDif[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrRefCThrUVDif[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrUVDif[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrUVDif[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrUVDif[2][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCThrUVDif[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrUVDif[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrUVDif[2][j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCThrUVDif[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrUVDif[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCThrUVDif[2][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.SubImgNrRefCThrUVDif[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6 };
	memcpy(attr.stAuto.SubImgNrRefCThrUVDif[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	memcpy(attr.stAuto.SubImgNrRefCThrUVDif[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCThrUVDif_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCThrUVDif_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCStrUV_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrRefCStrUV[0] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrRefCStrUV[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCStrUV[1] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrRefCStrUV[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCStrUV[2] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.SubImgNrRefCStrUV[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrRefCStrUV[3] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SubImgNrRefCStrUV[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrRefCStrUV[0] = 0;
	attr.stManual.SubImgNrRefCStrUV[1] = 0;
	attr.stManual.SubImgNrRefCStrUV[2] = 0;
	attr.stManual.SubImgNrRefCStrUV[3] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCStrUV[0] = 8;
	attr.stManual.SubImgNrRefCStrUV[1] = 8;
	attr.stManual.SubImgNrRefCStrUV[2] = 8;
	attr.stManual.SubImgNrRefCStrUV[3] = 8;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCStrUV[0] = rand_range(0, 8);
	attr.stManual.SubImgNrRefCStrUV[1] = rand_range(0, 8);
	attr.stManual.SubImgNrRefCStrUV[2] = rand_range(0, 8);
	attr.stManual.SubImgNrRefCStrUV[3] = rand_range(0, 8);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrRefCStrUV[0] = rand_range(9, 255);
	attr.stManual.SubImgNrRefCStrUV[1] = rand_range(9, 255);
	attr.stManual.SubImgNrRefCStrUV[2] = rand_range(9, 255);
	attr.stManual.SubImgNrRefCStrUV[3] = rand_range(9, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrRefCStrUV[0] = 8;
	attr.stManual.SubImgNrRefCStrUV[1] = 4;
	attr.stManual.SubImgNrRefCStrUV[2] = 2;
	attr.stManual.SubImgNrRefCStrUV[3] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCStrUV_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCStrUV_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrRefCStrUV_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrRefCStrUV[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrRefCStrUV[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrRefCStrUV[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrRefCStrUV[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[3][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCStrUV[0][j] = 8;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[1][j] = 8;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[2][j] = 8;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[3][j] = 8;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCStrUV[0][j] = rand_range(0, 8);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[1][j] = rand_range(0, 8);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[2][j] = rand_range(0, 8);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[3][j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrRefCStrUV[0][j] = rand_range(9, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[1][j] = rand_range(9, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[2][j] = rand_range(9, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrRefCStrUV[3][j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrRefCStrUV[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.SubImgNrRefCStrUV[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.SubImgNrRefCStrUV[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SubImgNrRefCStrUV[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrRefCStrUV_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrRefCStrUV_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltThrY_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrDirFltThrY[0] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrDirFltThrY[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrY[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrDirFltThrY[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrY[2] != 12) {
		TEST_FAIL("test default value: 12 != %d\n", (int)attr.stManual.SubImgNrDirFltThrY[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrY[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.SubImgNrDirFltThrY[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrY[4] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.SubImgNrDirFltThrY[4]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrDirFltThrY[0] = 0;
	attr.stManual.SubImgNrDirFltThrY[1] = 0;
	attr.stManual.SubImgNrDirFltThrY[2] = 0;
	attr.stManual.SubImgNrDirFltThrY[3] = 0;
	attr.stManual.SubImgNrDirFltThrY[4] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrY[0] = 255;
	attr.stManual.SubImgNrDirFltThrY[1] = 255;
	attr.stManual.SubImgNrDirFltThrY[2] = 255;
	attr.stManual.SubImgNrDirFltThrY[3] = 255;
	attr.stManual.SubImgNrDirFltThrY[4] = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrY[0] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrY[1] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrY[2] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrY[3] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrY[4] = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrY[0] = 4;
	attr.stManual.SubImgNrDirFltThrY[1] = 8;
	attr.stManual.SubImgNrDirFltThrY[2] = 12;
	attr.stManual.SubImgNrDirFltThrY[3] = 16;
	attr.stManual.SubImgNrDirFltThrY[4] = 20;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltThrY_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltThrY_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltThrY_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrDirFltThrY[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrDirFltThrY[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrDirFltThrY[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrDirFltThrY[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_4, attr.stAuto.SubImgNrDirFltThrY[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[4][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltThrY[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[4][j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltThrY[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrY[4][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.SubImgNrDirFltThrY[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrDirFltThrY[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	memcpy(attr.stAuto.SubImgNrDirFltThrY[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.SubImgNrDirFltThrY[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.SubImgNrDirFltThrY[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltThrY_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltThrY_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltStrY_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrDirFltStrY[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.SubImgNrDirFltStrY[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrY[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrDirFltStrY[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrY[2] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrDirFltStrY[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrY[3] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.SubImgNrDirFltStrY[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrY[4] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.SubImgNrDirFltStrY[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrY[5] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SubImgNrDirFltStrY[5]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrDirFltStrY[0] = 0;
	attr.stManual.SubImgNrDirFltStrY[1] = 0;
	attr.stManual.SubImgNrDirFltStrY[2] = 0;
	attr.stManual.SubImgNrDirFltStrY[3] = 0;
	attr.stManual.SubImgNrDirFltStrY[4] = 0;
	attr.stManual.SubImgNrDirFltStrY[5] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrY[0] = 16;
	attr.stManual.SubImgNrDirFltStrY[1] = 16;
	attr.stManual.SubImgNrDirFltStrY[2] = 16;
	attr.stManual.SubImgNrDirFltStrY[3] = 16;
	attr.stManual.SubImgNrDirFltStrY[4] = 16;
	attr.stManual.SubImgNrDirFltStrY[5] = 16;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrY[0] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrY[1] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrY[2] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrY[3] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrY[4] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrY[5] = rand_range(0, 16);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrY[0] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrY[1] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrY[2] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrY[3] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrY[4] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrY[5] = rand_range(17, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrDirFltStrY[0] = 16;
	attr.stManual.SubImgNrDirFltStrY[1] = 8;
	attr.stManual.SubImgNrDirFltStrY[2] = 4;
	attr.stManual.SubImgNrDirFltStrY[3] = 2;
	attr.stManual.SubImgNrDirFltStrY[4] = 1;
	attr.stManual.SubImgNrDirFltStrY[5] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltStrY_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltStrY_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltStrY_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrDirFltStrY[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrDirFltStrY[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrDirFltStrY[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrDirFltStrY[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_4, attr.stAuto.SubImgNrDirFltStrY[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_5, attr.stAuto.SubImgNrDirFltStrY[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[5][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrY[0][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[1][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[2][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[3][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[4][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[5][j] = 16;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrY[0][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[1][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[2][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[3][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[4][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[5][j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrY[0][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[1][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[2][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[3][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[4][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrY[5][j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.SubImgNrDirFltStrY[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrDirFltStrY[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.SubImgNrDirFltStrY[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.SubImgNrDirFltStrY[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.SubImgNrDirFltStrY[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SubImgNrDirFltStrY[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltStrY_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltStrY_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltThrU_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrDirFltThrU[0] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrDirFltThrU[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrU[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrDirFltThrU[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrU[2] != 12) {
		TEST_FAIL("test default value: 12 != %d\n", (int)attr.stManual.SubImgNrDirFltThrU[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrU[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.SubImgNrDirFltThrU[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrU[4] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.SubImgNrDirFltThrU[4]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrDirFltThrU[0] = 0;
	attr.stManual.SubImgNrDirFltThrU[1] = 0;
	attr.stManual.SubImgNrDirFltThrU[2] = 0;
	attr.stManual.SubImgNrDirFltThrU[3] = 0;
	attr.stManual.SubImgNrDirFltThrU[4] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrU[0] = 255;
	attr.stManual.SubImgNrDirFltThrU[1] = 255;
	attr.stManual.SubImgNrDirFltThrU[2] = 255;
	attr.stManual.SubImgNrDirFltThrU[3] = 255;
	attr.stManual.SubImgNrDirFltThrU[4] = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrU[0] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrU[1] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrU[2] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrU[3] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrU[4] = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrU[0] = 4;
	attr.stManual.SubImgNrDirFltThrU[1] = 8;
	attr.stManual.SubImgNrDirFltThrU[2] = 12;
	attr.stManual.SubImgNrDirFltThrU[3] = 16;
	attr.stManual.SubImgNrDirFltThrU[4] = 20;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltThrU_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltThrU_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltThrU_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrDirFltThrU[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrDirFltThrU[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrDirFltThrU[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrDirFltThrU[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_4, attr.stAuto.SubImgNrDirFltThrU[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[4][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltThrU[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[4][j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltThrU[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrU[4][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.SubImgNrDirFltThrU[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrDirFltThrU[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	memcpy(attr.stAuto.SubImgNrDirFltThrU[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.SubImgNrDirFltThrU[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.SubImgNrDirFltThrU[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltThrU_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltThrU_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltStrU_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrDirFltStrU[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.SubImgNrDirFltStrU[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrU[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrDirFltStrU[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrU[2] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrDirFltStrU[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrU[3] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.SubImgNrDirFltStrU[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrU[4] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.SubImgNrDirFltStrU[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrU[5] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SubImgNrDirFltStrU[5]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrDirFltStrU[0] = 0;
	attr.stManual.SubImgNrDirFltStrU[1] = 0;
	attr.stManual.SubImgNrDirFltStrU[2] = 0;
	attr.stManual.SubImgNrDirFltStrU[3] = 0;
	attr.stManual.SubImgNrDirFltStrU[4] = 0;
	attr.stManual.SubImgNrDirFltStrU[5] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrU[0] = 16;
	attr.stManual.SubImgNrDirFltStrU[1] = 16;
	attr.stManual.SubImgNrDirFltStrU[2] = 16;
	attr.stManual.SubImgNrDirFltStrU[3] = 16;
	attr.stManual.SubImgNrDirFltStrU[4] = 16;
	attr.stManual.SubImgNrDirFltStrU[5] = 16;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrU[0] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrU[1] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrU[2] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrU[3] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrU[4] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrU[5] = rand_range(0, 16);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrU[0] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrU[1] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrU[2] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrU[3] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrU[4] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrU[5] = rand_range(17, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrDirFltStrU[0] = 16;
	attr.stManual.SubImgNrDirFltStrU[1] = 8;
	attr.stManual.SubImgNrDirFltStrU[2] = 4;
	attr.stManual.SubImgNrDirFltStrU[3] = 2;
	attr.stManual.SubImgNrDirFltStrU[4] = 1;
	attr.stManual.SubImgNrDirFltStrU[5] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltStrU_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltStrU_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltStrU_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrDirFltStrU[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrDirFltStrU[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrDirFltStrU[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrDirFltStrU[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_4, attr.stAuto.SubImgNrDirFltStrU[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_5, attr.stAuto.SubImgNrDirFltStrU[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[5][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrU[0][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[1][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[2][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[3][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[4][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[5][j] = 16;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrU[0][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[1][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[2][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[3][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[4][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[5][j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrU[0][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[1][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[2][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[3][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[4][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrU[5][j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.SubImgNrDirFltStrU[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrDirFltStrU[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.SubImgNrDirFltStrU[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.SubImgNrDirFltStrU[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.SubImgNrDirFltStrU[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SubImgNrDirFltStrU[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltStrU_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltStrU_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltThrV_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrDirFltThrV[0] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrDirFltThrV[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrV[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrDirFltThrV[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrV[2] != 12) {
		TEST_FAIL("test default value: 12 != %d\n", (int)attr.stManual.SubImgNrDirFltThrV[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrV[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.SubImgNrDirFltThrV[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltThrV[4] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.SubImgNrDirFltThrV[4]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrDirFltThrV[0] = 0;
	attr.stManual.SubImgNrDirFltThrV[1] = 0;
	attr.stManual.SubImgNrDirFltThrV[2] = 0;
	attr.stManual.SubImgNrDirFltThrV[3] = 0;
	attr.stManual.SubImgNrDirFltThrV[4] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrV[0] = 255;
	attr.stManual.SubImgNrDirFltThrV[1] = 255;
	attr.stManual.SubImgNrDirFltThrV[2] = 255;
	attr.stManual.SubImgNrDirFltThrV[3] = 255;
	attr.stManual.SubImgNrDirFltThrV[4] = 255;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrV[0] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrV[1] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrV[2] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrV[3] = rand_range(0, 255);
	attr.stManual.SubImgNrDirFltThrV[4] = rand_range(0, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltThrV[0] = 4;
	attr.stManual.SubImgNrDirFltThrV[1] = 8;
	attr.stManual.SubImgNrDirFltThrV[2] = 12;
	attr.stManual.SubImgNrDirFltThrV[3] = 16;
	attr.stManual.SubImgNrDirFltThrV[4] = 20;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltThrV_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltThrV_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltThrV_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrDirFltThrV[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrDirFltThrV[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrDirFltThrV[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrDirFltThrV[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_4, attr.stAuto.SubImgNrDirFltThrV[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[4][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltThrV[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[4][j] = 255;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltThrV[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltThrV[4][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.SubImgNrDirFltThrV[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrDirFltThrV[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	memcpy(attr.stAuto.SubImgNrDirFltThrV[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.SubImgNrDirFltThrV[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.SubImgNrDirFltThrV[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltThrV_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltThrV_auto);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltStrV_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SubImgNrDirFltStrV[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.SubImgNrDirFltStrV[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrV[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SubImgNrDirFltStrV[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrV[2] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.SubImgNrDirFltStrV[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrV[3] != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.SubImgNrDirFltStrV[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrV[4] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.SubImgNrDirFltStrV[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SubImgNrDirFltStrV[5] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SubImgNrDirFltStrV[5]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SubImgNrDirFltStrV[0] = 0;
	attr.stManual.SubImgNrDirFltStrV[1] = 0;
	attr.stManual.SubImgNrDirFltStrV[2] = 0;
	attr.stManual.SubImgNrDirFltStrV[3] = 0;
	attr.stManual.SubImgNrDirFltStrV[4] = 0;
	attr.stManual.SubImgNrDirFltStrV[5] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrV[0] = 16;
	attr.stManual.SubImgNrDirFltStrV[1] = 16;
	attr.stManual.SubImgNrDirFltStrV[2] = 16;
	attr.stManual.SubImgNrDirFltStrV[3] = 16;
	attr.stManual.SubImgNrDirFltStrV[4] = 16;
	attr.stManual.SubImgNrDirFltStrV[5] = 16;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrV[0] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrV[1] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrV[2] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrV[3] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrV[4] = rand_range(0, 16);
	attr.stManual.SubImgNrDirFltStrV[5] = rand_range(0, 16);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
	attr.stManual.SubImgNrDirFltStrV[0] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrV[1] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrV[2] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrV[3] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrV[4] = rand_range(17, 255);
	attr.stManual.SubImgNrDirFltStrV[5] = rand_range(17, 255);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SubImgNrDirFltStrV[0] = 16;
	attr.stManual.SubImgNrDirFltStrV[1] = 8;
	attr.stManual.SubImgNrDirFltStrV[2] = 4;
	attr.stManual.SubImgNrDirFltStrV[3] = 2;
	attr.stManual.SubImgNrDirFltStrV[4] = 1;
	attr.stManual.SubImgNrDirFltStrV[5] = 0;
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltStrV_manual(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltStrV_manual);
}

static int test_CNR_CNRFilterAttr_SubImgNrDirFltStrV_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CNR_FILTER_ATTR_S attr;

	ret = CVI_ISP_GetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCNRFilterAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.SubImgNrDirFltStrV[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.SubImgNrDirFltStrV[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_2, attr.stAuto.SubImgNrDirFltStrV[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_3, attr.stAuto.SubImgNrDirFltStrV[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_4, attr.stAuto.SubImgNrDirFltStrV[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_5, attr.stAuto.SubImgNrDirFltStrV[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[5][j] = 0;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrV[0][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[1][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[2][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[3][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[4][j] = 16;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[5][j] = 16;
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrV[0][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[1][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[2][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[3][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[4][j] = rand_range(0, 16);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[5][j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
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
		attr.stAuto.SubImgNrDirFltStrV[0][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[1][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[2][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[3][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[4][j] = rand_range(17, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SubImgNrDirFltStrV[5][j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.SubImgNrDirFltStrV[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SubImgNrDirFltStrV[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.SubImgNrDirFltStrV[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.SubImgNrDirFltStrV[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.SubImgNrDirFltStrV[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SubImgNrDirFltStrV[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCNRFilterAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CNR_CNRFilterAttr_SubImgNrDirFltStrV_auto(void)
{
	reg_api_test_fn(test_CNR_CNRFilterAttr_SubImgNrDirFltStrV_auto);
}
