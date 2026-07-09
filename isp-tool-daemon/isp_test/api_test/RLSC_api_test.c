
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

static int test_RLSC_RadialShadingAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	attr.Enable = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_Enable(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_Enable);
}

static int test_RLSC_RadialShadingAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_enOpType(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_enOpType);
}

static int test_RLSC_RadialShadingAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_UpdateInterval);
}

static int test_RLSC_RadialShadingAttr_CenterX(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CenterX != 960) {
		TEST_FAIL("test default value: 960 != %d\n", (int)attr.CenterX);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CenterX = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.CenterX = 8191;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.CenterX = rand_range(0, 8191);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	attr.CenterX = rand_range(8192, 65535);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.CenterX = 960;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_CenterX(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_CenterX);
}

static int test_RLSC_RadialShadingAttr_CenterY(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CenterY != 540) {
		TEST_FAIL("test default value: 540 != %d\n", (int)attr.CenterY);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CenterY = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.CenterY = 8191;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.CenterY = rand_range(0, 8191);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	attr.CenterY = rand_range(8192, 65535);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.CenterY = 540;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_CenterY(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_CenterY);
}

static int test_RLSC_RadialShadingAttr_RadiusScaleRGB(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.RadiusScaleRGB != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.RadiusScaleRGB);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.RadiusScaleRGB = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.RadiusScaleRGB = 32767;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.RadiusScaleRGB = rand_range(0, 32767);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	attr.RadiusScaleRGB = rand_range(32768, 65535);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.RadiusScaleRGB = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_RadiusScaleRGB(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_RadiusScaleRGB);
}

static int test_RLSC_RadialShadingAttr_RadiusStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.RadiusStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.RadiusStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.RadiusStr = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.RadiusStr = 4095;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.RadiusStr = rand_range(0, 4095);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	attr.stManual.RadiusStr = rand_range(4096, 65535);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.RadiusStr = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_RadiusStr_manual(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_RadiusStr_manual);
}

static int test_RLSC_RadialShadingAttr_RadiusStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.RadiusStr, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.RadiusStr[j] = 0;
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.RadiusStr[j] = 4095;
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.RadiusStr[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
		attr.stAuto.RadiusStr[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	memcpy(attr.stAuto.RadiusStr, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_RadiusStr_auto(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_RadiusStr_auto);
}

static int test_RLSC_RadialShadingAttr_NDStr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.NDStr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.NDStr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.NDStr = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.NDStr = 255;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.NDStr = rand_range(0, 255);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	attr.stManual.NDStr = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_NDStr_manual(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_NDStr_manual);
}

static int test_RLSC_RadialShadingAttr_NDStr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.NDStr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NDStr[j] = 0;
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.NDStr[j] = 255;
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.NDStr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.NDStr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_NDStr_auto(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_NDStr_auto);
}

static int test_RLSC_RadialShadingAttr_NDThr_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.NDThr != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.stManual.NDThr);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.NDThr = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.NDThr = 255;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.NDThr = rand_range(0, 255);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	attr.stManual.NDThr = 0;
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_NDThr_manual(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_NDThr_manual);
}

static int test_RLSC_RadialShadingAttr_NDThr_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val_0, attr.stAuto.NDThr, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.NDThr[j] = 0;
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.NDThr[j] = 255;
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.NDThr[j] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
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
	memcpy(attr.stAuto.NDThr, restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetRadialShadingAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingAttr_NDThr_auto(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingAttr_NDThr_auto);
}

static int test_RLSC_RadialShadingGainLutAttr_GGain(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_RADIAL_SHADING_GAIN_LUT_ATTR_S attr;

	ret = CVI_ISP_GetRadialShadingGainLutAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetRadialShadingGainLutAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val, attr.GGain, sizeof(CVI_U16) * 32) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 32; ++i) {
		attr.GGain[i] = 0;
	}
	ret = CVI_ISP_SetRadialShadingGainLutAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 32; ++i) {
		attr.GGain[i] = 4095;
	}
	ret = CVI_ISP_SetRadialShadingGainLutAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 32; i++) {
		attr.GGain[i] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetRadialShadingGainLutAttr(ViPipe, &attr);
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
	for (int i = 0; i < 32; i++) {
		attr.GGain[i] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetRadialShadingGainLutAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	memcpy(attr.GGain, default_val, sizeof(CVI_U16) * 32);
	ret = CVI_ISP_SetRadialShadingGainLutAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_RLSC_RadialShadingGainLutAttr_GGain(void)
{
	reg_api_test_fn(test_RLSC_RadialShadingGainLutAttr_GGain);
}
