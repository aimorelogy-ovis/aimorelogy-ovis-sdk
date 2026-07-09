
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

static int test_PRESHARPEN_PreSharpenAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_Enable(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_Enable);
}

static int test_PRESHARPEN_PreSharpenAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_enOpType(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_enOpType);
}

static int test_PRESHARPEN_PreSharpenAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_UpdateInterval);
}

static int test_PRESHARPEN_PreSharpenAttr_TuningMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.TuningMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.TuningMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.TuningMode = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.TuningMode = 15;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.TuningMode = rand_range(0, 15);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.TuningMode = rand_range(16, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.TuningMode = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_TuningMode(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_TuningMode);
}

static int test_PRESHARPEN_PreSharpenAttr_GammaSelection(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.GammaSelection != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.GammaSelection);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.GammaSelection = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.GammaSelection = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.GammaSelection = rand_range(0, 1);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.GammaSelection = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_GammaSelection(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_GammaSelection);
}

static int test_PRESHARPEN_PreSharpenAttr_ChnSelection(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.ChnSelection != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.ChnSelection);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.ChnSelection = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ChnSelection = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ChnSelection = rand_range(0, 1);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ChnSelection = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ChnSelection(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ChnSelection);
}

static int test_PRESHARPEN_PreSharpenAttr_ParamAutoEn(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.ParamAutoEn != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.ParamAutoEn);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.ParamAutoEn = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ParamAutoEn = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ParamAutoEn = rand_range(0, 1);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ParamAutoEn = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ParamAutoEn(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ParamAutoEn);
}

static int test_PRESHARPEN_PreSharpenAttr_SatShtCtrlEn(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.SatShtCtrlEn != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.SatShtCtrlEn);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.SatShtCtrlEn = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.SatShtCtrlEn = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.SatShtCtrlEn = rand_range(0, 1);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.SatShtCtrlEn = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_SatShtCtrlEn(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_SatShtCtrlEn);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeOutFormat(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.EdgeOutFormat != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.EdgeOutFormat);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.EdgeOutFormat = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.EdgeOutFormat = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.EdgeOutFormat = rand_range(0, 1);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.EdgeOutFormat = 1;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeOutFormat(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeOutFormat);
}

static int test_PRESHARPEN_PreSharpenAttr_ReduceByEvRatioThr(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.ReduceByEvRatioThr != 80) {
		TEST_FAIL("test default value: 80 != %d\n", (int)attr.ReduceByEvRatioThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.ReduceByEvRatioThr = 64;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ReduceByEvRatioThr = 255;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ReduceByEvRatioThr = rand_range(64, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ReduceByEvRatioThr = rand_range(0, 63);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.ReduceByEvRatioThr = 80;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ReduceByEvRatioThr(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ReduceByEvRatioThr);
}

static int test_PRESHARPEN_PreSharpenAttr_ReduceByEvRatioStr(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.ReduceByEvRatioStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.ReduceByEvRatioStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.ReduceByEvRatioStr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ReduceByEvRatioStr = 255;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ReduceByEvRatioStr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.ReduceByEvRatioStr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ReduceByEvRatioStr(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ReduceByEvRatioStr);
}

static int test_PRESHARPEN_PreSharpenAttr_FlatThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FlatThr != 40) {
		TEST_FAIL("test default value: 40 != %d\n", (int)attr.stManual.FlatThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FlatThr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatThr = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.FlatThr = 40;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_FlatThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_FlatThr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_FlatThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40 };
	if (memcmp(default_val_0, attr.stAuto.FlatThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FlatThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40 };
	memcpy(attr.stAuto.FlatThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_FlatThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_FlatThr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeThr != 240) {
		TEST_FAIL("test default value: 240 != %d\n", (int)attr.stManual.EdgeThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeThr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeThr = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeThr = 240;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeThr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	if (memcmp(default_val_0, attr.stAuto.EdgeThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	memcpy(attr.stAuto.EdgeThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeThr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_FlatThrHLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FlatThrHLD != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.FlatThrHLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FlatThrHLD = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatThrHLD = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatThrHLD = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatThrHLD = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.FlatThrHLD = 20;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_FlatThrHLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_FlatThrHLD_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_FlatThrHLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_0, attr.stAuto.FlatThrHLD, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FlatThrHLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatThrHLD[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatThrHLD[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatThrHLD[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.FlatThrHLD, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_FlatThrHLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_FlatThrHLD_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeThrHLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeThrHLD != 240) {
		TEST_FAIL("test default value: 240 != %d\n", (int)attr.stManual.EdgeThrHLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeThrHLD = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeThrHLD = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeThrHLD = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeThrHLD = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeThrHLD = 240;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeThrHLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeThrHLD_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeThrHLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	if (memcmp(default_val_0, attr.stAuto.EdgeThrHLD, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeThrHLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeThrHLD[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeThrHLD[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeThrHLD[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	memcpy(attr.stAuto.EdgeThrHLD, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeThrHLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeThrHLD_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_LumaThrHLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaThrHLD != 600) {
		TEST_FAIL("test default value: 600 != %d\n", (int)attr.stManual.LumaThrHLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaThrHLD = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.LumaThrHLD = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.LumaThrHLD = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.LumaThrHLD = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LumaThrHLD = 600;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_LumaThrHLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_LumaThrHLD_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_LumaThrHLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600 };
	if (memcmp(default_val_0, attr.stAuto.LumaThrHLD, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaThrHLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.LumaThrHLD[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.LumaThrHLD[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.LumaThrHLD[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600 };
	memcpy(attr.stAuto.LumaThrHLD, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_LumaThrHLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_LumaThrHLD_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_ThinStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinStr != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.ThinStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinStr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.ThinStr = 255;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.ThinStr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.ThinStr = 64;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ThinStr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ThinStr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_ThinStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.ThinStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinStr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.ThinStr[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.ThinStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.ThinStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ThinStr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ThinStr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_RobustStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.RobustStr != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.RobustStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.RobustStr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.RobustStr = 255;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.RobustStr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.RobustStr = 64;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_RobustStr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_RobustStr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_RobustStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.RobustStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.RobustStr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.RobustStr[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.RobustStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.RobustStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_RobustStr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_RobustStr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_FlatStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FlatStr != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.FlatStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FlatStr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatStr = 255;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatStr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.FlatStr = 64;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_FlatStr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_FlatStr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_FlatStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.FlatStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FlatStr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatStr[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.FlatStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.FlatStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_FlatStr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_FlatStr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeStr != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.EdgeStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeStr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeStr = 255;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeStr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeStr = 64;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeStr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeStr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.EdgeStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeStr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeStr[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.EdgeStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeStr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeStr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_StrFlatSlope_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.StrFlatSlope != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.StrFlatSlope);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.StrFlatSlope = -32768;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.StrFlatSlope = 32767;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.StrFlatSlope = rand_range(-32768, 32767);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.StrFlatSlope = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_StrFlatSlope_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_StrFlatSlope_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_StrFlatSlope_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_S16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.StrFlatSlope, sizeof(CVI_S16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.StrFlatSlope[j] = -32768;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.StrFlatSlope[j] = 32767;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.StrFlatSlope[j] = rand_range(-32768, 32767);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	CVI_S16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.StrFlatSlope, restore_val_0, sizeof(CVI_S16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_StrFlatSlope_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_StrFlatSlope_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_StrEdgeSlope_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.StrEdgeSlope != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.StrEdgeSlope);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.StrEdgeSlope = -32768;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.StrEdgeSlope = 32767;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.StrEdgeSlope = rand_range(-32768, 32767);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.StrEdgeSlope = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_StrEdgeSlope_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_StrEdgeSlope_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_StrEdgeSlope_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_S16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.StrEdgeSlope, sizeof(CVI_S16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.StrEdgeSlope[j] = -32768;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.StrEdgeSlope[j] = 32767;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.StrEdgeSlope[j] = rand_range(-32768, 32767);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	CVI_S16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.StrEdgeSlope, restore_val_0, sizeof(CVI_S16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_StrEdgeSlope_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_StrEdgeSlope_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_ConEngBlendWt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ConEngBlendWt != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.ConEngBlendWt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ConEngBlendWt = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.ConEngBlendWt = 15;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.ConEngBlendWt = rand_range(0, 15);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.ConEngBlendWt = rand_range(16, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ConEngBlendWt = 4;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ConEngBlendWt_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ConEngBlendWt_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_ConEngBlendWt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.ConEngBlendWt, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ConEngBlendWt[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.ConEngBlendWt[j] = 15;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.ConEngBlendWt[j] = rand_range(0, 15);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.ConEngBlendWt[j] = rand_range(16, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.ConEngBlendWt, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_ConEngBlendWt_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_ConEngBlendWt_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeLowEndThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeLowEndThr != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.EdgeLowEndThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeLowEndThr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowEndThr = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowEndThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowEndThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeLowEndThr = 5;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeLowEndThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeLowEndThr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeLowEndThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_0, attr.stAuto.EdgeLowEndThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowEndThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowEndThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowEndThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowEndThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.EdgeLowEndThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeLowEndThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeLowEndThr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeMidEndThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeMidEndThr != 10) {
		TEST_FAIL("test default value: 10 != %d\n", (int)attr.stManual.EdgeMidEndThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeMidEndThr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeMidEndThr = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeMidEndThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeMidEndThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeMidEndThr = 10;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeMidEndThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeMidEndThr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeMidEndThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	if (memcmp(default_val_0, attr.stAuto.EdgeMidEndThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeMidEndThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeMidEndThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeMidEndThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeMidEndThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	memcpy(attr.stAuto.EdgeMidEndThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeMidEndThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeMidEndThr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeHighEndThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeHighEndThr != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.EdgeHighEndThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeHighEndThr = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighEndThr = 1023;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighEndThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighEndThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeHighEndThr = 20;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeHighEndThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeHighEndThr_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_EdgeHighEndThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_0, attr.stAuto.EdgeHighEndThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighEndThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighEndThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighEndThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighEndThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.EdgeHighEndThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_EdgeHighEndThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_EdgeHighEndThr_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_HueShtCtrl_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.HueShtCtrl[0] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[1] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[3] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[4] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[5] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[6] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[7] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[7]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[8] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[8]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[9] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[9]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[10] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[10]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[11] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[11]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[12] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[12]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[13] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[13]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[14] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[14]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[15] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[15]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[16] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.HueShtCtrl[16]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[17] != 24) {
		TEST_FAIL("test default value: 24 != %d\n", (int)attr.stManual.HueShtCtrl[17]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[18] != 28) {
		TEST_FAIL("test default value: 28 != %d\n", (int)attr.stManual.HueShtCtrl[18]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[19] != 30) {
		TEST_FAIL("test default value: 30 != %d\n", (int)attr.stManual.HueShtCtrl[19]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[20] != 32) {
		TEST_FAIL("test default value: 32 != %d\n", (int)attr.stManual.HueShtCtrl[20]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[21] != 30) {
		TEST_FAIL("test default value: 30 != %d\n", (int)attr.stManual.HueShtCtrl[21]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[22] != 38) {
		TEST_FAIL("test default value: 38 != %d\n", (int)attr.stManual.HueShtCtrl[22]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[23] != 24) {
		TEST_FAIL("test default value: 24 != %d\n", (int)attr.stManual.HueShtCtrl[23]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[24] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.HueShtCtrl[24]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[25] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[25]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[26] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[26]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[27] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[27]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[28] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[28]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[29] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[29]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[30] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[30]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[31] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[31]);
		return CVI_FAILURE;
	}
	if (attr.stManual.HueShtCtrl[32] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.HueShtCtrl[32]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.HueShtCtrl[0] = 0;
	attr.stManual.HueShtCtrl[1] = 0;
	attr.stManual.HueShtCtrl[2] = 0;
	attr.stManual.HueShtCtrl[3] = 0;
	attr.stManual.HueShtCtrl[4] = 0;
	attr.stManual.HueShtCtrl[5] = 0;
	attr.stManual.HueShtCtrl[6] = 0;
	attr.stManual.HueShtCtrl[7] = 0;
	attr.stManual.HueShtCtrl[8] = 0;
	attr.stManual.HueShtCtrl[9] = 0;
	attr.stManual.HueShtCtrl[10] = 0;
	attr.stManual.HueShtCtrl[11] = 0;
	attr.stManual.HueShtCtrl[12] = 0;
	attr.stManual.HueShtCtrl[13] = 0;
	attr.stManual.HueShtCtrl[14] = 0;
	attr.stManual.HueShtCtrl[15] = 0;
	attr.stManual.HueShtCtrl[16] = 0;
	attr.stManual.HueShtCtrl[17] = 0;
	attr.stManual.HueShtCtrl[18] = 0;
	attr.stManual.HueShtCtrl[19] = 0;
	attr.stManual.HueShtCtrl[20] = 0;
	attr.stManual.HueShtCtrl[21] = 0;
	attr.stManual.HueShtCtrl[22] = 0;
	attr.stManual.HueShtCtrl[23] = 0;
	attr.stManual.HueShtCtrl[24] = 0;
	attr.stManual.HueShtCtrl[25] = 0;
	attr.stManual.HueShtCtrl[26] = 0;
	attr.stManual.HueShtCtrl[27] = 0;
	attr.stManual.HueShtCtrl[28] = 0;
	attr.stManual.HueShtCtrl[29] = 0;
	attr.stManual.HueShtCtrl[30] = 0;
	attr.stManual.HueShtCtrl[31] = 0;
	attr.stManual.HueShtCtrl[32] = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.HueShtCtrl[0] = 63;
	attr.stManual.HueShtCtrl[1] = 63;
	attr.stManual.HueShtCtrl[2] = 63;
	attr.stManual.HueShtCtrl[3] = 63;
	attr.stManual.HueShtCtrl[4] = 63;
	attr.stManual.HueShtCtrl[5] = 63;
	attr.stManual.HueShtCtrl[6] = 63;
	attr.stManual.HueShtCtrl[7] = 63;
	attr.stManual.HueShtCtrl[8] = 63;
	attr.stManual.HueShtCtrl[9] = 63;
	attr.stManual.HueShtCtrl[10] = 63;
	attr.stManual.HueShtCtrl[11] = 63;
	attr.stManual.HueShtCtrl[12] = 63;
	attr.stManual.HueShtCtrl[13] = 63;
	attr.stManual.HueShtCtrl[14] = 63;
	attr.stManual.HueShtCtrl[15] = 63;
	attr.stManual.HueShtCtrl[16] = 63;
	attr.stManual.HueShtCtrl[17] = 63;
	attr.stManual.HueShtCtrl[18] = 63;
	attr.stManual.HueShtCtrl[19] = 63;
	attr.stManual.HueShtCtrl[20] = 63;
	attr.stManual.HueShtCtrl[21] = 63;
	attr.stManual.HueShtCtrl[22] = 63;
	attr.stManual.HueShtCtrl[23] = 63;
	attr.stManual.HueShtCtrl[24] = 63;
	attr.stManual.HueShtCtrl[25] = 63;
	attr.stManual.HueShtCtrl[26] = 63;
	attr.stManual.HueShtCtrl[27] = 63;
	attr.stManual.HueShtCtrl[28] = 63;
	attr.stManual.HueShtCtrl[29] = 63;
	attr.stManual.HueShtCtrl[30] = 63;
	attr.stManual.HueShtCtrl[31] = 63;
	attr.stManual.HueShtCtrl[32] = 63;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.HueShtCtrl[0] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[1] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[2] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[3] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[4] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[5] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[6] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[7] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[8] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[9] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[10] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[11] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[12] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[13] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[14] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[15] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[16] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[17] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[18] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[19] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[20] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[21] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[22] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[23] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[24] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[25] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[26] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[27] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[28] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[29] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[30] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[31] = rand_range(0, 63);
	attr.stManual.HueShtCtrl[32] = rand_range(0, 63);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.HueShtCtrl[0] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[1] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[2] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[3] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[4] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[5] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[6] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[7] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[8] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[9] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[10] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[11] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[12] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[13] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[14] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[15] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[16] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[17] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[18] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[19] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[20] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[21] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[22] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[23] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[24] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[25] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[26] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[27] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[28] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[29] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[30] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[31] = rand_range(64, 255);
	attr.stManual.HueShtCtrl[32] = rand_range(64, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.HueShtCtrl[0] = 16;
	attr.stManual.HueShtCtrl[1] = 16;
	attr.stManual.HueShtCtrl[2] = 16;
	attr.stManual.HueShtCtrl[3] = 16;
	attr.stManual.HueShtCtrl[4] = 16;
	attr.stManual.HueShtCtrl[5] = 16;
	attr.stManual.HueShtCtrl[6] = 16;
	attr.stManual.HueShtCtrl[7] = 16;
	attr.stManual.HueShtCtrl[8] = 16;
	attr.stManual.HueShtCtrl[9] = 16;
	attr.stManual.HueShtCtrl[10] = 16;
	attr.stManual.HueShtCtrl[11] = 16;
	attr.stManual.HueShtCtrl[12] = 16;
	attr.stManual.HueShtCtrl[13] = 16;
	attr.stManual.HueShtCtrl[14] = 16;
	attr.stManual.HueShtCtrl[15] = 16;
	attr.stManual.HueShtCtrl[16] = 20;
	attr.stManual.HueShtCtrl[17] = 24;
	attr.stManual.HueShtCtrl[18] = 28;
	attr.stManual.HueShtCtrl[19] = 30;
	attr.stManual.HueShtCtrl[20] = 32;
	attr.stManual.HueShtCtrl[21] = 30;
	attr.stManual.HueShtCtrl[22] = 38;
	attr.stManual.HueShtCtrl[23] = 24;
	attr.stManual.HueShtCtrl[24] = 20;
	attr.stManual.HueShtCtrl[25] = 16;
	attr.stManual.HueShtCtrl[26] = 16;
	attr.stManual.HueShtCtrl[27] = 16;
	attr.stManual.HueShtCtrl[28] = 16;
	attr.stManual.HueShtCtrl[29] = 16;
	attr.stManual.HueShtCtrl[30] = 16;
	attr.stManual.HueShtCtrl[31] = 16;
	attr.stManual.HueShtCtrl[32] = 16;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_HueShtCtrl_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_HueShtCtrl_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_HueShtCtrl_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.HueShtCtrl[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_1, attr.stAuto.HueShtCtrl[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.HueShtCtrl[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_3, attr.stAuto.HueShtCtrl[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_4, attr.stAuto.HueShtCtrl[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_5, attr.stAuto.HueShtCtrl[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_6, attr.stAuto.HueShtCtrl[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_7, attr.stAuto.HueShtCtrl[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_8, attr.stAuto.HueShtCtrl[8], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_9, attr.stAuto.HueShtCtrl[9], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_10, attr.stAuto.HueShtCtrl[10], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_11, attr.stAuto.HueShtCtrl[11], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_12, attr.stAuto.HueShtCtrl[12], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_13, attr.stAuto.HueShtCtrl[13], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_14, attr.stAuto.HueShtCtrl[14], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_15, attr.stAuto.HueShtCtrl[15], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_16, attr.stAuto.HueShtCtrl[16], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_17[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	if (memcmp(default_val_17, attr.stAuto.HueShtCtrl[17], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_18[ISP_AUTO_ISO_STRENGTH_NUM] = { 28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28 };
	if (memcmp(default_val_18, attr.stAuto.HueShtCtrl[18], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_19[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	if (memcmp(default_val_19, attr.stAuto.HueShtCtrl[19], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_20[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	if (memcmp(default_val_20, attr.stAuto.HueShtCtrl[20], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_21[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	if (memcmp(default_val_21, attr.stAuto.HueShtCtrl[21], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_22[ISP_AUTO_ISO_STRENGTH_NUM] = { 38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38 };
	if (memcmp(default_val_22, attr.stAuto.HueShtCtrl[22], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_23[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	if (memcmp(default_val_23, attr.stAuto.HueShtCtrl[23], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_24[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_24, attr.stAuto.HueShtCtrl[24], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_25[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_25, attr.stAuto.HueShtCtrl[25], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_26[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_26, attr.stAuto.HueShtCtrl[26], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_27[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_27, attr.stAuto.HueShtCtrl[27], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_28[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_28, attr.stAuto.HueShtCtrl[28], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_29[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_29, attr.stAuto.HueShtCtrl[29], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_30[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_30, attr.stAuto.HueShtCtrl[30], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_31[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_31, attr.stAuto.HueShtCtrl[31], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_32[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_32, attr.stAuto.HueShtCtrl[32], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[7][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[8][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[9][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[10][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[11][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[12][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[13][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[14][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[15][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[16][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[17][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[18][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[19][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[20][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[21][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[22][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[23][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[24][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[25][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[26][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[27][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[28][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[29][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[30][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[31][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[32][j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.HueShtCtrl[0][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[1][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[2][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[3][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[4][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[5][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[6][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[7][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[8][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[9][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[10][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[11][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[12][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[13][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[14][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[15][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[16][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[17][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[18][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[19][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[20][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[21][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[22][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[23][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[24][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[25][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[26][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[27][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[28][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[29][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[30][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[31][j] = 63;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[32][j] = 63;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.HueShtCtrl[0][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[1][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[2][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[3][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[4][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[5][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[6][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[7][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[8][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[9][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[10][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[11][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[12][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[13][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[14][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[15][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[16][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[17][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[18][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[19][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[20][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[21][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[22][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[23][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[24][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[25][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[26][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[27][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[28][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[29][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[30][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[31][j] = rand_range(0, 63);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[32][j] = rand_range(0, 63);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.HueShtCtrl[0][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[1][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[2][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[3][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[4][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[5][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[6][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[7][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[8][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[9][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[10][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[11][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[12][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[13][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[14][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[15][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[16][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[17][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[18][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[19][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[20][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[21][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[22][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[23][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[24][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[25][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[26][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[27][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[28][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[29][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[30][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[31][j] = rand_range(64, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.HueShtCtrl[32][j] = rand_range(64, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[8], restore_val_8, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[9], restore_val_9, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[10], restore_val_10, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[11], restore_val_11, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[12], restore_val_12, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[13], restore_val_13, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[14], restore_val_14, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[15], restore_val_15, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.HueShtCtrl[16], restore_val_16, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_17[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	memcpy(attr.stAuto.HueShtCtrl[17], restore_val_17, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_18[ISP_AUTO_ISO_STRENGTH_NUM] = { 28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28 };
	memcpy(attr.stAuto.HueShtCtrl[18], restore_val_18, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_19[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	memcpy(attr.stAuto.HueShtCtrl[19], restore_val_19, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_20[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	memcpy(attr.stAuto.HueShtCtrl[20], restore_val_20, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_21[ISP_AUTO_ISO_STRENGTH_NUM] = { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 };
	memcpy(attr.stAuto.HueShtCtrl[21], restore_val_21, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_22[ISP_AUTO_ISO_STRENGTH_NUM] = { 38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38 };
	memcpy(attr.stAuto.HueShtCtrl[22], restore_val_22, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_23[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24 };
	memcpy(attr.stAuto.HueShtCtrl[23], restore_val_23, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_24[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.HueShtCtrl[24], restore_val_24, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_25[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[25], restore_val_25, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_26[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[26], restore_val_26, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_27[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[27], restore_val_27, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_28[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[28], restore_val_28, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_29[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[29], restore_val_29, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_30[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[30], restore_val_30, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_31[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[31], restore_val_31, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_32[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.HueShtCtrl[32], restore_val_32, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_HueShtCtrl_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_HueShtCtrl_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_SatShtGainIn_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SatShtGainIn[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SatShtGainIn[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatShtGainIn[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.SatShtGainIn[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatShtGainIn[2] != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.SatShtGainIn[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatShtGainIn[3] != 192) {
		TEST_FAIL("test default value: 192 != %d\n", (int)attr.stManual.SatShtGainIn[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SatShtGainIn[0] = 0;
	attr.stManual.SatShtGainIn[1] = 0;
	attr.stManual.SatShtGainIn[2] = 0;
	attr.stManual.SatShtGainIn[3] = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.SatShtGainIn[0] = 255;
	attr.stManual.SatShtGainIn[1] = 255;
	attr.stManual.SatShtGainIn[2] = 255;
	attr.stManual.SatShtGainIn[3] = 255;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.SatShtGainIn[0] = rand_range(0, 255);
	attr.stManual.SatShtGainIn[1] = rand_range(0, 255);
	attr.stManual.SatShtGainIn[2] = rand_range(0, 255);
	attr.stManual.SatShtGainIn[3] = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.SatShtGainIn[0] = 0;
	attr.stManual.SatShtGainIn[1] = 8;
	attr.stManual.SatShtGainIn[2] = 16;
	attr.stManual.SatShtGainIn[3] = 192;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_SatShtGainIn_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_SatShtGainIn_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_SatShtGainIn_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.SatShtGainIn[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.SatShtGainIn[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_2, attr.stAuto.SatShtGainIn[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	if (memcmp(default_val_3, attr.stAuto.SatShtGainIn[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[3][j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.SatShtGainIn[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[3][j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.SatShtGainIn[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainIn[3][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.SatShtGainIn[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.SatShtGainIn[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.SatShtGainIn[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	memcpy(attr.stAuto.SatShtGainIn[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_SatShtGainIn_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_SatShtGainIn_auto);
}

static int test_PRESHARPEN_PreSharpenAttr_SatShtGainOut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SatShtGainOut[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SatShtGainOut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatShtGainOut[1] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SatShtGainOut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatShtGainOut[2] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.SatShtGainOut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatShtGainOut[3] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.SatShtGainOut[3]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SatShtGainOut[0] = 0;
	attr.stManual.SatShtGainOut[1] = 0;
	attr.stManual.SatShtGainOut[2] = 0;
	attr.stManual.SatShtGainOut[3] = 0;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.SatShtGainOut[0] = 128;
	attr.stManual.SatShtGainOut[1] = 128;
	attr.stManual.SatShtGainOut[2] = 128;
	attr.stManual.SatShtGainOut[3] = 128;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.SatShtGainOut[0] = rand_range(0, 128);
	attr.stManual.SatShtGainOut[1] = rand_range(0, 128);
	attr.stManual.SatShtGainOut[2] = rand_range(0, 128);
	attr.stManual.SatShtGainOut[3] = rand_range(0, 128);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
	attr.stManual.SatShtGainOut[0] = rand_range(129, 255);
	attr.stManual.SatShtGainOut[1] = rand_range(129, 255);
	attr.stManual.SatShtGainOut[2] = rand_range(129, 255);
	attr.stManual.SatShtGainOut[3] = rand_range(129, 255);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SatShtGainOut[0] = 0;
	attr.stManual.SatShtGainOut[1] = 0;
	attr.stManual.SatShtGainOut[2] = 128;
	attr.stManual.SatShtGainOut[3] = 128;
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_SatShtGainOut_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_SatShtGainOut_manual);
}

static int test_PRESHARPEN_PreSharpenAttr_SatShtGainOut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.SatShtGainOut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_1, attr.stAuto.SatShtGainOut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_2, attr.stAuto.SatShtGainOut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_3, attr.stAuto.SatShtGainOut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[3][j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.SatShtGainOut[0][j] = 128;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[1][j] = 128;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[2][j] = 128;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[3][j] = 128;
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.SatShtGainOut[0][j] = rand_range(0, 128);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[1][j] = rand_range(0, 128);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[2][j] = rand_range(0, 128);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[3][j] = rand_range(0, 128);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
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
		attr.stAuto.SatShtGainOut[0][j] = rand_range(129, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[1][j] = rand_range(129, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[2][j] = rand_range(129, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatShtGainOut[3][j] = rand_range(129, 255);
	}
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SatShtGainOut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SatShtGainOut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.SatShtGainOut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.SatShtGainOut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenAttr_SatShtGainOut_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenAttr_SatShtGainOut_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_OverUnderShootEn(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.OverUnderShootEn != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.OverUnderShootEn);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.OverUnderShootEn = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.OverUnderShootEn = 1;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.OverUnderShootEn = rand_range(0, 1);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.OverUnderShootEn = 1;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_OverUnderShootEn(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_OverUnderShootEn);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_RefineEdgeType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.RefineEdgeType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.RefineEdgeType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.RefineEdgeType = OP_TYPE_EDGE;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.RefineEdgeType = OP_TYPE_AVERAGE;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.RefineEdgeType = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_RefineEdgeType(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_RefineEdgeType);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaLowThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaLowThr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.LumaLowThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaLowThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowThr = 1023;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LumaLowThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaLowThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaLowThr_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaLowThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.LumaLowThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.LumaLowThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaLowThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaLowThr_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaHighThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaHighThr != 512) {
		TEST_FAIL("test default value: 512 != %d\n", (int)attr.stManual.LumaHighThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaHighThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighThr = 1023;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LumaHighThr = 512;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaHighThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaHighThr_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaHighThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512 };
	if (memcmp(default_val_0, attr.stAuto.LumaHighThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512 };
	memcpy(attr.stAuto.LumaHighThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaHighThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaHighThr_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaLowStep_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaLowStep != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.LumaLowStep);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaLowStep = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowStep = 7;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowStep = rand_range(0, 7);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowStep = rand_range(8, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LumaLowStep = 2;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaLowStep_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaLowStep_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaLowStep_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.LumaLowStep, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowStep[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowStep[j] = 7;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowStep[j] = rand_range(0, 7);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowStep[j] = rand_range(8, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.LumaLowStep, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaLowStep_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaLowStep_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaHighStep_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaHighStep != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.LumaHighStep);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaHighStep = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighStep = 7;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighStep = rand_range(0, 7);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighStep = rand_range(8, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LumaHighStep = 2;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaHighStep_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaHighStep_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaHighStep_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.LumaHighStep, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighStep[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighStep[j] = 7;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighStep[j] = rand_range(0, 7);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighStep[j] = rand_range(8, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.LumaHighStep, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaHighStep_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaHighStep_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaLowLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaLowLut[0] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaLowLut[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaLowLut[2] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaLowLut[3] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaLowLut[4] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaLowLut[5] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaLowLut[6] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaLowLut[7] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaLowLut[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaLowLut[0] = 0;
	attr.stManual.LumaLowLut[1] = 0;
	attr.stManual.LumaLowLut[2] = 0;
	attr.stManual.LumaLowLut[3] = 0;
	attr.stManual.LumaLowLut[4] = 0;
	attr.stManual.LumaLowLut[5] = 0;
	attr.stManual.LumaLowLut[6] = 0;
	attr.stManual.LumaLowLut[7] = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowLut[0] = 255;
	attr.stManual.LumaLowLut[1] = 255;
	attr.stManual.LumaLowLut[2] = 255;
	attr.stManual.LumaLowLut[3] = 255;
	attr.stManual.LumaLowLut[4] = 255;
	attr.stManual.LumaLowLut[5] = 255;
	attr.stManual.LumaLowLut[6] = 255;
	attr.stManual.LumaLowLut[7] = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowLut[0] = rand_range(0, 255);
	attr.stManual.LumaLowLut[1] = rand_range(0, 255);
	attr.stManual.LumaLowLut[2] = rand_range(0, 255);
	attr.stManual.LumaLowLut[3] = rand_range(0, 255);
	attr.stManual.LumaLowLut[4] = rand_range(0, 255);
	attr.stManual.LumaLowLut[5] = rand_range(0, 255);
	attr.stManual.LumaLowLut[6] = rand_range(0, 255);
	attr.stManual.LumaLowLut[7] = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaLowLut[0] = 64;
	attr.stManual.LumaLowLut[1] = 64;
	attr.stManual.LumaLowLut[2] = 64;
	attr.stManual.LumaLowLut[3] = 64;
	attr.stManual.LumaLowLut[4] = 64;
	attr.stManual.LumaLowLut[5] = 64;
	attr.stManual.LumaLowLut[6] = 64;
	attr.stManual.LumaLowLut[7] = 64;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaLowLut_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaLowLut_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaLowLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.LumaLowLut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.LumaLowLut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_2, attr.stAuto.LumaLowLut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_3, attr.stAuto.LumaLowLut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_4, attr.stAuto.LumaLowLut[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_5, attr.stAuto.LumaLowLut[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_6, attr.stAuto.LumaLowLut[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_7, attr.stAuto.LumaLowLut[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[7][j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowLut[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[4][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[5][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[6][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[7][j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaLowLut[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[4][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[5][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[6][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaLowLut[7][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.LumaLowLut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaLowLut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaLowLut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaLowLut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaLowLut[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaLowLut[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaLowLut[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaLowLut[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaLowLut_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaLowLut_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaHighLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaHighLut[0] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaHighLut[1] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaHighLut[2] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaHighLut[3] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaHighLut[4] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaHighLut[5] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaHighLut[6] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumaHighLut[7] != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.LumaHighLut[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaHighLut[0] = 0;
	attr.stManual.LumaHighLut[1] = 0;
	attr.stManual.LumaHighLut[2] = 0;
	attr.stManual.LumaHighLut[3] = 0;
	attr.stManual.LumaHighLut[4] = 0;
	attr.stManual.LumaHighLut[5] = 0;
	attr.stManual.LumaHighLut[6] = 0;
	attr.stManual.LumaHighLut[7] = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighLut[0] = 255;
	attr.stManual.LumaHighLut[1] = 255;
	attr.stManual.LumaHighLut[2] = 255;
	attr.stManual.LumaHighLut[3] = 255;
	attr.stManual.LumaHighLut[4] = 255;
	attr.stManual.LumaHighLut[5] = 255;
	attr.stManual.LumaHighLut[6] = 255;
	attr.stManual.LumaHighLut[7] = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighLut[0] = rand_range(0, 255);
	attr.stManual.LumaHighLut[1] = rand_range(0, 255);
	attr.stManual.LumaHighLut[2] = rand_range(0, 255);
	attr.stManual.LumaHighLut[3] = rand_range(0, 255);
	attr.stManual.LumaHighLut[4] = rand_range(0, 255);
	attr.stManual.LumaHighLut[5] = rand_range(0, 255);
	attr.stManual.LumaHighLut[6] = rand_range(0, 255);
	attr.stManual.LumaHighLut[7] = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.LumaHighLut[0] = 64;
	attr.stManual.LumaHighLut[1] = 64;
	attr.stManual.LumaHighLut[2] = 64;
	attr.stManual.LumaHighLut[3] = 64;
	attr.stManual.LumaHighLut[4] = 64;
	attr.stManual.LumaHighLut[5] = 64;
	attr.stManual.LumaHighLut[6] = 64;
	attr.stManual.LumaHighLut[7] = 64;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaHighLut_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaHighLut_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_LumaHighLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.LumaHighLut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_1, attr.stAuto.LumaHighLut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_2, attr.stAuto.LumaHighLut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_3, attr.stAuto.LumaHighLut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_4, attr.stAuto.LumaHighLut[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_5, attr.stAuto.LumaHighLut[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_6, attr.stAuto.LumaHighLut[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_7, attr.stAuto.LumaHighLut[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[7][j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighLut[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[4][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[5][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[6][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[7][j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.LumaHighLut[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[4][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[5][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[6][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaHighLut[7][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.LumaHighLut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaHighLut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaHighLut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaHighLut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaHighLut[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaHighLut[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaHighLut[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.LumaHighLut[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_LumaHighLut_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_LumaHighLut_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeLowThr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.EdgeLowThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeLowThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowThr = 1023;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeLowThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowThr_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.EdgeLowThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.EdgeLowThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowThr_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeHighThr != 512) {
		TEST_FAIL("test default value: 512 != %d\n", (int)attr.stManual.EdgeHighThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeHighThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighThr = 1023;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighThr = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighThr = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeHighThr = 512;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighThr_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512 };
	if (memcmp(default_val_0, attr.stAuto.EdgeHighThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighThr[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighThr[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighThr[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512 };
	memcpy(attr.stAuto.EdgeHighThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighThr_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowStep_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeLowStep != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.EdgeLowStep);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeLowStep = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowStep = 7;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowStep = rand_range(0, 7);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowStep = rand_range(8, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeLowStep = 2;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowStep_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowStep_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowStep_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.EdgeLowStep, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowStep[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowStep[j] = 7;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowStep[j] = rand_range(0, 7);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowStep[j] = rand_range(8, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.EdgeLowStep, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowStep_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowStep_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighStep_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeHighStep != 2) {
		TEST_FAIL("test default value: 2 != %d\n", (int)attr.stManual.EdgeHighStep);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeHighStep = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighStep = 7;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighStep = rand_range(0, 7);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighStep = rand_range(8, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeHighStep = 2;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighStep_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighStep_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighStep_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	if (memcmp(default_val_0, attr.stAuto.EdgeHighStep, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighStep[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighStep[j] = 7;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighStep[j] = rand_range(0, 7);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighStep[j] = rand_range(8, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
	memcpy(attr.stAuto.EdgeHighStep, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighStep_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighStep_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeLowLut[0] != 210) {
		TEST_FAIL("test default value: 210 != %d\n", (int)attr.stManual.EdgeLowLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeLowLut[1] != 216) {
		TEST_FAIL("test default value: 216 != %d\n", (int)attr.stManual.EdgeLowLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeLowLut[2] != 224) {
		TEST_FAIL("test default value: 224 != %d\n", (int)attr.stManual.EdgeLowLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeLowLut[3] != 232) {
		TEST_FAIL("test default value: 232 != %d\n", (int)attr.stManual.EdgeLowLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeLowLut[4] != 240) {
		TEST_FAIL("test default value: 240 != %d\n", (int)attr.stManual.EdgeLowLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeLowLut[5] != 248) {
		TEST_FAIL("test default value: 248 != %d\n", (int)attr.stManual.EdgeLowLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeLowLut[6] != 254) {
		TEST_FAIL("test default value: 254 != %d\n", (int)attr.stManual.EdgeLowLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeLowLut[7] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.EdgeLowLut[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeLowLut[0] = 0;
	attr.stManual.EdgeLowLut[1] = 0;
	attr.stManual.EdgeLowLut[2] = 0;
	attr.stManual.EdgeLowLut[3] = 0;
	attr.stManual.EdgeLowLut[4] = 0;
	attr.stManual.EdgeLowLut[5] = 0;
	attr.stManual.EdgeLowLut[6] = 0;
	attr.stManual.EdgeLowLut[7] = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowLut[0] = 255;
	attr.stManual.EdgeLowLut[1] = 255;
	attr.stManual.EdgeLowLut[2] = 255;
	attr.stManual.EdgeLowLut[3] = 255;
	attr.stManual.EdgeLowLut[4] = 255;
	attr.stManual.EdgeLowLut[5] = 255;
	attr.stManual.EdgeLowLut[6] = 255;
	attr.stManual.EdgeLowLut[7] = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowLut[0] = rand_range(0, 255);
	attr.stManual.EdgeLowLut[1] = rand_range(0, 255);
	attr.stManual.EdgeLowLut[2] = rand_range(0, 255);
	attr.stManual.EdgeLowLut[3] = rand_range(0, 255);
	attr.stManual.EdgeLowLut[4] = rand_range(0, 255);
	attr.stManual.EdgeLowLut[5] = rand_range(0, 255);
	attr.stManual.EdgeLowLut[6] = rand_range(0, 255);
	attr.stManual.EdgeLowLut[7] = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeLowLut[0] = 210;
	attr.stManual.EdgeLowLut[1] = 216;
	attr.stManual.EdgeLowLut[2] = 224;
	attr.stManual.EdgeLowLut[3] = 232;
	attr.stManual.EdgeLowLut[4] = 240;
	attr.stManual.EdgeLowLut[5] = 248;
	attr.stManual.EdgeLowLut[6] = 254;
	attr.stManual.EdgeLowLut[7] = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowLut_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowLut_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 210,210,210,210,210,210,210,210,210,210,210,210,210,210,210,210 };
	if (memcmp(default_val_0, attr.stAuto.EdgeLowLut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 216,216,216,216,216,216,216,216,216,216,216,216,216,216,216,216 };
	if (memcmp(default_val_1, attr.stAuto.EdgeLowLut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	if (memcmp(default_val_2, attr.stAuto.EdgeLowLut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 232,232,232,232,232,232,232,232,232,232,232,232,232,232,232,232 };
	if (memcmp(default_val_3, attr.stAuto.EdgeLowLut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	if (memcmp(default_val_4, attr.stAuto.EdgeLowLut[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248 };
	if (memcmp(default_val_5, attr.stAuto.EdgeLowLut[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254 };
	if (memcmp(default_val_6, attr.stAuto.EdgeLowLut[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_7, attr.stAuto.EdgeLowLut[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[7][j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowLut[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[4][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[5][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[6][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[7][j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeLowLut[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[4][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[5][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[6][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeLowLut[7][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 210,210,210,210,210,210,210,210,210,210,210,210,210,210,210,210 };
	memcpy(attr.stAuto.EdgeLowLut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 216,216,216,216,216,216,216,216,216,216,216,216,216,216,216,216 };
	memcpy(attr.stAuto.EdgeLowLut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	memcpy(attr.stAuto.EdgeLowLut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 232,232,232,232,232,232,232,232,232,232,232,232,232,232,232,232 };
	memcpy(attr.stAuto.EdgeLowLut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	memcpy(attr.stAuto.EdgeLowLut[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248 };
	memcpy(attr.stAuto.EdgeLowLut[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254 };
	memcpy(attr.stAuto.EdgeLowLut[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.EdgeLowLut[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowLut_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeLowLut_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeHighLut[0] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.EdgeHighLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeHighLut[1] != 254) {
		TEST_FAIL("test default value: 254 != %d\n", (int)attr.stManual.EdgeHighLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeHighLut[2] != 248) {
		TEST_FAIL("test default value: 248 != %d\n", (int)attr.stManual.EdgeHighLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeHighLut[3] != 240) {
		TEST_FAIL("test default value: 240 != %d\n", (int)attr.stManual.EdgeHighLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeHighLut[4] != 232) {
		TEST_FAIL("test default value: 232 != %d\n", (int)attr.stManual.EdgeHighLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeHighLut[5] != 224) {
		TEST_FAIL("test default value: 224 != %d\n", (int)attr.stManual.EdgeHighLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeHighLut[6] != 216) {
		TEST_FAIL("test default value: 216 != %d\n", (int)attr.stManual.EdgeHighLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeHighLut[7] != 210) {
		TEST_FAIL("test default value: 210 != %d\n", (int)attr.stManual.EdgeHighLut[7]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeHighLut[0] = 0;
	attr.stManual.EdgeHighLut[1] = 0;
	attr.stManual.EdgeHighLut[2] = 0;
	attr.stManual.EdgeHighLut[3] = 0;
	attr.stManual.EdgeHighLut[4] = 0;
	attr.stManual.EdgeHighLut[5] = 0;
	attr.stManual.EdgeHighLut[6] = 0;
	attr.stManual.EdgeHighLut[7] = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighLut[0] = 255;
	attr.stManual.EdgeHighLut[1] = 255;
	attr.stManual.EdgeHighLut[2] = 255;
	attr.stManual.EdgeHighLut[3] = 255;
	attr.stManual.EdgeHighLut[4] = 255;
	attr.stManual.EdgeHighLut[5] = 255;
	attr.stManual.EdgeHighLut[6] = 255;
	attr.stManual.EdgeHighLut[7] = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighLut[0] = rand_range(0, 255);
	attr.stManual.EdgeHighLut[1] = rand_range(0, 255);
	attr.stManual.EdgeHighLut[2] = rand_range(0, 255);
	attr.stManual.EdgeHighLut[3] = rand_range(0, 255);
	attr.stManual.EdgeHighLut[4] = rand_range(0, 255);
	attr.stManual.EdgeHighLut[5] = rand_range(0, 255);
	attr.stManual.EdgeHighLut[6] = rand_range(0, 255);
	attr.stManual.EdgeHighLut[7] = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.EdgeHighLut[0] = 255;
	attr.stManual.EdgeHighLut[1] = 254;
	attr.stManual.EdgeHighLut[2] = 248;
	attr.stManual.EdgeHighLut[3] = 240;
	attr.stManual.EdgeHighLut[4] = 232;
	attr.stManual.EdgeHighLut[5] = 224;
	attr.stManual.EdgeHighLut[6] = 216;
	attr.stManual.EdgeHighLut[7] = 210;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighLut_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighLut_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_0, attr.stAuto.EdgeHighLut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254 };
	if (memcmp(default_val_1, attr.stAuto.EdgeHighLut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248 };
	if (memcmp(default_val_2, attr.stAuto.EdgeHighLut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	if (memcmp(default_val_3, attr.stAuto.EdgeHighLut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 232,232,232,232,232,232,232,232,232,232,232,232,232,232,232,232 };
	if (memcmp(default_val_4, attr.stAuto.EdgeHighLut[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	if (memcmp(default_val_5, attr.stAuto.EdgeHighLut[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 216,216,216,216,216,216,216,216,216,216,216,216,216,216,216,216 };
	if (memcmp(default_val_6, attr.stAuto.EdgeHighLut[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 210,210,210,210,210,210,210,210,210,210,210,210,210,210,210,210 };
	if (memcmp(default_val_7, attr.stAuto.EdgeHighLut[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[7][j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighLut[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[4][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[5][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[6][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[7][j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeHighLut[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[4][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[5][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[6][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeHighLut[7][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.EdgeHighLut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254 };
	memcpy(attr.stAuto.EdgeHighLut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248 };
	memcpy(attr.stAuto.EdgeHighLut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	memcpy(attr.stAuto.EdgeHighLut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 232,232,232,232,232,232,232,232,232,232,232,232,232,232,232,232 };
	memcpy(attr.stAuto.EdgeHighLut[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	memcpy(attr.stAuto.EdgeHighLut[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 216,216,216,216,216,216,216,216,216,216,216,216,216,216,216,216 };
	memcpy(attr.stAuto.EdgeHighLut[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 210,210,210,210,210,210,210,210,210,210,210,210,210,210,210,210 };
	memcpy(attr.stAuto.EdgeHighLut[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighLut_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_EdgeHighLut_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_OvershootThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.OvershootThr != 250) {
		TEST_FAIL("test default value: 250 != %d\n", (int)attr.stManual.OvershootThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.OvershootThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OvershootThr = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OvershootThr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OvershootThr = 250;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_OvershootThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_OvershootThr_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_OvershootThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250 };
	if (memcmp(default_val_0, attr.stAuto.OvershootThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.OvershootThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OvershootThr[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OvershootThr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250 };
	memcpy(attr.stAuto.OvershootThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_OvershootThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_OvershootThr_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_OshootSlope_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.OshootSlope != 96) {
		TEST_FAIL("test default value: 96 != %d\n", (int)attr.stManual.OshootSlope);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.OshootSlope = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OshootSlope = 32767;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OshootSlope = rand_range(0, 32767);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OshootSlope = rand_range(32768, 65535);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.OshootSlope = 96;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_OshootSlope_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_OshootSlope_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_OshootSlope_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	if (memcmp(default_val_0, attr.stAuto.OshootSlope, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.OshootSlope[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OshootSlope[j] = 32767;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OshootSlope[j] = rand_range(0, 32767);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OshootSlope[j] = rand_range(32768, 65535);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	memcpy(attr.stAuto.OshootSlope, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_OshootSlope_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_OshootSlope_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_OshootWgtHigh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.OshootWgtHigh != 96) {
		TEST_FAIL("test default value: 96 != %d\n", (int)attr.stManual.OshootWgtHigh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.OshootWgtHigh = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OshootWgtHigh = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OshootWgtHigh = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.OshootWgtHigh = rand_range(256, 65535);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.OshootWgtHigh = 96;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_OshootWgtHigh_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_OshootWgtHigh_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_OshootWgtHigh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	if (memcmp(default_val_0, attr.stAuto.OshootWgtHigh, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.OshootWgtHigh[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OshootWgtHigh[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OshootWgtHigh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.OshootWgtHigh[j] = rand_range(256, 65535);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	memcpy(attr.stAuto.OshootWgtHigh, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_OshootWgtHigh_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_OshootWgtHigh_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UndershootThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UndershootThr != 5) {
		TEST_FAIL("test default value: 5 != %d\n", (int)attr.stManual.UndershootThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UndershootThr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UndershootThr = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UndershootThr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UndershootThr = 5;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UndershootThr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UndershootThr_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UndershootThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	if (memcmp(default_val_0, attr.stAuto.UndershootThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UndershootThr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UndershootThr[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UndershootThr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	memcpy(attr.stAuto.UndershootThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UndershootThr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UndershootThr_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootSlope_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootSlope != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.UshootSlope);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootSlope = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootSlope = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootSlope = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootSlope = 128;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootSlope_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootSlope_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootSlope_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.UshootSlope, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootSlope[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootSlope[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootSlope[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.UshootSlope, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootSlope_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootSlope_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootWgtHigh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootWgtHigh != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.UshootWgtHigh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootWgtHigh = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootWgtHigh = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootWgtHigh = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootWgtHigh = 128;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootWgtHigh_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootWgtHigh_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootWgtHigh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.UshootWgtHigh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootWgtHigh[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootWgtHigh[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootWgtHigh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.UshootWgtHigh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootWgtHigh_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootWgtHigh_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrLow_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootLumThrLow != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.UshootLumThrLow);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootLumThrLow = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumThrLow = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumThrLow = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumThrLow = 128;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrLow_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrLow_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrLow_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.UshootLumThrLow, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootLumThrLow[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumThrLow[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumThrLow[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.UshootLumThrLow, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrLow_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrLow_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrHigh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootLumThrHigh != 224) {
		TEST_FAIL("test default value: 224 != %d\n", (int)attr.stManual.UshootLumThrHigh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootLumThrHigh = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumThrHigh = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumThrHigh = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumThrHigh = 224;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrHigh_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrHigh_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrHigh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	if (memcmp(default_val_0, attr.stAuto.UshootLumThrHigh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootLumThrHigh[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumThrHigh[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumThrHigh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	memcpy(attr.stAuto.UshootLumThrHigh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrHigh_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumThrHigh_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumClampWgt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootLumClampWgt != 96) {
		TEST_FAIL("test default value: 96 != %d\n", (int)attr.stManual.UshootLumClampWgt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootLumClampWgt = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumClampWgt = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumClampWgt = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumClampWgt = 96;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumClampWgt_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumClampWgt_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumClampWgt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	if (memcmp(default_val_0, attr.stAuto.UshootLumClampWgt, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootLumClampWgt[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumClampWgt[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumClampWgt[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	memcpy(attr.stAuto.UshootLumClampWgt, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumClampWgt_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumClampWgt_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrLow_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootEngThrLow != 60) {
		TEST_FAIL("test default value: 60 != %d\n", (int)attr.stManual.UshootEngThrLow);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootEngThrLow = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngThrLow = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngThrLow = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngThrLow = 60;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrLow_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrLow_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrLow_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60 };
	if (memcmp(default_val_0, attr.stAuto.UshootEngThrLow, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootEngThrLow[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootEngThrLow[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootEngThrLow[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60 };
	memcpy(attr.stAuto.UshootEngThrLow, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrLow_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrLow_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrHigh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootEngThrHigh != 156) {
		TEST_FAIL("test default value: 156 != %d\n", (int)attr.stManual.UshootEngThrHigh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootEngThrHigh = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngThrHigh = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngThrHigh = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngThrHigh = 156;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrHigh_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrHigh_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrHigh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 156,156,156,156,156,156,156,156,156,156,156,156,156,156,156,156 };
	if (memcmp(default_val_0, attr.stAuto.UshootEngThrHigh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootEngThrHigh[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootEngThrHigh[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootEngThrHigh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 156,156,156,156,156,156,156,156,156,156,156,156,156,156,156,156 };
	memcpy(attr.stAuto.UshootEngThrHigh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrHigh_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootEngThrHigh_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootEngClampWgt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootEngClampWgt != 96) {
		TEST_FAIL("test default value: 96 != %d\n", (int)attr.stManual.UshootEngClampWgt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootEngClampWgt = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngClampWgt = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngClampWgt = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootEngClampWgt = 96;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootEngClampWgt_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootEngClampWgt_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootEngClampWgt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	if (memcmp(default_val_0, attr.stAuto.UshootEngClampWgt, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootEngClampWgt[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootEngClampWgt[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootEngClampWgt[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,96 };
	memcpy(attr.stAuto.UshootEngClampWgt, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootEngClampWgt_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootEngClampWgt_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootLumEngStr != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.UshootLumEngStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootLumEngStr = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumEngStr = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumEngStr = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumEngStr = 1;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngStr_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngStr_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_0, attr.stAuto.UshootLumEngStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootLumEngStr[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumEngStr[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumEngStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.UshootLumEngStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngStr_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngStr_auto);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngNorm_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UshootLumEngNorm != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.UshootLumEngNorm);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UshootLumEngNorm = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumEngNorm = 255;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumEngNorm = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	attr.stManual.UshootLumEngNorm = 0;
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngNorm_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngNorm_manual);
}

static int test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngNorm_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_REFINE_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenRefineAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.UshootLumEngNorm, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UshootLumEngNorm[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumEngNorm[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
		attr.stAuto.UshootLumEngNorm[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.UshootLumEngNorm, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenRefineAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngNorm_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenRefineAttr_UshootLumEngNorm_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RegionStrEn(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.RegionStrEn != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.RegionStrEn);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.RegionStrEn = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.RegionStrEn = 1;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.RegionStrEn = rand_range(0, 1);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.RegionStrEn = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RegionStrEn(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RegionStrEn);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E5aEnhance_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.E5aEnhance != 48) {
		TEST_FAIL("test default value: 48 != %d\n", (int)attr.stManual.E5aEnhance);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.E5aEnhance = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5aEnhance = 1023;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5aEnhance = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5aEnhance = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.E5aEnhance = 48;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E5aEnhance_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E5aEnhance_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E5aEnhance_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	if (memcmp(default_val_0, attr.stAuto.E5aEnhance, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.E5aEnhance[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5aEnhance[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5aEnhance[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5aEnhance[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	memcpy(attr.stAuto.E5aEnhance, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E5aEnhance_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E5aEnhance_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E5bEnhance_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.E5bEnhance != 48) {
		TEST_FAIL("test default value: 48 != %d\n", (int)attr.stManual.E5bEnhance);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.E5bEnhance = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5bEnhance = 1023;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5bEnhance = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5bEnhance = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.E5bEnhance = 48;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E5bEnhance_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E5bEnhance_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E5bEnhance_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	if (memcmp(default_val_0, attr.stAuto.E5bEnhance, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.E5bEnhance[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5bEnhance[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5bEnhance[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5bEnhance[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	memcpy(attr.stAuto.E5bEnhance, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E5bEnhance_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E5bEnhance_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E5cEnhance_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.E5cEnhance != 48) {
		TEST_FAIL("test default value: 48 != %d\n", (int)attr.stManual.E5cEnhance);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.E5cEnhance = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5cEnhance = 1023;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5cEnhance = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E5cEnhance = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.E5cEnhance = 48;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E5cEnhance_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E5cEnhance_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E5cEnhance_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	if (memcmp(default_val_0, attr.stAuto.E5cEnhance, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.E5cEnhance[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5cEnhance[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5cEnhance[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E5cEnhance[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	memcpy(attr.stAuto.E5cEnhance, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E5cEnhance_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E5cEnhance_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E7Enhance_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.E7Enhance != 48) {
		TEST_FAIL("test default value: 48 != %d\n", (int)attr.stManual.E7Enhance);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.E7Enhance = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E7Enhance = 1023;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E7Enhance = rand_range(0, 1023);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.E7Enhance = rand_range(1024, 65535);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.E7Enhance = 48;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E7Enhance_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E7Enhance_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_E7Enhance_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	if (memcmp(default_val_0, attr.stAuto.E7Enhance, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.E7Enhance[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E7Enhance[j] = 1023;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E7Enhance[j] = rand_range(0, 1023);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.E7Enhance[j] = rand_range(1024, 65535);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48 };
	memcpy(attr.stAuto.E7Enhance, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_E7Enhance_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_E7Enhance_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrLow_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.BlendThrLow != 50) {
		TEST_FAIL("test default value: 50 != %d\n", (int)attr.stManual.BlendThrLow);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.BlendThrLow = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendThrLow = 255;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendThrLow = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendThrLow = 50;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrLow_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrLow_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrLow_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50 };
	if (memcmp(default_val_0, attr.stAuto.BlendThrLow, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.BlendThrLow[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendThrLow[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendThrLow[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50 };
	memcpy(attr.stAuto.BlendThrLow, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrLow_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrLow_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrHigh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.BlendThrHigh != 200) {
		TEST_FAIL("test default value: 200 != %d\n", (int)attr.stManual.BlendThrHigh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.BlendThrHigh = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendThrHigh = 255;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendThrHigh = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendThrHigh = 200;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrHigh_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrHigh_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrHigh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	if (memcmp(default_val_0, attr.stAuto.BlendThrHigh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.BlendThrHigh[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendThrHigh[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendThrHigh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	memcpy(attr.stAuto.BlendThrHigh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrHigh_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendThrHigh_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtLow_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.BlendWgtLow != 220) {
		TEST_FAIL("test default value: 220 != %d\n", (int)attr.stManual.BlendWgtLow);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.BlendWgtLow = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendWgtLow = 255;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendWgtLow = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendWgtLow = 220;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtLow_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtLow_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtLow_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220 };
	if (memcmp(default_val_0, attr.stAuto.BlendWgtLow, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.BlendWgtLow[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendWgtLow[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendWgtLow[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220 };
	memcpy(attr.stAuto.BlendWgtLow, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtLow_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtLow_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtHigh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.BlendWgtHigh != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.BlendWgtHigh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.BlendWgtHigh = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendWgtHigh = 255;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendWgtHigh = rand_range(0, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.BlendWgtHigh = 16;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtHigh_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtHigh_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtHigh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.BlendWgtHigh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.BlendWgtHigh[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendWgtHigh[j] = 255;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.BlendWgtHigh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.BlendWgtHigh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtHigh_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_BlendWgtHigh_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlat_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtFlat != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.ThinWgtFlat);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtFlat = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtFlat = 16;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtFlat = rand_range(0, 16);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtFlat = rand_range(17, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtFlat = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlat_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlat_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlat_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtFlat, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtFlat[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtFlat[j] = 16;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtFlat[j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtFlat[j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.ThinWgtFlat, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlat_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlat_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdge_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtEdge != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.ThinWgtEdge);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtEdge = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtEdge = 16;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtEdge = rand_range(0, 16);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtEdge = rand_range(17, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtEdge = 16;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdge_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdge_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdge_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtEdge, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtEdge[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtEdge[j] = 16;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtEdge[j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtEdge[j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.ThinWgtEdge, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdge_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdge_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtUHF_HF != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.ThinWgtUHF_HF);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtUHF_HF = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtUHF_HF = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtUHF_HF = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtUHF_HF = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtUHF_HF = 4;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtUHF_HF, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtUHF_HF[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtUHF_HF[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtUHF_HF[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtUHF_HF[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.ThinWgtUHF_HF, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.RobustWgtHF_SHF != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.RobustWgtHF_SHF);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.RobustWgtHF_SHF = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtHF_SHF = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtHF_SHF = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtHF_SHF = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.RobustWgtHF_SHF = 4;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.RobustWgtHF_SHF, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.RobustWgtHF_SHF[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtHF_SHF[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtHF_SHF[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtHF_SHF[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.RobustWgtHF_SHF, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtAgainstSHF != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.ThinWgtAgainstSHF);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtAgainstSHF = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtAgainstSHF = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtAgainstSHF = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtAgainstSHF = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtAgainstSHF = 4;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtAgainstSHF, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtAgainstSHF[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtAgainstSHF[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtAgainstSHF[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtAgainstSHF[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.ThinWgtAgainstSHF, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.RobustWgtAgainstMF != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.RobustWgtAgainstMF);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.RobustWgtAgainstMF = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtAgainstMF = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtAgainstMF = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtAgainstMF = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.RobustWgtAgainstMF = 4;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.RobustWgtAgainstMF, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.RobustWgtAgainstMF[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtAgainstMF[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtAgainstMF[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtAgainstMF[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.RobustWgtAgainstMF, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlatHLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtFlatHLD != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.ThinWgtFlatHLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtFlatHLD = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtFlatHLD = 16;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtFlatHLD = rand_range(0, 16);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtFlatHLD = rand_range(17, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtFlatHLD = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlatHLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlatHLD_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlatHLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtFlatHLD, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtFlatHLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtFlatHLD[j] = 16;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtFlatHLD[j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtFlatHLD[j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.ThinWgtFlatHLD, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlatHLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtFlatHLD_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdgeHLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtEdgeHLD != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.ThinWgtEdgeHLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtEdgeHLD = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtEdgeHLD = 16;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtEdgeHLD = rand_range(0, 16);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtEdgeHLD = rand_range(17, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtEdgeHLD = 16;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdgeHLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdgeHLD_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdgeHLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtEdgeHLD, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtEdgeHLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtEdgeHLD[j] = 16;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtEdgeHLD[j] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtEdgeHLD[j] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	memcpy(attr.stAuto.ThinWgtEdgeHLD, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdgeHLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtEdgeHLD_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_HLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtUHF_HF_HLD != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.ThinWgtUHF_HF_HLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtUHF_HF_HLD = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtUHF_HF_HLD = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtUHF_HF_HLD = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtUHF_HF_HLD = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtUHF_HF_HLD = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_HLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_HLD_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_HLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtUHF_HF_HLD, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtUHF_HF_HLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtUHF_HF_HLD[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtUHF_HF_HLD[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtUHF_HF_HLD[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.ThinWgtUHF_HF_HLD, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_HLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtUHF_HF_HLD_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_HLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.RobustWgtHF_SHF_HLD != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.RobustWgtHF_SHF_HLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.RobustWgtHF_SHF_HLD = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtHF_SHF_HLD = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtHF_SHF_HLD = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtHF_SHF_HLD = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.RobustWgtHF_SHF_HLD = 4;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_HLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_HLD_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_HLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.RobustWgtHF_SHF_HLD, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.RobustWgtHF_SHF_HLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtHF_SHF_HLD[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtHF_SHF_HLD[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtHF_SHF_HLD[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.RobustWgtHF_SHF_HLD, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_HLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtHF_SHF_HLD_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_HLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ThinWgtAgainstSHF_HLD != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.ThinWgtAgainstSHF_HLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ThinWgtAgainstSHF_HLD = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtAgainstSHF_HLD = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtAgainstSHF_HLD = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.ThinWgtAgainstSHF_HLD = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ThinWgtAgainstSHF_HLD = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_HLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_HLD_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_HLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.ThinWgtAgainstSHF_HLD, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ThinWgtAgainstSHF_HLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtAgainstSHF_HLD[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtAgainstSHF_HLD[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.ThinWgtAgainstSHF_HLD[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.ThinWgtAgainstSHF_HLD, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_HLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_ThinWgtAgainstSHF_HLD_auto);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_HLD_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.RobustWgtAgainstMF_HLD != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.RobustWgtAgainstMF_HLD);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.RobustWgtAgainstMF_HLD = 0;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtAgainstMF_HLD = 8;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtAgainstMF_HLD = rand_range(0, 8);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
	attr.stManual.RobustWgtAgainstMF_HLD = rand_range(9, 255);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.RobustWgtAgainstMF_HLD = 4;
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_HLD_manual(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_HLD_manual);
}

static int test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_HLD_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S attr;

	ret = CVI_ISP_GetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPreSharpenEdgeExtAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.RobustWgtAgainstMF_HLD, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.RobustWgtAgainstMF_HLD[j] = 0;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtAgainstMF_HLD[j] = 8;
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtAgainstMF_HLD[j] = rand_range(0, 8);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
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
		attr.stAuto.RobustWgtAgainstMF_HLD[j] = rand_range(9, 255);
	}
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.RobustWgtAgainstMF_HLD, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPreSharpenEdgeExtAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_HLD_auto(void)
{
	reg_api_test_fn(test_PRESHARPEN_PreSharpenEdgeExtAttr_RobustWgtAgainstMF_HLD_auto);
}
