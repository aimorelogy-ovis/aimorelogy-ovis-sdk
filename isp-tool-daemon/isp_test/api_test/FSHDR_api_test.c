
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

static int test_FSHDR_FSHDRAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_enOpType(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_enOpType);
}

static int test_FSHDR_FSHDRAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_UpdateInterval);
}

static int test_FSHDR_FSHDRAttr_FusionMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.FusionMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionMode = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionMode = 2;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionMode = rand_range(0, 2);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.FusionMode = rand_range(3, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.FusionMode = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionMode(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionMode);
}

static int test_FSHDR_FSHDRAttr_FusionDbgMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionDbgMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.FusionDbgMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionDbgMode = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionDbgMode = 4;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionDbgMode = rand_range(0, 4);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.FusionDbgMode = rand_range(5, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.FusionDbgMode = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionDbgMode(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionDbgMode);
}

static int test_FSHDR_FSHDRAttr_FusionYsel(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionYsel != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.FusionYsel);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionYsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionYsel = 2;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionYsel = rand_range(0, 2);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.FusionYsel = rand_range(3, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.FusionYsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionYsel(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionYsel);
}

static int test_FSHDR_FSHDRAttr_FusionBldNsel(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionBldNsel != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.FusionBldNsel);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionBldNsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionBldNsel = 1;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionBldNsel = rand_range(0, 1);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.FusionBldNsel = rand_range(2, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.FusionBldNsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionBldNsel(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionBldNsel);
}

static int test_FSHDR_FSHDRAttr_FusionBldDsel(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionBldDsel != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.FusionBldDsel);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionBldDsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionBldDsel = 1;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionBldDsel = rand_range(0, 1);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.FusionBldDsel = rand_range(2, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.FusionBldDsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionBldDsel(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionBldDsel);
}

static int test_FSHDR_FSHDRAttr_FusionDS1Th(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionDS1Th != 4095) {
		TEST_FAIL("test default value: 4095 != %d\n", (int)attr.FusionDS1Th);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionDS1Th = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionDS1Th = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionDS1Th = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.FusionDS1Th = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.FusionDS1Th = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionDS1Th(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionDS1Th);
}

static int test_FSHDR_FSHDRAttr_FusionDS1Step(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionDS1Step != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.FusionDS1Step);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionDS1Step = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionDS1Step = 255;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionDS1Step = rand_range(0, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	attr.FusionDS1Step = 16;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionDS1Step(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionDS1Step);
}

static int test_FSHDR_FSHDRAttr_FusionDS1Lb(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionDS1Lb != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.FusionDS1Lb);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionDS1Lb = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionDS1Lb = 255;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionDS1Lb = rand_range(0, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	attr.FusionDS1Lb = 255;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionDS1Lb(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionDS1Lb);
}

static int test_FSHDR_FSHDRAttr_FusionDS2Th(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionDS2Th != 4095) {
		TEST_FAIL("test default value: 4095 != %d\n", (int)attr.FusionDS2Th);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionDS2Th = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionDS2Th = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionDS2Th = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.FusionDS2Th = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.FusionDS2Th = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionDS2Th(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionDS2Th);
}

static int test_FSHDR_FSHDRAttr_FusionDS2Step(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionDS2Step != 16) {
		TEST_FAIL("test default value: 16 != %d\n", (int)attr.FusionDS2Step);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionDS2Step = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionDS2Step = 255;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionDS2Step = rand_range(0, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	attr.FusionDS2Step = 16;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionDS2Step(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionDS2Step);
}

static int test_FSHDR_FSHDRAttr_FusionDS2Lb(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.FusionDS2Lb != 255) {
		TEST_FAIL("test default value: 255 != %d\n", (int)attr.FusionDS2Lb);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.FusionDS2Lb = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.FusionDS2Lb = 255;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.FusionDS2Lb = rand_range(0, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	attr.FusionDS2Lb = 255;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_FusionDS2Lb(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_FusionDS2Lb);
}

static int test_FSHDR_FSHDRAttr_LENormBldRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LENormBldRange != 600) {
		TEST_FAIL("test default value: 600 != %d\n", (int)attr.stManual.LENormBldRange);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LENormBldRange = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LENormBldRange = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LENormBldRange = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.LENormBldRange = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LENormBldRange = 600;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LENormBldRange_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LENormBldRange_manual);
}

static int test_FSHDR_FSHDRAttr_LENormBldRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600 };
	if (memcmp(default_val_0, attr.stAuto.LENormBldRange, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldRange[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldRange[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldRange[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldRange[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600 };
	memcpy(attr.stAuto.LENormBldRange, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LENormBldRange_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LENormBldRange_auto);
}

static int test_FSHDR_FSHDRAttr_LENormBldP0_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LENormBldP0 != 3000) {
		TEST_FAIL("test default value: 3000 != %d\n", (int)attr.stManual.LENormBldP0);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LENormBldP0 = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LENormBldP0 = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LENormBldP0 = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.LENormBldP0 = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LENormBldP0 = 3000;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LENormBldP0_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LENormBldP0_manual);
}

static int test_FSHDR_FSHDRAttr_LENormBldP0_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000 };
	if (memcmp(default_val_0, attr.stAuto.LENormBldP0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldP0[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldP0[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldP0[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LENormBldP0[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000 };
	memcpy(attr.stAuto.LENormBldP0, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LENormBldP0_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LENormBldP0_auto);
}

static int test_FSHDR_FSHDRAttr_SENormBldRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SENormBldRange != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.SENormBldRange);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SENormBldRange = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.SENormBldRange = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.SENormBldRange = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.SENormBldRange = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SENormBldRange = 64;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SENormBldRange_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SENormBldRange_manual);
}

static int test_FSHDR_FSHDRAttr_SENormBldRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.SENormBldRange, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldRange[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldRange[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldRange[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldRange[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.SENormBldRange, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SENormBldRange_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SENormBldRange_auto);
}

static int test_FSHDR_FSHDRAttr_SENormBldP0_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SENormBldP0 != 192) {
		TEST_FAIL("test default value: 192 != %d\n", (int)attr.stManual.SENormBldP0);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SENormBldP0 = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.SENormBldP0 = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.SENormBldP0 = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.SENormBldP0 = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SENormBldP0 = 192;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SENormBldP0_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SENormBldP0_manual);
}

static int test_FSHDR_FSHDRAttr_SENormBldP0_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	if (memcmp(default_val_0, attr.stAuto.SENormBldP0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldP0[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldP0[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldP0[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SENormBldP0[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	memcpy(attr.stAuto.SENormBldP0, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SENormBldP0_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SENormBldP0_auto);
}

static int test_FSHDR_FSHDRAttr_LEDynmBldRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LEDynmBldRange != 1024) {
		TEST_FAIL("test default value: 1024 != %d\n", (int)attr.stManual.LEDynmBldRange);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LEDynmBldRange = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LEDynmBldRange = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LEDynmBldRange = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.LEDynmBldRange = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LEDynmBldRange = 1024;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LEDynmBldRange_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LEDynmBldRange_manual);
}

static int test_FSHDR_FSHDRAttr_LEDynmBldRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024 };
	if (memcmp(default_val_0, attr.stAuto.LEDynmBldRange, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldRange[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldRange[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldRange[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldRange[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024 };
	memcpy(attr.stAuto.LEDynmBldRange, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LEDynmBldRange_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LEDynmBldRange_auto);
}

static int test_FSHDR_FSHDRAttr_LEDynmBldP0_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.LEDynmBldP0 != 2560) {
		TEST_FAIL("test default value: 2560 != %d\n", (int)attr.stManual.LEDynmBldP0);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.LEDynmBldP0 = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.LEDynmBldP0 = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.LEDynmBldP0 = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.LEDynmBldP0 = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.LEDynmBldP0 = 2560;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LEDynmBldP0_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LEDynmBldP0_manual);
}

static int test_FSHDR_FSHDRAttr_LEDynmBldP0_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560 };
	if (memcmp(default_val_0, attr.stAuto.LEDynmBldP0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldP0[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldP0[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldP0[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.LEDynmBldP0[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560 };
	memcpy(attr.stAuto.LEDynmBldP0, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_LEDynmBldP0_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_LEDynmBldP0_auto);
}

static int test_FSHDR_FSHDRAttr_SEDynmBldRange_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SEDynmBldRange != 64) {
		TEST_FAIL("test default value: 64 != %d\n", (int)attr.stManual.SEDynmBldRange);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SEDynmBldRange = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.SEDynmBldRange = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.SEDynmBldRange = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.SEDynmBldRange = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SEDynmBldRange = 64;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SEDynmBldRange_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SEDynmBldRange_manual);
}

static int test_FSHDR_FSHDRAttr_SEDynmBldRange_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	if (memcmp(default_val_0, attr.stAuto.SEDynmBldRange, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldRange[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldRange[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldRange[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldRange[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };
	memcpy(attr.stAuto.SEDynmBldRange, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SEDynmBldRange_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SEDynmBldRange_auto);
}

static int test_FSHDR_FSHDRAttr_SEDynmBldP0_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.SEDynmBldP0 != 192) {
		TEST_FAIL("test default value: 192 != %d\n", (int)attr.stManual.SEDynmBldP0);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.SEDynmBldP0 = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.SEDynmBldP0 = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.SEDynmBldP0 = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.SEDynmBldP0 = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.SEDynmBldP0 = 192;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SEDynmBldP0_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SEDynmBldP0_manual);
}

static int test_FSHDR_FSHDRAttr_SEDynmBldP0_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	if (memcmp(default_val_0, attr.stAuto.SEDynmBldP0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldP0[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldP0[j] = 4095;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldP0[j] = rand_range(0, 4095);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.SEDynmBldP0[j] = rand_range(4096, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	memcpy(attr.stAuto.SEDynmBldP0, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_SEDynmBldP0_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_SEDynmBldP0_auto);
}

static int test_FSHDR_FSHDRAttr_NDBldLumTh(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.NDBldLumTh != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.NDBldLumTh);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.NDBldLumTh = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.NDBldLumTh = 256;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.NDBldLumTh = rand_range(0, 256);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.NDBldLumTh = rand_range(257, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.NDBldLumTh = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_NDBldLumTh(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_NDBldLumTh);
}

static int test_FSHDR_FSHDRAttr_NDBldDarkWt(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.NDBldDarkWt != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.NDBldDarkWt);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.NDBldDarkWt = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.NDBldDarkWt = 16;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.NDBldDarkWt = rand_range(0, 16);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.NDBldDarkWt = rand_range(17, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.NDBldDarkWt = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_NDBldDarkWt(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_NDBldDarkWt);
}

static int test_FSHDR_FSHDRAttr_NDBldDiffLut(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val, attr.NDBldDiffLut, sizeof(CVI_U8) * 16) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 16; ++i) {
		attr.NDBldDiffLut[i] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 16; ++i) {
		attr.NDBldDiffLut[i] = 16;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 16; i++) {
		attr.NDBldDiffLut[i] = rand_range(0, 16);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int i = 0; i < 16; i++) {
		attr.NDBldDiffLut[i] = rand_range(17, 255);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	memcpy(attr.NDBldDiffLut, default_val, sizeof(CVI_U8) * 16);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_NDBldDiffLut(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_NDBldDiffLut);
}

static int test_FSHDR_FSHDRAttr_MCurveMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.MCurveMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveMode = MCURVE_AUTO;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveMode = MCURVE_MODE_BUTT;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	attr.MCurveMode = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveMode(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveMode);
}

static int test_FSHDR_FSHDRAttr_MCurveAutoSEMin(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveAutoSEMin != 3000) {
		TEST_FAIL("test default value: 3000 != %d\n", (int)attr.MCurveAutoSEMin);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveAutoSEMin = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveAutoSEMin = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.MCurveAutoSEMin = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.MCurveAutoSEMin = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.MCurveAutoSEMin = 3000;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveAutoSEMin(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveAutoSEMin);
}

static int test_FSHDR_FSHDRAttr_MCurveManualSEMax(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveManualSEMax != 4095) {
		TEST_FAIL("test default value: 4095 != %d\n", (int)attr.MCurveManualSEMax);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveManualSEMax = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveManualSEMax = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.MCurveManualSEMax = rand_range(0, 4095);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.MCurveManualSEMax = rand_range(4096, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.MCurveManualSEMax = 4095;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveManualSEMax(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveManualSEMax);
}

static int test_FSHDR_FSHDRAttr_MCurveSmooth(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveSmooth != 200) {
		TEST_FAIL("test default value: 200 != %d\n", (int)attr.MCurveSmooth);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveSmooth = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveSmooth = 255;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.MCurveSmooth = rand_range(0, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	attr.MCurveSmooth = 200;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveSmooth(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveSmooth);
}

static int test_FSHDR_FSHDRAttr_MCurveDelta_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.MCurveDelta != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.stManual.MCurveDelta);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.MCurveDelta = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.MCurveDelta = 8560;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.MCurveDelta = rand_range(0, 8560);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.MCurveDelta = rand_range(8561, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.MCurveDelta = 1;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveDelta_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveDelta_manual);
}

static int test_FSHDR_FSHDRAttr_MCurveDelta_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	if (memcmp(default_val_0, attr.stAuto.MCurveDelta, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.MCurveDelta[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.MCurveDelta[j] = 8560;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.MCurveDelta[j] = rand_range(0, 8560);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.MCurveDelta[j] = rand_range(8561, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	memcpy(attr.stAuto.MCurveDelta, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveDelta_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveDelta_auto);
}

static int test_FSHDR_FSHDRAttr_MCurveX1_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.MCurveX1 != 4088) {
		TEST_FAIL("test default value: 4088 != %d\n", (int)attr.stManual.MCurveX1);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.MCurveX1 = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.MCurveX1 = 65535;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.MCurveX1 = rand_range(0, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	attr.stManual.MCurveX1 = 4088;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveX1_manual(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveX1_manual);
}

static int test_FSHDR_FSHDRAttr_MCurveX1_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_LV_NUM] = { 4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088 };
	if (memcmp(default_val_0, attr.stAuto.MCurveX1, sizeof(CVI_U16) * ISP_AUTO_LV_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.MCurveX1[j] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.MCurveX1[j] = 65535;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int j = 0; j < ISP_AUTO_LV_NUM; ++j) {
		attr.stAuto.MCurveX1[j] = rand_range(0, 65535);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	CVI_U16 restore_val_0[ISP_AUTO_LV_NUM] = { 4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088,4088 };
	memcpy(attr.stAuto.MCurveX1, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_LV_NUM);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveX1_auto(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveX1_auto);
}

static int test_FSHDR_FSHDRAttr_MCurveBldRatio(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveBldRatio != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.MCurveBldRatio);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveBldRatio = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveBldRatio = 10;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.MCurveBldRatio = rand_range(0, 10);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.MCurveBldRatio = rand_range(11, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.MCurveBldRatio = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveBldRatio(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveBldRatio);
}

static int test_FSHDR_FSHDRAttr_MCurveXMaxRatio(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveXMaxRatio != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.MCurveXMaxRatio);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveXMaxRatio = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveXMaxRatio = 256;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.MCurveXMaxRatio = rand_range(0, 256);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.MCurveXMaxRatio = rand_range(257, 65535);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.MCurveXMaxRatio = 128;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveXMaxRatio(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveXMaxRatio);
}

static int test_FSHDR_FSHDRAttr_MCurveYsel(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveYsel != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.MCurveYsel);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveYsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveYsel = 2;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.MCurveYsel = rand_range(0, 2);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.MCurveYsel = rand_range(3, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.MCurveYsel = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveYsel(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveYsel);
}

static int test_FSHDR_FSHDRAttr_MCurveYvWet(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.MCurveYvWet != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.MCurveYvWet);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.MCurveYvWet = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.MCurveYvWet = 8;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.MCurveYvWet = rand_range(0, 8);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.MCurveYvWet = rand_range(9, 255);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.MCurveYvWet = 0;
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveYvWet(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveYvWet);
}

static int test_FSHDR_FSHDRAttr_MCurveFlumWet(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_FSHDR_ATTR_S attr;

	ret = CVI_ISP_GetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetFSHDRAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[17] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (memcmp(default_val, attr.MCurveFlumWet, sizeof(CVI_U8) * 17) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 17; ++i) {
		attr.MCurveFlumWet[i] = 0;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 17; ++i) {
		attr.MCurveFlumWet[i] = 255;
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 17; i++) {
		attr.MCurveFlumWet[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
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
	memcpy(attr.MCurveFlumWet, default_val, sizeof(CVI_U8) * 17);
	ret = CVI_ISP_SetFSHDRAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_FSHDR_FSHDRAttr_MCurveFlumWet(void)
{
	reg_api_test_fn(test_FSHDR_FSHDRAttr_MCurveFlumWet);
}
