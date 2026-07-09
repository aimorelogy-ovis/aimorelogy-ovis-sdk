
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

static int test_CA2_CA2Attr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA2_ATTR_S attr;

	ret = CVI_ISP_GetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCA2Attr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA2_CA2Attr_Enable(void)
{
	reg_api_test_fn(test_CA2_CA2Attr_Enable);
}

static int test_CA2_CA2Attr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA2_ATTR_S attr;

	ret = CVI_ISP_GetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCA2Attr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA2_CA2Attr_enOpType(void)
{
	reg_api_test_fn(test_CA2_CA2Attr_enOpType);
}

static int test_CA2_CA2Attr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA2_ATTR_S attr;

	ret = CVI_ISP_GetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCA2Attr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA2_CA2Attr_UpdateInterval(void)
{
	reg_api_test_fn(test_CA2_CA2Attr_UpdateInterval);
}

static int test_CA2_CA2Attr_Ca2In_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA2_ATTR_S attr;

	ret = CVI_ISP_GetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCA2Attr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.Ca2In[0] != 4) {
		TEST_FAIL("test default value: 4 != %d\n", (int)attr.stManual.Ca2In[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2In[1] != 8) {
		TEST_FAIL("test default value: 8 != %d\n", (int)attr.stManual.Ca2In[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2In[2] != 12) {
		TEST_FAIL("test default value: 12 != %d\n", (int)attr.stManual.Ca2In[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2In[3] != 18) {
		TEST_FAIL("test default value: 18 != %d\n", (int)attr.stManual.Ca2In[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2In[4] != 32) {
		TEST_FAIL("test default value: 32 != %d\n", (int)attr.stManual.Ca2In[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2In[5] != 192) {
		TEST_FAIL("test default value: 192 != %d\n", (int)attr.stManual.Ca2In[5]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.Ca2In[0] = 0;
	attr.stManual.Ca2In[1] = 0;
	attr.stManual.Ca2In[2] = 0;
	attr.stManual.Ca2In[3] = 0;
	attr.stManual.Ca2In[4] = 0;
	attr.stManual.Ca2In[5] = 0;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.Ca2In[0] = 192;
	attr.stManual.Ca2In[1] = 192;
	attr.stManual.Ca2In[2] = 192;
	attr.stManual.Ca2In[3] = 192;
	attr.stManual.Ca2In[4] = 192;
	attr.stManual.Ca2In[5] = 192;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.Ca2In[0] = rand_range(0, 192);
	attr.stManual.Ca2In[1] = rand_range(0, 192);
	attr.stManual.Ca2In[2] = rand_range(0, 192);
	attr.stManual.Ca2In[3] = rand_range(0, 192);
	attr.stManual.Ca2In[4] = rand_range(0, 192);
	attr.stManual.Ca2In[5] = rand_range(0, 192);
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.Ca2In[0] = rand_range(193, 255);
	attr.stManual.Ca2In[1] = rand_range(193, 255);
	attr.stManual.Ca2In[2] = rand_range(193, 255);
	attr.stManual.Ca2In[3] = rand_range(193, 255);
	attr.stManual.Ca2In[4] = rand_range(193, 255);
	attr.stManual.Ca2In[5] = rand_range(193, 255);
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.Ca2In[0] = 4;
	attr.stManual.Ca2In[1] = 8;
	attr.stManual.Ca2In[2] = 12;
	attr.stManual.Ca2In[3] = 18;
	attr.stManual.Ca2In[4] = 32;
	attr.stManual.Ca2In[5] = 192;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA2_CA2Attr_Ca2In_manual(void)
{
	reg_api_test_fn(test_CA2_CA2Attr_Ca2In_manual);
}

static int test_CA2_CA2Attr_Ca2In_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA2_ATTR_S attr;

	ret = CVI_ISP_GetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCA2Attr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	if (memcmp(default_val_0, attr.stAuto.Ca2In[0], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	if (memcmp(default_val_1, attr.stAuto.Ca2In[1], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	if (memcmp(default_val_2, attr.stAuto.Ca2In[2], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18 };
	if (memcmp(default_val_3, attr.stAuto.Ca2In[3], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	if (memcmp(default_val_4, attr.stAuto.Ca2In[4], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U8 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	if (memcmp(default_val_5, attr.stAuto.Ca2In[5], sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[5][j] = 0;
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.Ca2In[0][j] = 192;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[1][j] = 192;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[2][j] = 192;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[3][j] = 192;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[4][j] = 192;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[5][j] = 192;
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.Ca2In[0][j] = rand_range(0, 192);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[1][j] = rand_range(0, 192);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[2][j] = rand_range(0, 192);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[3][j] = rand_range(0, 192);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[4][j] = rand_range(0, 192);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[5][j] = rand_range(0, 192);
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.Ca2In[0][j] = rand_range(193, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[1][j] = rand_range(193, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[2][j] = rand_range(193, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[3][j] = rand_range(193, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[4][j] = rand_range(193, 255);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2In[5][j] = rand_range(193, 255);
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U8 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4 };
	memcpy(attr.stAuto.Ca2In[0], restore_val_0, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };
	memcpy(attr.stAuto.Ca2In[1], restore_val_1, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	memcpy(attr.stAuto.Ca2In[2], restore_val_2, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18 };
	memcpy(attr.stAuto.Ca2In[3], restore_val_3, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
	memcpy(attr.stAuto.Ca2In[4], restore_val_4, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U8 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192 };
	memcpy(attr.stAuto.Ca2In[5], restore_val_5, sizeof(CVI_U8) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA2_CA2Attr_Ca2In_auto(void)
{
	reg_api_test_fn(test_CA2_CA2Attr_Ca2In_auto);
}

static int test_CA2_CA2Attr_Ca2Out_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA2_ATTR_S attr;

	ret = CVI_ISP_GetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCA2Attr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.Ca2Out[0] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.Ca2Out[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2Out[1] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.Ca2Out[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2Out[2] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.Ca2Out[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2Out[3] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.Ca2Out[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2Out[4] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.Ca2Out[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.Ca2Out[5] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.Ca2Out[5]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.Ca2Out[0] = 0;
	attr.stManual.Ca2Out[1] = 0;
	attr.stManual.Ca2Out[2] = 0;
	attr.stManual.Ca2Out[3] = 0;
	attr.stManual.Ca2Out[4] = 0;
	attr.stManual.Ca2Out[5] = 0;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.Ca2Out[0] = 2047;
	attr.stManual.Ca2Out[1] = 2047;
	attr.stManual.Ca2Out[2] = 2047;
	attr.stManual.Ca2Out[3] = 2047;
	attr.stManual.Ca2Out[4] = 2047;
	attr.stManual.Ca2Out[5] = 2047;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.Ca2Out[0] = rand_range(0, 2047);
	attr.stManual.Ca2Out[1] = rand_range(0, 2047);
	attr.stManual.Ca2Out[2] = rand_range(0, 2047);
	attr.stManual.Ca2Out[3] = rand_range(0, 2047);
	attr.stManual.Ca2Out[4] = rand_range(0, 2047);
	attr.stManual.Ca2Out[5] = rand_range(0, 2047);
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.Ca2Out[0] = rand_range(2048, 65535);
	attr.stManual.Ca2Out[1] = rand_range(2048, 65535);
	attr.stManual.Ca2Out[2] = rand_range(2048, 65535);
	attr.stManual.Ca2Out[3] = rand_range(2048, 65535);
	attr.stManual.Ca2Out[4] = rand_range(2048, 65535);
	attr.stManual.Ca2Out[5] = rand_range(2048, 65535);
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.Ca2Out[0] = 128;
	attr.stManual.Ca2Out[1] = 128;
	attr.stManual.Ca2Out[2] = 128;
	attr.stManual.Ca2Out[3] = 128;
	attr.stManual.Ca2Out[4] = 128;
	attr.stManual.Ca2Out[5] = 128;
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA2_CA2Attr_Ca2Out_manual(void)
{
	reg_api_test_fn(test_CA2_CA2Attr_Ca2Out_manual);
}

static int test_CA2_CA2Attr_Ca2Out_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA2_ATTR_S attr;

	ret = CVI_ISP_GetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCA2Attr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.Ca2Out[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_1, attr.stAuto.Ca2Out[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_2, attr.stAuto.Ca2Out[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_3, attr.stAuto.Ca2Out[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_4, attr.stAuto.Ca2Out[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_5, attr.stAuto.Ca2Out[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[5][j] = 0;
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.Ca2Out[0][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[1][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[2][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[3][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[4][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[5][j] = 2047;
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.Ca2Out[0][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[1][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[2][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[3][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[4][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[5][j] = rand_range(0, 2047);
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.Ca2Out[0][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[1][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[2][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[3][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[4][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.Ca2Out[5][j] = rand_range(2048, 65535);
	}
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.Ca2Out[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.Ca2Out[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.Ca2Out[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.Ca2Out[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.Ca2Out[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.Ca2Out[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCA2Attr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA2_CA2Attr_Ca2Out_auto(void)
{
	reg_api_test_fn(test_CA2_CA2Attr_Ca2Out_auto);
}
