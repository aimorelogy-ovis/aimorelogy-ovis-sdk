
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

static int test_PFR_PFRAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_Enable(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_Enable);
}

static int test_PFR_PFRAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_enOpType(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_enOpType);
}

static int test_PFR_PFRAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_UpdateInterval);
}

static int test_PFR_PFRAttr_TuningMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.TuningMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.TuningMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.TuningMode = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_TuningMode(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_TuningMode);
}

static int test_PFR_PFRAttr_LumaEN(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.LumaEN != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.LumaEN);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.LumaEN = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.LumaEN = 1;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.LumaEN = rand_range(0, 1);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	attr.LumaEN = 1;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_LumaEN(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_LumaEN);
}

static int test_PFR_PFRAttr_UVENLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_BOOL default_val[2] = { 1,1 };
	if (memcmp(default_val, attr.UVENLut, sizeof(CVI_BOOL) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.UVENLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.UVENLut[i] = 1;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.UVENLut[i] = rand_range(0, 1);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.UVENLut, default_val, sizeof(CVI_BOOL) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_UVENLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_UVENLut);
}

static int test_PFR_PFRAttr_HueENLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_BOOL default_val[2] = { 1,1 };
	if (memcmp(default_val, attr.HueENLut, sizeof(CVI_BOOL) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.HueENLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.HueENLut[i] = 1;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.HueENLut[i] = rand_range(0, 1);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.HueENLut, default_val, sizeof(CVI_BOOL) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_HueENLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_HueENLut);
}

static int test_PFR_PFRAttr_ULut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[2] = { 175,144 };
	if (memcmp(default_val, attr.ULut, sizeof(CVI_U8) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.ULut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.ULut[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.ULut[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.ULut, default_val, sizeof(CVI_U8) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_ULut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_ULut);
}

static int test_PFR_PFRAttr_VLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[2] = { 144,175 };
	if (memcmp(default_val, attr.VLut, sizeof(CVI_U8) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.VLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.VLut[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.VLut[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.VLut, default_val, sizeof(CVI_U8) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_VLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_VLut);
}

static int test_PFR_PFRAttr_UVDiffThLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[2] = { 16,16 };
	if (memcmp(default_val, attr.UVDiffThLut, sizeof(CVI_U8) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.UVDiffThLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.UVDiffThLut[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.UVDiffThLut[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.UVDiffThLut, default_val, sizeof(CVI_U8) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_UVDiffThLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_UVDiffThLut);
}

static int test_PFR_PFRAttr_UVDiffLut0(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[5] = { 255,132,12,0,0 };
	if (memcmp(default_val, attr.UVDiffLut0, sizeof(CVI_U8) * 5) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 5; ++i) {
		attr.UVDiffLut0[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 5; ++i) {
		attr.UVDiffLut0[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 5; i++) {
		attr.UVDiffLut0[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.UVDiffLut0, default_val, sizeof(CVI_U8) * 5);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_UVDiffLut0(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_UVDiffLut0);
}

static int test_PFR_PFRAttr_UVDiffLut1(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[5] = { 255,132,12,0,0 };
	if (memcmp(default_val, attr.UVDiffLut1, sizeof(CVI_U8) * 5) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 5; ++i) {
		attr.UVDiffLut1[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 5; ++i) {
		attr.UVDiffLut1[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 5; i++) {
		attr.UVDiffLut1[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.UVDiffLut1, default_val, sizeof(CVI_U8) * 5);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_UVDiffLut1(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_UVDiffLut1);
}

static int test_PFR_PFRAttr_RWetLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[2] = { 255,232 };
	if (memcmp(default_val, attr.RWetLut, sizeof(CVI_U8) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.RWetLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.RWetLut[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.RWetLut[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.RWetLut, default_val, sizeof(CVI_U8) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_RWetLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_RWetLut);
}

static int test_PFR_PFRAttr_BWetLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[2] = { 255,232 };
	if (memcmp(default_val, attr.BWetLut, sizeof(CVI_U8) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.BWetLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.BWetLut[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.BWetLut[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.BWetLut, default_val, sizeof(CVI_U8) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_BWetLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_BWetLut);
}

static int test_PFR_PFRAttr_HueLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val[2] = { 260,260 };
	if (memcmp(default_val, attr.HueLut, sizeof(CVI_U16) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.HueLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.HueLut[i] = 359;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.HueLut[i] = rand_range(0, 359);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	for (int i = 0; i < 2; i++) {
		attr.HueLut[i] = rand_range(360, 65535);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	memcpy(attr.HueLut, default_val, sizeof(CVI_U16) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_HueLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_HueLut);
}

static int test_PFR_PFRAttr_HueRngLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[2] = { 64,64 };
	if (memcmp(default_val, attr.HueRngLut, sizeof(CVI_U8) * 2) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 2; ++i) {
		attr.HueRngLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 2; ++i) {
		attr.HueRngLut[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 2; i++) {
		attr.HueRngLut[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.HueRngLut, default_val, sizeof(CVI_U8) * 2);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_HueRngLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_HueRngLut);
}

static int test_PFR_PFRAttr_HueTh(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.HueTh != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.HueTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.HueTh = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.HueTh = 255;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.HueTh = rand_range(0, 255);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	attr.HueTh = 128;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_HueTh(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_HueTh);
}

static int test_PFR_PFRAttr_EdgeThLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[4] = { 8,12,16,32 };
	if (memcmp(default_val, attr.EdgeThLut, sizeof(CVI_U8) * 4) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 4; ++i) {
		attr.EdgeThLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 4; ++i) {
		attr.EdgeThLut[i] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 4; i++) {
		attr.EdgeThLut[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.EdgeThLut, default_val, sizeof(CVI_U8) * 4);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_EdgeThLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_EdgeThLut);
}

static int test_PFR_PFRAttr_GDiffLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[5] = { 16,32,48,64,80 };
	if (memcmp(default_val, attr.GDiffLut, sizeof(CVI_U8) * 5) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 5; ++i) {
		attr.GDiffLut[i] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 5; ++i) {
		attr.GDiffLut[i] = 127;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 5; i++) {
		attr.GDiffLut[i] = rand_range(0, 127);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	for (int i = 0; i < 5; i++) {
		attr.GDiffLut[i] = rand_range(128, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	memcpy(attr.GDiffLut, default_val, sizeof(CVI_U8) * 5);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_GDiffLut(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_GDiffLut);
}

static int test_PFR_PFRAttr_LumaIniVal_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumaIniVal != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.LumaIniVal);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumaIniVal = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LumaIniVal = 255;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LumaIniVal = rand_range(0, 255);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	attr.stManual.LumaIniVal = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_LumaIniVal_manual(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_LumaIniVal_manual);
}

static int test_PFR_PFRAttr_LumaIniVal_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.LumaIniVal, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumaIniVal[j] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumaIniVal[j] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumaIniVal[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.LumaIniVal, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_LumaIniVal_auto(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_LumaIniVal_auto);
}

static int test_PFR_PFRAttr_LumLevelTh_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumLevelTh != 240) {
		TEST_FAIL("test default value: 240 != %d\n", (int)attr.stManual.LumLevelTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumLevelTh = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LumLevelTh = 255;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LumLevelTh = rand_range(0, 255);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	attr.stManual.LumLevelTh = 240;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_LumLevelTh_manual(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_LumLevelTh_manual);
}

static int test_PFR_PFRAttr_LumLevelTh_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	if (memcmp(default_val_0, attr.stAuto.LumLevelTh, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelTh[j] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumLevelTh[j] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumLevelTh[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240 };
	memcpy(attr.stAuto.LumLevelTh, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_LumLevelTh_auto(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_LumLevelTh_auto);
}

static int test_PFR_PFRAttr_LumLevelLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LumLevelLut[0] != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.LumLevelLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[1] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.LumLevelLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[2] != 144) {
		TEST_FAIL("test default value: 144 != %d\n", (int)attr.stManual.LumLevelLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[3] != 160) {
		TEST_FAIL("test default value: 160 != %d\n", (int)attr.stManual.LumLevelLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[4] != 176) {
		TEST_FAIL("test default value: 176 != %d\n", (int)attr.stManual.LumLevelLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[5] != 192) {
		TEST_FAIL("test default value: 192 != %d\n", (int)attr.stManual.LumLevelLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[6] != 208) {
		TEST_FAIL("test default value: 208 != %d\n", (int)attr.stManual.LumLevelLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[7] != 224) {
		TEST_FAIL("test default value: 224 != %d\n", (int)attr.stManual.LumLevelLut[7]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[8] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[8]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[9] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[9]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[10] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[10]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[11] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[11]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[12] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[12]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[13] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[13]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[14] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[14]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[15] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[15]);
		return CVI_FAILURE;
	}
	if (attr.stManual.LumLevelLut[16] != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.stManual.LumLevelLut[16]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LumLevelLut[0] = 0;
	attr.stManual.LumLevelLut[1] = 0;
	attr.stManual.LumLevelLut[2] = 0;
	attr.stManual.LumLevelLut[3] = 0;
	attr.stManual.LumLevelLut[4] = 0;
	attr.stManual.LumLevelLut[5] = 0;
	attr.stManual.LumLevelLut[6] = 0;
	attr.stManual.LumLevelLut[7] = 0;
	attr.stManual.LumLevelLut[8] = 0;
	attr.stManual.LumLevelLut[9] = 0;
	attr.stManual.LumLevelLut[10] = 0;
	attr.stManual.LumLevelLut[11] = 0;
	attr.stManual.LumLevelLut[12] = 0;
	attr.stManual.LumLevelLut[13] = 0;
	attr.stManual.LumLevelLut[14] = 0;
	attr.stManual.LumLevelLut[15] = 0;
	attr.stManual.LumLevelLut[16] = 0;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LumLevelLut[0] = 255;
	attr.stManual.LumLevelLut[1] = 255;
	attr.stManual.LumLevelLut[2] = 255;
	attr.stManual.LumLevelLut[3] = 255;
	attr.stManual.LumLevelLut[4] = 255;
	attr.stManual.LumLevelLut[5] = 255;
	attr.stManual.LumLevelLut[6] = 255;
	attr.stManual.LumLevelLut[7] = 255;
	attr.stManual.LumLevelLut[8] = 255;
	attr.stManual.LumLevelLut[9] = 255;
	attr.stManual.LumLevelLut[10] = 255;
	attr.stManual.LumLevelLut[11] = 255;
	attr.stManual.LumLevelLut[12] = 255;
	attr.stManual.LumLevelLut[13] = 255;
	attr.stManual.LumLevelLut[14] = 255;
	attr.stManual.LumLevelLut[15] = 255;
	attr.stManual.LumLevelLut[16] = 255;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LumLevelLut[0] = rand_range(0, 255);
	attr.stManual.LumLevelLut[1] = rand_range(0, 255);
	attr.stManual.LumLevelLut[2] = rand_range(0, 255);
	attr.stManual.LumLevelLut[3] = rand_range(0, 255);
	attr.stManual.LumLevelLut[4] = rand_range(0, 255);
	attr.stManual.LumLevelLut[5] = rand_range(0, 255);
	attr.stManual.LumLevelLut[6] = rand_range(0, 255);
	attr.stManual.LumLevelLut[7] = rand_range(0, 255);
	attr.stManual.LumLevelLut[8] = rand_range(0, 255);
	attr.stManual.LumLevelLut[9] = rand_range(0, 255);
	attr.stManual.LumLevelLut[10] = rand_range(0, 255);
	attr.stManual.LumLevelLut[11] = rand_range(0, 255);
	attr.stManual.LumLevelLut[12] = rand_range(0, 255);
	attr.stManual.LumLevelLut[13] = rand_range(0, 255);
	attr.stManual.LumLevelLut[14] = rand_range(0, 255);
	attr.stManual.LumLevelLut[15] = rand_range(0, 255);
	attr.stManual.LumLevelLut[16] = rand_range(0, 255);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	attr.stManual.LumLevelLut[0] = 0;
	attr.stManual.LumLevelLut[1] = 128;
	attr.stManual.LumLevelLut[2] = 144;
	attr.stManual.LumLevelLut[3] = 160;
	attr.stManual.LumLevelLut[4] = 176;
	attr.stManual.LumLevelLut[5] = 192;
	attr.stManual.LumLevelLut[6] = 208;
	attr.stManual.LumLevelLut[7] = 224;
	attr.stManual.LumLevelLut[8] = 255;
	attr.stManual.LumLevelLut[9] = 255;
	attr.stManual.LumLevelLut[10] = 255;
	attr.stManual.LumLevelLut[11] = 255;
	attr.stManual.LumLevelLut[12] = 255;
	attr.stManual.LumLevelLut[13] = 255;
	attr.stManual.LumLevelLut[14] = 255;
	attr.stManual.LumLevelLut[15] = 255;
	attr.stManual.LumLevelLut[16] = 255;
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_LumLevelLut_manual(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_LumLevelLut_manual);
}

static int test_PFR_PFRAttr_LumLevelLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_PFR_ATTR_S attr;

	ret = CVI_ISP_GetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetPFRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.LumLevelLut[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_1, attr.stAuto.LumLevelLut[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 144,144,144,144,144,144,144,144,144,144,144,144,144,144,144,144 };
	if (memcmp(default_val_2, attr.stAuto.LumLevelLut[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160 };
	if (memcmp(default_val_3, attr.stAuto.LumLevelLut[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176 };
	if (memcmp(default_val_4, attr.stAuto.LumLevelLut[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	if (memcmp(default_val_5, attr.stAuto.LumLevelLut[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 208,208,208,208,208,208,208,208,208,208,208,208,208,208,208,208 };
	if (memcmp(default_val_6, attr.stAuto.LumLevelLut[6], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	if (memcmp(default_val_7, attr.stAuto.LumLevelLut[7], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_8, attr.stAuto.LumLevelLut[8], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_9, attr.stAuto.LumLevelLut[9], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_10, attr.stAuto.LumLevelLut[10], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_11, attr.stAuto.LumLevelLut[11], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_12, attr.stAuto.LumLevelLut[12], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_13, attr.stAuto.LumLevelLut[13], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_14, attr.stAuto.LumLevelLut[14], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_15, attr.stAuto.LumLevelLut[15], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	if (memcmp(default_val_16, attr.stAuto.LumLevelLut[16], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[7][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[8][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[9][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[10][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[11][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[12][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[13][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[14][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[15][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[16][j] = 0;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumLevelLut[0][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[1][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[2][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[3][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[4][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[5][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[6][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[7][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[8][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[9][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[10][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[11][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[12][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[13][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[14][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[15][j] = 255;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[16][j] = 255;
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.LumLevelLut[0][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[1][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[2][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[3][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[4][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[5][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[6][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[7][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[8][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[9][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[10][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[11][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[12][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[13][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[14][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[15][j] = rand_range(0, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.LumLevelLut[16][j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.LumLevelLut[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.LumLevelLut[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 144,144,144,144,144,144,144,144,144,144,144,144,144,144,144,144 };
	memcpy(attr.stAuto.LumLevelLut[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160 };
	memcpy(attr.stAuto.LumLevelLut[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176 };
	memcpy(attr.stAuto.LumLevelLut[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	memcpy(attr.stAuto.LumLevelLut[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 208,208,208,208,208,208,208,208,208,208,208,208,208,208,208,208 };
	memcpy(attr.stAuto.LumLevelLut[6], restore_val_6, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224 };
	memcpy(attr.stAuto.LumLevelLut[7], restore_val_7, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[8], restore_val_8, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[9], restore_val_9, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[10], restore_val_10, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[11], restore_val_11, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[12], restore_val_12, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[13], restore_val_13, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[14], restore_val_14, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[15], restore_val_15, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
	memcpy(attr.stAuto.LumLevelLut[16], restore_val_16, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetPFRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_PFR_PFRAttr_LumLevelLut_auto(void)
{
	reg_api_test_fn(test_PFR_PFRAttr_LumLevelLut_auto);
}
