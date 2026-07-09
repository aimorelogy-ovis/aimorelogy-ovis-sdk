
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

static int test_DEMOSAIC_DemosaicAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_Enable(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_Enable);
}

static int test_DEMOSAIC_DemosaicAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_enOpType(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_enOpType);
}

static int test_DEMOSAIC_DemosaicAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_UpdateInterval);
}

static int test_DEMOSAIC_DemosaicAttr_TuningMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.TuningMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.TuningMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.TuningMode = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.TuningMode = 1;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.TuningMode = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.TuningMode = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_TuningMode(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_TuningMode);
}

static int test_DEMOSAIC_DemosaicAttr_RbVtEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.RbVtEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.RbVtEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.RbVtEnable = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.RbVtEnable = 1;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.RbVtEnable = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.RbVtEnable = 1;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_RbVtEnable(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_RbVtEnable);
}

static int test_DEMOSAIC_DemosaicAttr_CoarseEdgeThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CoarseEdgeThr != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.CoarseEdgeThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CoarseEdgeThr = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.CoarseEdgeThr = 4095;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.CoarseEdgeThr = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.stManual.CoarseEdgeThr = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CoarseEdgeThr = 8;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_CoarseEdgeThr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_CoarseEdgeThr_manual);
}

static int test_DEMOSAIC_DemosaicAttr_CoarseEdgeThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_0, attr.stAuto.CoarseEdgeThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CoarseEdgeThr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.CoarseEdgeThr[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.CoarseEdgeThr[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
		attr.stAuto.CoarseEdgeThr[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.CoarseEdgeThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_CoarseEdgeThr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_CoarseEdgeThr_auto);
}

static int test_DEMOSAIC_DemosaicAttr_CoarseStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.CoarseStr != 2560) {
		TEST_FAIL("test default value: 2560 != %d\n", (int)attr.stManual.CoarseStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.CoarseStr = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.CoarseStr = 4095;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.CoarseStr = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.stManual.CoarseStr = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.CoarseStr = 2560;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_CoarseStr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_CoarseStr_manual);
}

static int test_DEMOSAIC_DemosaicAttr_CoarseStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560 };
	if (memcmp(default_val_0, attr.stAuto.CoarseStr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.CoarseStr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.CoarseStr[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.CoarseStr[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
		attr.stAuto.CoarseStr[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560 };
	memcpy(attr.stAuto.CoarseStr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_CoarseStr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_CoarseStr_auto);
}

static int test_DEMOSAIC_DemosaicAttr_FineEdgeThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FineEdgeThr != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.FineEdgeThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FineEdgeThr = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.FineEdgeThr = 4095;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.FineEdgeThr = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.stManual.FineEdgeThr = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.FineEdgeThr = 128;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_FineEdgeThr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_FineEdgeThr_manual);
}

static int test_DEMOSAIC_DemosaicAttr_FineEdgeThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	if (memcmp(default_val_0, attr.stAuto.FineEdgeThr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FineEdgeThr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.FineEdgeThr[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.FineEdgeThr[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
		attr.stAuto.FineEdgeThr[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	memcpy(attr.stAuto.FineEdgeThr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_FineEdgeThr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_FineEdgeThr_auto);
}

static int test_DEMOSAIC_DemosaicAttr_FineStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FineStr != 32) {
		TEST_FAIL("test default value: 32 != %d\n", (int)attr.stManual.FineStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FineStr = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.FineStr = 4095;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.FineStr = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.stManual.FineStr = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.FineStr = 32;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_FineStr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_FineStr_manual);
}

static int test_DEMOSAIC_DemosaicAttr_FineStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.FineStr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FineStr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.FineStr[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.FineStr[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
		attr.stAuto.FineStr[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.FineStr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_FineStr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_FineStr_auto);
}

static int test_DEMOSAIC_DemosaicAttr_RbSigLumaThd_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.RbSigLumaThd != 320) {
		TEST_FAIL("test default value: 320 != %d\n", (int)attr.stManual.RbSigLumaThd);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.RbSigLumaThd = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.RbSigLumaThd = 4095;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.RbSigLumaThd = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.stManual.RbSigLumaThd = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.RbSigLumaThd = 320;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_RbSigLumaThd_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_RbSigLumaThd_manual);
}

static int test_DEMOSAIC_DemosaicAttr_RbSigLumaThd_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 320,320,320,320,320,320,320,320,320,320,320,320,320,320,320,320 };
	if (memcmp(default_val_0, attr.stAuto.RbSigLumaThd, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.RbSigLumaThd[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.RbSigLumaThd[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.RbSigLumaThd[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
		attr.stAuto.RbSigLumaThd[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 320,320,320,320,320,320,320,320,320,320,320,320,320,320,320,320 };
	memcpy(attr.stAuto.RbSigLumaThd, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_RbSigLumaThd_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_RbSigLumaThd_auto);
}

static int test_DEMOSAIC_DemosaicAttr_FilterMode_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.FilterMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.FilterMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.FilterMode = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.FilterMode = 1;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.FilterMode = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
	attr.stManual.FilterMode = rand_range(2, 255);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.FilterMode = 0;
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_FilterMode_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_FilterMode_manual);
}

static int test_DEMOSAIC_DemosaicAttr_FilterMode_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.FilterMode, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.FilterMode[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.FilterMode[j] = 1;
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.FilterMode[j] = rand_range(0, 1);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
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
		attr.stAuto.FilterMode[j] = rand_range(2, 255);
	}
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.FilterMode, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicAttr_FilterMode_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicAttr_FilterMode_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.AntiFalseColorEnable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.AntiFalseColorEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.AntiFalseColorEnable = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.AntiFalseColorEnable = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.AntiFalseColorEnable = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.AntiFalseColorEnable = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorEnable(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorEnable);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.ProtectColorEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.ProtectColorEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.ProtectColorEnable = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.ProtectColorEnable = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.ProtectColorEnable = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.ProtectColorEnable = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorEnable(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorEnable);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DetailDetectLumaEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.DetailDetectLumaEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DetailDetectLumaEnable = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.DetailDetectLumaEnable = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.DetailDetectLumaEnable = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.DetailDetectLumaEnable = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaEnable(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaEnable);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothEnable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DetailSmoothEnable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.DetailSmoothEnable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DetailSmoothEnable = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.DetailSmoothEnable = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.DetailSmoothEnable = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.DetailSmoothEnable = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothEnable(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothEnable);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.DetailMode != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.DetailMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.DetailMode = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.DetailMode = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.DetailMode = rand_range(0, 1);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.DetailMode = 1;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailMode(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailMode);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.AntiFalseColorStr != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.AntiFalseColorStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.AntiFalseColorStr = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.AntiFalseColorStr = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.AntiFalseColorStr = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.AntiFalseColorStr = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorStr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorStr_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_0, attr.stAuto.AntiFalseColorStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.AntiFalseColorStr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.AntiFalseColorStr[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.AntiFalseColorStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.AntiFalseColorStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorStr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_AntiFalseColorStr_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_SatGainIn_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SatGainIn[0] != 200) {
		TEST_FAIL("test default value: 200 != %d\n", (int)attr.stManual.SatGainIn[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatGainIn[1] != 800) {
		TEST_FAIL("test default value: 800 != %d\n", (int)attr.stManual.SatGainIn[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SatGainIn[0] = 0;
	attr.stManual.SatGainIn[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.SatGainIn[0] = 4095;
	attr.stManual.SatGainIn[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.SatGainIn[0] = rand_range(0, 4095);
	attr.stManual.SatGainIn[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.SatGainIn[0] = rand_range(4096, 65535);
	attr.stManual.SatGainIn[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SatGainIn[0] = 200;
	attr.stManual.SatGainIn[1] = 800;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_SatGainIn_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_SatGainIn_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_SatGainIn_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	if (memcmp(default_val_0, attr.stAuto.SatGainIn[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800 };
	if (memcmp(default_val_1, attr.stAuto.SatGainIn[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainIn[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainIn[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.SatGainIn[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainIn[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.SatGainIn[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainIn[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.SatGainIn[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainIn[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	memcpy(attr.stAuto.SatGainIn[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800 };
	memcpy(attr.stAuto.SatGainIn[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_SatGainIn_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_SatGainIn_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_SatGainOut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SatGainOut[0] != 4095) {
		TEST_FAIL("test default value: 4095 != %d\n", (int)attr.stManual.SatGainOut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.SatGainOut[1] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SatGainOut[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SatGainOut[0] = 0;
	attr.stManual.SatGainOut[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.SatGainOut[0] = 4095;
	attr.stManual.SatGainOut[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.SatGainOut[0] = rand_range(0, 4095);
	attr.stManual.SatGainOut[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.SatGainOut[0] = rand_range(4096, 65535);
	attr.stManual.SatGainOut[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SatGainOut[0] = 4095;
	attr.stManual.SatGainOut[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_SatGainOut_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_SatGainOut_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_SatGainOut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	if (memcmp(default_val_0, attr.stAuto.SatGainOut[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_1, attr.stAuto.SatGainOut[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainOut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainOut[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.SatGainOut[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainOut[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.SatGainOut[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainOut[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.SatGainOut[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SatGainOut[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	memcpy(attr.stAuto.SatGainOut[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.SatGainOut[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_SatGainOut_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_SatGainOut_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainIn_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ProtectColorGainIn[0] != 20) {
		TEST_FAIL("test default value: 20 != %d\n", (int)attr.stManual.ProtectColorGainIn[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.ProtectColorGainIn[1] != 500) {
		TEST_FAIL("test default value: 500 != %d\n", (int)attr.stManual.ProtectColorGainIn[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ProtectColorGainIn[0] = 0;
	attr.stManual.ProtectColorGainIn[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.ProtectColorGainIn[0] = 4095;
	attr.stManual.ProtectColorGainIn[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.ProtectColorGainIn[0] = rand_range(0, 4095);
	attr.stManual.ProtectColorGainIn[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.ProtectColorGainIn[0] = rand_range(4096, 65535);
	attr.stManual.ProtectColorGainIn[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ProtectColorGainIn[0] = 20;
	attr.stManual.ProtectColorGainIn[1] = 500;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainIn_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainIn_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainIn_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	if (memcmp(default_val_0, attr.stAuto.ProtectColorGainIn[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500 };
	if (memcmp(default_val_1, attr.stAuto.ProtectColorGainIn[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainIn[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainIn[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.ProtectColorGainIn[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainIn[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.ProtectColorGainIn[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainIn[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.ProtectColorGainIn[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainIn[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 };
	memcpy(attr.stAuto.ProtectColorGainIn[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500 };
	memcpy(attr.stAuto.ProtectColorGainIn[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainIn_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainIn_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainOut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ProtectColorGainOut[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.ProtectColorGainOut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.ProtectColorGainOut[1] != 4095) {
		TEST_FAIL("test default value: 4095 != %d\n", (int)attr.stManual.ProtectColorGainOut[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ProtectColorGainOut[0] = 0;
	attr.stManual.ProtectColorGainOut[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.ProtectColorGainOut[0] = 4095;
	attr.stManual.ProtectColorGainOut[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.ProtectColorGainOut[0] = rand_range(0, 4095);
	attr.stManual.ProtectColorGainOut[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.ProtectColorGainOut[0] = rand_range(4096, 65535);
	attr.stManual.ProtectColorGainOut[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ProtectColorGainOut[0] = 0;
	attr.stManual.ProtectColorGainOut[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainOut_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainOut_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainOut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.ProtectColorGainOut[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	if (memcmp(default_val_1, attr.stAuto.ProtectColorGainOut[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainOut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainOut[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.ProtectColorGainOut[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainOut[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.ProtectColorGainOut[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainOut[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.ProtectColorGainOut[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ProtectColorGainOut[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.ProtectColorGainOut[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	memcpy(attr.stAuto.ProtectColorGainOut[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainOut_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_ProtectColorGainOut_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor1_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UserDefineProtectColor1 != 960) {
		TEST_FAIL("test default value: 960 != %d\n", (int)attr.stManual.UserDefineProtectColor1);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UserDefineProtectColor1 = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.UserDefineProtectColor1 = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.UserDefineProtectColor1 = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.UserDefineProtectColor1 = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.UserDefineProtectColor1 = 960;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor1_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor1_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor1_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 960,960,960,960,960,960,960,960,960,960,960,960,960,960,960,960 };
	if (memcmp(default_val_0, attr.stAuto.UserDefineProtectColor1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UserDefineProtectColor1[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.UserDefineProtectColor1[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.UserDefineProtectColor1[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.UserDefineProtectColor1[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 960,960,960,960,960,960,960,960,960,960,960,960,960,960,960,960 };
	memcpy(attr.stAuto.UserDefineProtectColor1, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor1_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor1_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor2_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UserDefineProtectColor2 != 560) {
		TEST_FAIL("test default value: 560 != %d\n", (int)attr.stManual.UserDefineProtectColor2);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UserDefineProtectColor2 = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.UserDefineProtectColor2 = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.UserDefineProtectColor2 = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.UserDefineProtectColor2 = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.UserDefineProtectColor2 = 560;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor2_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor2_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor2_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 560,560,560,560,560,560,560,560,560,560,560,560,560,560,560,560 };
	if (memcmp(default_val_0, attr.stAuto.UserDefineProtectColor2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UserDefineProtectColor2[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.UserDefineProtectColor2[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.UserDefineProtectColor2[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.UserDefineProtectColor2[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 560,560,560,560,560,560,560,560,560,560,560,560,560,560,560,560 };
	memcpy(attr.stAuto.UserDefineProtectColor2, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor2_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor2_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor3_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.UserDefineProtectColor3 != 960) {
		TEST_FAIL("test default value: 960 != %d\n", (int)attr.stManual.UserDefineProtectColor3);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.UserDefineProtectColor3 = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.UserDefineProtectColor3 = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.UserDefineProtectColor3 = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.UserDefineProtectColor3 = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.UserDefineProtectColor3 = 960;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor3_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor3_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor3_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 960,960,960,960,960,960,960,960,960,960,960,960,960,960,960,960 };
	if (memcmp(default_val_0, attr.stAuto.UserDefineProtectColor3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.UserDefineProtectColor3[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.UserDefineProtectColor3[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.UserDefineProtectColor3[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.UserDefineProtectColor3[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 960,960,960,960,960,960,960,960,960,960,960,960,960,960,960,960 };
	memcpy(attr.stAuto.UserDefineProtectColor3, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor3_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_UserDefineProtectColor3_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainIn_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeGainIn[0] != 150) {
		TEST_FAIL("test default value: 150 != %d\n", (int)attr.stManual.EdgeGainIn[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeGainIn[1] != 200) {
		TEST_FAIL("test default value: 200 != %d\n", (int)attr.stManual.EdgeGainIn[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeGainIn[0] = 0;
	attr.stManual.EdgeGainIn[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.EdgeGainIn[0] = 4095;
	attr.stManual.EdgeGainIn[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.EdgeGainIn[0] = rand_range(0, 4095);
	attr.stManual.EdgeGainIn[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.EdgeGainIn[0] = rand_range(4096, 65535);
	attr.stManual.EdgeGainIn[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeGainIn[0] = 150;
	attr.stManual.EdgeGainIn[1] = 200;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainIn_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainIn_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainIn_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	if (memcmp(default_val_0, attr.stAuto.EdgeGainIn[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	if (memcmp(default_val_1, attr.stAuto.EdgeGainIn[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainIn[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainIn[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeGainIn[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainIn[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeGainIn[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainIn[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeGainIn[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainIn[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	memcpy(attr.stAuto.EdgeGainIn[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200 };
	memcpy(attr.stAuto.EdgeGainIn[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainIn_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainIn_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainOut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeGainOut[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.EdgeGainOut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.EdgeGainOut[1] != 4095) {
		TEST_FAIL("test default value: 4095 != %d\n", (int)attr.stManual.EdgeGainOut[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeGainOut[0] = 0;
	attr.stManual.EdgeGainOut[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.EdgeGainOut[0] = 4095;
	attr.stManual.EdgeGainOut[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.EdgeGainOut[0] = rand_range(0, 4095);
	attr.stManual.EdgeGainOut[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.EdgeGainOut[0] = rand_range(4096, 65535);
	attr.stManual.EdgeGainOut[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeGainOut[0] = 0;
	attr.stManual.EdgeGainOut[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainOut_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainOut_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainOut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.EdgeGainOut[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	if (memcmp(default_val_1, attr.stAuto.EdgeGainOut[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainOut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainOut[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeGainOut[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainOut[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeGainOut[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainOut[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeGainOut[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeGainOut[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.EdgeGainOut[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	memcpy(attr.stAuto.EdgeGainOut[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainOut_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeGainOut_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailGainIn_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailGainIn[0] != 10) {
		TEST_FAIL("test default value: 10 != %d\n", (int)attr.stManual.DetailGainIn[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.DetailGainIn[1] != 150) {
		TEST_FAIL("test default value: 150 != %d\n", (int)attr.stManual.DetailGainIn[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailGainIn[0] = 0;
	attr.stManual.DetailGainIn[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailGainIn[0] = 4095;
	attr.stManual.DetailGainIn[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailGainIn[0] = rand_range(0, 4095);
	attr.stManual.DetailGainIn[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailGainIn[0] = rand_range(4096, 65535);
	attr.stManual.DetailGainIn[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailGainIn[0] = 10;
	attr.stManual.DetailGainIn[1] = 150;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailGainIn_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailGainIn_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailGainIn_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	if (memcmp(default_val_0, attr.stAuto.DetailGainIn[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	if (memcmp(default_val_1, attr.stAuto.DetailGainIn[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGainIn[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGainIn[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailGainIn[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGainIn[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailGainIn[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGainIn[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailGainIn[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGainIn[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	memcpy(attr.stAuto.DetailGainIn[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150 };
	memcpy(attr.stAuto.DetailGainIn[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailGainIn_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailGainIn_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailGaintOut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailGaintOut[0] != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.DetailGaintOut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.DetailGaintOut[1] != 4095) {
		TEST_FAIL("test default value: 4095 != %d\n", (int)attr.stManual.DetailGaintOut[1]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailGaintOut[0] = 0;
	attr.stManual.DetailGaintOut[1] = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailGaintOut[0] = 4095;
	attr.stManual.DetailGaintOut[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailGaintOut[0] = rand_range(0, 4095);
	attr.stManual.DetailGaintOut[1] = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailGaintOut[0] = rand_range(4096, 65535);
	attr.stManual.DetailGaintOut[1] = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailGaintOut[0] = 1;
	attr.stManual.DetailGaintOut[1] = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailGaintOut_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailGaintOut_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailGaintOut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_0, attr.stAuto.DetailGaintOut[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	if (memcmp(default_val_1, attr.stAuto.DetailGaintOut[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGaintOut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGaintOut[1][j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailGaintOut[0][j] = 4095;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGaintOut[1][j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailGaintOut[0][j] = rand_range(0, 4095);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGaintOut[1][j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailGaintOut[0][j] = rand_range(4096, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailGaintOut[1][j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.DetailGaintOut[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095 };
	memcpy(attr.stAuto.DetailGaintOut[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailGaintOut_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailGaintOut_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailDetectLumaStr != 480) {
		TEST_FAIL("test default value: 480 != %d\n", (int)attr.stManual.DetailDetectLumaStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailDetectLumaStr = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailDetectLumaStr = 4095;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailDetectLumaStr = rand_range(0, 4095);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailDetectLumaStr = rand_range(4096, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailDetectLumaStr = 480;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaStr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaStr_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 480,480,480,480,480,480,480,480,480,480,480,480,480,480,480,480 };
	if (memcmp(default_val_0, attr.stAuto.DetailDetectLumaStr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailDetectLumaStr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailDetectLumaStr[j] = 4095;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailDetectLumaStr[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailDetectLumaStr[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 480,480,480,480,480,480,480,480,480,480,480,480,480,480,480,480 };
	memcpy(attr.stAuto.DetailDetectLumaStr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaStr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailDetectLumaStr_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailSmoothStr != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.DetailSmoothStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailSmoothStr = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailSmoothStr = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailSmoothStr = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailSmoothStr = 128;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothStr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothStr_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.DetailSmoothStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailSmoothStr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothStr[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.DetailSmoothStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothStr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothStr_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailWgtThr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.DetailWgtThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailWgtThr = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailWgtThr = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailWgtThr = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailWgtThr = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtThr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtThr_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.DetailWgtThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailWgtThr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtThr[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtThr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.DetailWgtThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtThr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtThr_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMin_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailWgtMin != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.DetailWgtMin);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailWgtMin = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailWgtMin = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailWgtMin = rand_range(0, 256);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailWgtMin = rand_range(257, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailWgtMin = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMin_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMin_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMin_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.DetailWgtMin, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailWgtMin[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtMin[j] = 256;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtMin[j] = rand_range(0, 256);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailWgtMin[j] = rand_range(257, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.DetailWgtMin, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMin_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMin_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMax_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailWgtMax != 256) {
		TEST_FAIL("test default value: 256 != %d\n", (int)attr.stManual.DetailWgtMax);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailWgtMax = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailWgtMax = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailWgtMax = rand_range(0, 256);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailWgtMax = rand_range(257, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailWgtMax = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMax_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMax_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMax_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	if (memcmp(default_val_0, attr.stAuto.DetailWgtMax, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailWgtMax[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtMax[j] = 256;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtMax[j] = rand_range(0, 256);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailWgtMax[j] = rand_range(257, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	memcpy(attr.stAuto.DetailWgtMax, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMax_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtMax_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtSlope_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailWgtSlope != 256) {
		TEST_FAIL("test default value: 256 != %d\n", (int)attr.stManual.DetailWgtSlope);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailWgtSlope = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailWgtSlope = 1024;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailWgtSlope = rand_range(0, 1024);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailWgtSlope = rand_range(1025, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailWgtSlope = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtSlope_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtSlope_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtSlope_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	if (memcmp(default_val_0, attr.stAuto.DetailWgtSlope, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailWgtSlope[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtSlope[j] = 1024;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailWgtSlope[j] = rand_range(0, 1024);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailWgtSlope[j] = rand_range(1025, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	memcpy(attr.stAuto.DetailWgtSlope, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtSlope_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailWgtSlope_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtNp_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeWgtNp != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.stManual.EdgeWgtNp);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeWgtNp = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.EdgeWgtNp = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.EdgeWgtNp = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.EdgeWgtNp = 16;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtNp_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtNp_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtNp_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
	if (memcmp(default_val_0, attr.stAuto.EdgeWgtNp, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeWgtNp[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtNp[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtNp[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.EdgeWgtNp, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtNp_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtNp_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeWgtThr != 160) {
		TEST_FAIL("test default value: 160 != %d\n", (int)attr.stManual.EdgeWgtThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeWgtThr = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.EdgeWgtThr = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.EdgeWgtThr = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.EdgeWgtThr = 160;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtThr_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtThr_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160 };
	if (memcmp(default_val_0, attr.stAuto.EdgeWgtThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeWgtThr[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtThr[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtThr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160 };
	memcpy(attr.stAuto.EdgeWgtThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtThr_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtThr_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMin_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeWgtMin != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.EdgeWgtMin);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeWgtMin = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.EdgeWgtMin = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.EdgeWgtMin = rand_range(0, 256);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.EdgeWgtMin = rand_range(257, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeWgtMin = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMin_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMin_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMin_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.EdgeWgtMin, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeWgtMin[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtMin[j] = 256;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtMin[j] = rand_range(0, 256);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeWgtMin[j] = rand_range(257, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.EdgeWgtMin, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMin_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMin_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMax_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeWgtMax != 256) {
		TEST_FAIL("test default value: 256 != %d\n", (int)attr.stManual.EdgeWgtMax);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeWgtMax = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.EdgeWgtMax = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.EdgeWgtMax = rand_range(0, 256);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.EdgeWgtMax = rand_range(257, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeWgtMax = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMax_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMax_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMax_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	if (memcmp(default_val_0, attr.stAuto.EdgeWgtMax, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeWgtMax[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtMax[j] = 256;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtMax[j] = rand_range(0, 256);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeWgtMax[j] = rand_range(257, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	memcpy(attr.stAuto.EdgeWgtMax, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMax_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtMax_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtSlope_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.EdgeWgtSlope != 256) {
		TEST_FAIL("test default value: 256 != %d\n", (int)attr.stManual.EdgeWgtSlope);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.EdgeWgtSlope = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.EdgeWgtSlope = 1024;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.EdgeWgtSlope = rand_range(0, 1024);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.EdgeWgtSlope = rand_range(1025, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.EdgeWgtSlope = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtSlope_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtSlope_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtSlope_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	if (memcmp(default_val_0, attr.stAuto.EdgeWgtSlope, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.EdgeWgtSlope[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtSlope[j] = 1024;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.EdgeWgtSlope[j] = rand_range(0, 1024);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.EdgeWgtSlope[j] = rand_range(1025, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	memcpy(attr.stAuto.EdgeWgtSlope, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtSlope_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_EdgeWgtSlope_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailSmoothMapTh != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.DetailSmoothMapTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailSmoothMapTh = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailSmoothMapTh = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailSmoothMapTh = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailSmoothMapTh = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapTh_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapTh_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.DetailSmoothMapTh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailSmoothMapTh[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapTh[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapTh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.DetailSmoothMapTh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapTh_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapTh_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMin_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailSmoothMapMin != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.DetailSmoothMapMin);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailSmoothMapMin = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailSmoothMapMin = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailSmoothMapMin = rand_range(0, 256);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailSmoothMapMin = rand_range(257, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailSmoothMapMin = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMin_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMin_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMin_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.DetailSmoothMapMin, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailSmoothMapMin[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapMin[j] = 256;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapMin[j] = rand_range(0, 256);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailSmoothMapMin[j] = rand_range(257, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.DetailSmoothMapMin, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMin_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMin_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMax_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailSmoothMapMax != 256) {
		TEST_FAIL("test default value: 256 != %d\n", (int)attr.stManual.DetailSmoothMapMax);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailSmoothMapMax = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailSmoothMapMax = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailSmoothMapMax = rand_range(0, 256);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailSmoothMapMax = rand_range(257, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailSmoothMapMax = 256;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMax_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMax_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMax_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	if (memcmp(default_val_0, attr.stAuto.DetailSmoothMapMax, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailSmoothMapMax[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapMax[j] = 256;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapMax[j] = rand_range(0, 256);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailSmoothMapMax[j] = rand_range(257, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256 };
	memcpy(attr.stAuto.DetailSmoothMapMax, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMax_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapMax_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapSlope_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.DetailSmoothMapSlope != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.DetailSmoothMapSlope);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.DetailSmoothMapSlope = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.DetailSmoothMapSlope = 1024;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.DetailSmoothMapSlope = rand_range(0, 1024);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.DetailSmoothMapSlope = rand_range(1025, 65535);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.DetailSmoothMapSlope = 128;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapSlope_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapSlope_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapSlope_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.DetailSmoothMapSlope, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.DetailSmoothMapSlope[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapSlope[j] = 1024;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.DetailSmoothMapSlope[j] = rand_range(0, 1024);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
		attr.stAuto.DetailSmoothMapSlope[j] = rand_range(1025, 65535);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.DetailSmoothMapSlope, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapSlope_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_DetailSmoothMapSlope_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_LumaWgt_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaWgt != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.LumaWgt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaWgt = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LumaWgt = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LumaWgt = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.LumaWgt = 128;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_LumaWgt_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_LumaWgt_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_LumaWgt_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.LumaWgt, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaWgt[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumaWgt[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumaWgt[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.LumaWgt, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_LumaWgt_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_LumaWgt_auto);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_SharpenGain_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SharpenGain != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.SharpenGain);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SharpenGain = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.SharpenGain = 255;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.SharpenGain = rand_range(0, 255);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	attr.stManual.SharpenGain = 0;
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_SharpenGain_manual(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_SharpenGain_manual);
}

static int test_DEMOSAIC_DemosaicDemoireAttr_SharpenGain_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S attr;

	ret = CVI_ISP_GetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetDemosaicDemoireAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.SharpenGain, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.SharpenGain[j] = 0;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.SharpenGain[j] = 255;
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.SharpenGain[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.SharpenGain, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetDemosaicDemoireAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_DEMOSAIC_DemosaicDemoireAttr_SharpenGain_auto(void)
{
	reg_api_test_fn(test_DEMOSAIC_DemosaicDemoireAttr_SharpenGain_auto);
}
