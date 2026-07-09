
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

static int test_CA_CAAttr_Enable(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.Enable != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.Enable);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.Enable = 0;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_Enable(void)
{
	reg_api_test_fn(test_CA_CAAttr_Enable);
}

static int test_CA_CAAttr_enOpType(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.enOpType != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.enOpType);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.enOpType = OP_TYPE_AUTO;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_enOpType(void)
{
	reg_api_test_fn(test_CA_CAAttr_enOpType);
}

static int test_CA_CAAttr_UpdateInterval(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.UpdateInterval != 1) {
		TEST_FAIL("test default value: 1 != %d\n", (int)attr.UpdateInterval);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.UpdateInterval = 1;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_UpdateInterval(void)
{
	reg_api_test_fn(test_CA_CAAttr_UpdateInterval);
}

static int test_CA_CAAttr_CaCpMode(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.CaCpMode != 0) {
		TEST_FAIL("test default value: 0 != %d\n", (int)attr.CaCpMode);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.CaCpMode = 0;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.CaCpMode = 1;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.CaCpMode = rand_range(0, 1);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
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
	attr.CaCpMode = 0;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_CaCpMode(void)
{
	reg_api_test_fn(test_CA_CAAttr_CaCpMode);
}

static int test_CA_CAAttr_CPLutY(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[256] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 };
	if (memcmp(default_val, attr.CPLutY, sizeof(CVI_U8) * 256) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 256; ++i) {
		attr.CPLutY[i] = 0;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 256; ++i) {
		attr.CPLutY[i] = 255;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 256; i++) {
		attr.CPLutY[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
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
	memcpy(attr.CPLutY, default_val, sizeof(CVI_U8) * 256);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_CPLutY(void)
{
	reg_api_test_fn(test_CA_CAAttr_CPLutY);
}

static int test_CA_CAAttr_CPLutU(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[256] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 };
	if (memcmp(default_val, attr.CPLutU, sizeof(CVI_U8) * 256) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 256; ++i) {
		attr.CPLutU[i] = 0;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 256; ++i) {
		attr.CPLutU[i] = 255;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 256; i++) {
		attr.CPLutU[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
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
	memcpy(attr.CPLutU, default_val, sizeof(CVI_U8) * 256);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_CPLutU(void)
{
	reg_api_test_fn(test_CA_CAAttr_CPLutU);
}

static int test_CA_CAAttr_CPLutV(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U8 default_val[256] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 };
	if (memcmp(default_val, attr.CPLutV, sizeof(CVI_U8) * 256) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int i = 0; i < 256; ++i) {
		attr.CPLutV[i] = 0;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	for (int i = 0; i < 256; ++i) {
		attr.CPLutV[i] = 255;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	for (int i = 0; i < 256; i++) {
		attr.CPLutV[i] = rand_range(0, 255);
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
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
	memcpy(attr.CPLutV, default_val, sizeof(CVI_U8) * 256);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_CPLutV(void)
{
	reg_api_test_fn(test_CA_CAAttr_CPLutV);
}

static int test_CA_CAAttr_ISORatio_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.ISORatio != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.ISORatio);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.ISORatio = 0;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.ISORatio = 2047;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.ISORatio = rand_range(0, 2047);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.ISORatio = rand_range(2048, 65535);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.ISORatio = 128;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_ISORatio_manual(void)
{
	reg_api_test_fn(test_CA_CAAttr_ISORatio_manual);
}

static int test_CA_CAAttr_ISORatio_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.ISORatio, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.ISORatio[j] = 0;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.ISORatio[j] = 2047;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.ISORatio[j] = rand_range(0, 2047);
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.ISORatio[j] = rand_range(2048, 65535);
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.ISORatio, restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_ISORatio_auto(void)
{
	reg_api_test_fn(test_CA_CAAttr_ISORatio_auto);
}

static int test_CA_CAAttr_YRatioLut_manual(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	if (attr.stManual.YRatioLut[0] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[0]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[1] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[1]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[2] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[2]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[3] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[3]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[4] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[4]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[5] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[5]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[6] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[6]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[7] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[7]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[8] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[8]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[9] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[9]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[10] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[10]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[11] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[11]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[12] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[12]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[13] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[13]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[14] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[14]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[15] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[15]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[16] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[16]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[17] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[17]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[18] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[18]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[19] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[19]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[20] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[20]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[21] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[21]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[22] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[22]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[23] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[23]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[24] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[24]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[25] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[25]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[26] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[26]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[27] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[27]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[28] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[28]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[29] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[29]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[30] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[30]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[31] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[31]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[32] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[32]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[33] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[33]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[34] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[34]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[35] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[35]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[36] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[36]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[37] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[37]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[38] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[38]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[39] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[39]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[40] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[40]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[41] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[41]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[42] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[42]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[43] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[43]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[44] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[44]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[45] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[45]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[46] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[46]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[47] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[47]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[48] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[48]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[49] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[49]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[50] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[50]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[51] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[51]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[52] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[52]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[53] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[53]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[54] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[54]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[55] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[55]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[56] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[56]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[57] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[57]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[58] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[58]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[59] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[59]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[60] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[60]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[61] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[61]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[62] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[62]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[63] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[63]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[64] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[64]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[65] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[65]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[66] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[66]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[67] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[67]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[68] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[68]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[69] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[69]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[70] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[70]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[71] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[71]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[72] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[72]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[73] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[73]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[74] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[74]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[75] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[75]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[76] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[76]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[77] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[77]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[78] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[78]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[79] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[79]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[80] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[80]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[81] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[81]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[82] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[82]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[83] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[83]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[84] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[84]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[85] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[85]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[86] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[86]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[87] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[87]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[88] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[88]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[89] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[89]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[90] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[90]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[91] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[91]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[92] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[92]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[93] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[93]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[94] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[94]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[95] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[95]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[96] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[96]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[97] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[97]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[98] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[98]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[99] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[99]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[100] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[100]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[101] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[101]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[102] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[102]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[103] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[103]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[104] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[104]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[105] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[105]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[106] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[106]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[107] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[107]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[108] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[108]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[109] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[109]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[110] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[110]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[111] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[111]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[112] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[112]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[113] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[113]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[114] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[114]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[115] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[115]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[116] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[116]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[117] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[117]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[118] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[118]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[119] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[119]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[120] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[120]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[121] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[121]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[122] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[122]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[123] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[123]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[124] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[124]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[125] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[125]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[126] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[126]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[127] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[127]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[128] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[128]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[129] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[129]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[130] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[130]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[131] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[131]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[132] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[132]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[133] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[133]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[134] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[134]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[135] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[135]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[136] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[136]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[137] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[137]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[138] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[138]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[139] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[139]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[140] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[140]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[141] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[141]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[142] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[142]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[143] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[143]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[144] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[144]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[145] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[145]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[146] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[146]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[147] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[147]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[148] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[148]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[149] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[149]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[150] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[150]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[151] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[151]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[152] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[152]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[153] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[153]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[154] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[154]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[155] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[155]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[156] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[156]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[157] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[157]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[158] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[158]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[159] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[159]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[160] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[160]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[161] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[161]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[162] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[162]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[163] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[163]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[164] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[164]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[165] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[165]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[166] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[166]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[167] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[167]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[168] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[168]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[169] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[169]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[170] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[170]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[171] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[171]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[172] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[172]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[173] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[173]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[174] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[174]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[175] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[175]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[176] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[176]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[177] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[177]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[178] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[178]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[179] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[179]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[180] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[180]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[181] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[181]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[182] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[182]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[183] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[183]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[184] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[184]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[185] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[185]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[186] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[186]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[187] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[187]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[188] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[188]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[189] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[189]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[190] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[190]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[191] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[191]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[192] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[192]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[193] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[193]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[194] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[194]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[195] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[195]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[196] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[196]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[197] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[197]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[198] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[198]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[199] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[199]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[200] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[200]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[201] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[201]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[202] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[202]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[203] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[203]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[204] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[204]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[205] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[205]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[206] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[206]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[207] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[207]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[208] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[208]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[209] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[209]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[210] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[210]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[211] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[211]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[212] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[212]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[213] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[213]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[214] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[214]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[215] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[215]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[216] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[216]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[217] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[217]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[218] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[218]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[219] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[219]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[220] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[220]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[221] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[221]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[222] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[222]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[223] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[223]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[224] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[224]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[225] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[225]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[226] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[226]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[227] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[227]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[228] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[228]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[229] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[229]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[230] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[230]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[231] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[231]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[232] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[232]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[233] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[233]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[234] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[234]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[235] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[235]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[236] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[236]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[237] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[237]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[238] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[238]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[239] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[239]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[240] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[240]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[241] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[241]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[242] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[242]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[243] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[243]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[244] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[244]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[245] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[245]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[246] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[246]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[247] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[247]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[248] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[248]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[249] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[249]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[250] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[250]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[251] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[251]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[252] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[252]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[253] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[253]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[254] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[254]);
		return CVI_FAILURE;
	}
	if (attr.stManual.YRatioLut[255] != 128) {
		TEST_FAIL("test default value: 128 != %d\n", (int)attr.stManual.YRatioLut[255]);
		return CVI_FAILURE;
	}
	// set minimum value
	attr.stManual.YRatioLut[0] = 0;
	attr.stManual.YRatioLut[1] = 0;
	attr.stManual.YRatioLut[2] = 0;
	attr.stManual.YRatioLut[3] = 0;
	attr.stManual.YRatioLut[4] = 0;
	attr.stManual.YRatioLut[5] = 0;
	attr.stManual.YRatioLut[6] = 0;
	attr.stManual.YRatioLut[7] = 0;
	attr.stManual.YRatioLut[8] = 0;
	attr.stManual.YRatioLut[9] = 0;
	attr.stManual.YRatioLut[10] = 0;
	attr.stManual.YRatioLut[11] = 0;
	attr.stManual.YRatioLut[12] = 0;
	attr.stManual.YRatioLut[13] = 0;
	attr.stManual.YRatioLut[14] = 0;
	attr.stManual.YRatioLut[15] = 0;
	attr.stManual.YRatioLut[16] = 0;
	attr.stManual.YRatioLut[17] = 0;
	attr.stManual.YRatioLut[18] = 0;
	attr.stManual.YRatioLut[19] = 0;
	attr.stManual.YRatioLut[20] = 0;
	attr.stManual.YRatioLut[21] = 0;
	attr.stManual.YRatioLut[22] = 0;
	attr.stManual.YRatioLut[23] = 0;
	attr.stManual.YRatioLut[24] = 0;
	attr.stManual.YRatioLut[25] = 0;
	attr.stManual.YRatioLut[26] = 0;
	attr.stManual.YRatioLut[27] = 0;
	attr.stManual.YRatioLut[28] = 0;
	attr.stManual.YRatioLut[29] = 0;
	attr.stManual.YRatioLut[30] = 0;
	attr.stManual.YRatioLut[31] = 0;
	attr.stManual.YRatioLut[32] = 0;
	attr.stManual.YRatioLut[33] = 0;
	attr.stManual.YRatioLut[34] = 0;
	attr.stManual.YRatioLut[35] = 0;
	attr.stManual.YRatioLut[36] = 0;
	attr.stManual.YRatioLut[37] = 0;
	attr.stManual.YRatioLut[38] = 0;
	attr.stManual.YRatioLut[39] = 0;
	attr.stManual.YRatioLut[40] = 0;
	attr.stManual.YRatioLut[41] = 0;
	attr.stManual.YRatioLut[42] = 0;
	attr.stManual.YRatioLut[43] = 0;
	attr.stManual.YRatioLut[44] = 0;
	attr.stManual.YRatioLut[45] = 0;
	attr.stManual.YRatioLut[46] = 0;
	attr.stManual.YRatioLut[47] = 0;
	attr.stManual.YRatioLut[48] = 0;
	attr.stManual.YRatioLut[49] = 0;
	attr.stManual.YRatioLut[50] = 0;
	attr.stManual.YRatioLut[51] = 0;
	attr.stManual.YRatioLut[52] = 0;
	attr.stManual.YRatioLut[53] = 0;
	attr.stManual.YRatioLut[54] = 0;
	attr.stManual.YRatioLut[55] = 0;
	attr.stManual.YRatioLut[56] = 0;
	attr.stManual.YRatioLut[57] = 0;
	attr.stManual.YRatioLut[58] = 0;
	attr.stManual.YRatioLut[59] = 0;
	attr.stManual.YRatioLut[60] = 0;
	attr.stManual.YRatioLut[61] = 0;
	attr.stManual.YRatioLut[62] = 0;
	attr.stManual.YRatioLut[63] = 0;
	attr.stManual.YRatioLut[64] = 0;
	attr.stManual.YRatioLut[65] = 0;
	attr.stManual.YRatioLut[66] = 0;
	attr.stManual.YRatioLut[67] = 0;
	attr.stManual.YRatioLut[68] = 0;
	attr.stManual.YRatioLut[69] = 0;
	attr.stManual.YRatioLut[70] = 0;
	attr.stManual.YRatioLut[71] = 0;
	attr.stManual.YRatioLut[72] = 0;
	attr.stManual.YRatioLut[73] = 0;
	attr.stManual.YRatioLut[74] = 0;
	attr.stManual.YRatioLut[75] = 0;
	attr.stManual.YRatioLut[76] = 0;
	attr.stManual.YRatioLut[77] = 0;
	attr.stManual.YRatioLut[78] = 0;
	attr.stManual.YRatioLut[79] = 0;
	attr.stManual.YRatioLut[80] = 0;
	attr.stManual.YRatioLut[81] = 0;
	attr.stManual.YRatioLut[82] = 0;
	attr.stManual.YRatioLut[83] = 0;
	attr.stManual.YRatioLut[84] = 0;
	attr.stManual.YRatioLut[85] = 0;
	attr.stManual.YRatioLut[86] = 0;
	attr.stManual.YRatioLut[87] = 0;
	attr.stManual.YRatioLut[88] = 0;
	attr.stManual.YRatioLut[89] = 0;
	attr.stManual.YRatioLut[90] = 0;
	attr.stManual.YRatioLut[91] = 0;
	attr.stManual.YRatioLut[92] = 0;
	attr.stManual.YRatioLut[93] = 0;
	attr.stManual.YRatioLut[94] = 0;
	attr.stManual.YRatioLut[95] = 0;
	attr.stManual.YRatioLut[96] = 0;
	attr.stManual.YRatioLut[97] = 0;
	attr.stManual.YRatioLut[98] = 0;
	attr.stManual.YRatioLut[99] = 0;
	attr.stManual.YRatioLut[100] = 0;
	attr.stManual.YRatioLut[101] = 0;
	attr.stManual.YRatioLut[102] = 0;
	attr.stManual.YRatioLut[103] = 0;
	attr.stManual.YRatioLut[104] = 0;
	attr.stManual.YRatioLut[105] = 0;
	attr.stManual.YRatioLut[106] = 0;
	attr.stManual.YRatioLut[107] = 0;
	attr.stManual.YRatioLut[108] = 0;
	attr.stManual.YRatioLut[109] = 0;
	attr.stManual.YRatioLut[110] = 0;
	attr.stManual.YRatioLut[111] = 0;
	attr.stManual.YRatioLut[112] = 0;
	attr.stManual.YRatioLut[113] = 0;
	attr.stManual.YRatioLut[114] = 0;
	attr.stManual.YRatioLut[115] = 0;
	attr.stManual.YRatioLut[116] = 0;
	attr.stManual.YRatioLut[117] = 0;
	attr.stManual.YRatioLut[118] = 0;
	attr.stManual.YRatioLut[119] = 0;
	attr.stManual.YRatioLut[120] = 0;
	attr.stManual.YRatioLut[121] = 0;
	attr.stManual.YRatioLut[122] = 0;
	attr.stManual.YRatioLut[123] = 0;
	attr.stManual.YRatioLut[124] = 0;
	attr.stManual.YRatioLut[125] = 0;
	attr.stManual.YRatioLut[126] = 0;
	attr.stManual.YRatioLut[127] = 0;
	attr.stManual.YRatioLut[128] = 0;
	attr.stManual.YRatioLut[129] = 0;
	attr.stManual.YRatioLut[130] = 0;
	attr.stManual.YRatioLut[131] = 0;
	attr.stManual.YRatioLut[132] = 0;
	attr.stManual.YRatioLut[133] = 0;
	attr.stManual.YRatioLut[134] = 0;
	attr.stManual.YRatioLut[135] = 0;
	attr.stManual.YRatioLut[136] = 0;
	attr.stManual.YRatioLut[137] = 0;
	attr.stManual.YRatioLut[138] = 0;
	attr.stManual.YRatioLut[139] = 0;
	attr.stManual.YRatioLut[140] = 0;
	attr.stManual.YRatioLut[141] = 0;
	attr.stManual.YRatioLut[142] = 0;
	attr.stManual.YRatioLut[143] = 0;
	attr.stManual.YRatioLut[144] = 0;
	attr.stManual.YRatioLut[145] = 0;
	attr.stManual.YRatioLut[146] = 0;
	attr.stManual.YRatioLut[147] = 0;
	attr.stManual.YRatioLut[148] = 0;
	attr.stManual.YRatioLut[149] = 0;
	attr.stManual.YRatioLut[150] = 0;
	attr.stManual.YRatioLut[151] = 0;
	attr.stManual.YRatioLut[152] = 0;
	attr.stManual.YRatioLut[153] = 0;
	attr.stManual.YRatioLut[154] = 0;
	attr.stManual.YRatioLut[155] = 0;
	attr.stManual.YRatioLut[156] = 0;
	attr.stManual.YRatioLut[157] = 0;
	attr.stManual.YRatioLut[158] = 0;
	attr.stManual.YRatioLut[159] = 0;
	attr.stManual.YRatioLut[160] = 0;
	attr.stManual.YRatioLut[161] = 0;
	attr.stManual.YRatioLut[162] = 0;
	attr.stManual.YRatioLut[163] = 0;
	attr.stManual.YRatioLut[164] = 0;
	attr.stManual.YRatioLut[165] = 0;
	attr.stManual.YRatioLut[166] = 0;
	attr.stManual.YRatioLut[167] = 0;
	attr.stManual.YRatioLut[168] = 0;
	attr.stManual.YRatioLut[169] = 0;
	attr.stManual.YRatioLut[170] = 0;
	attr.stManual.YRatioLut[171] = 0;
	attr.stManual.YRatioLut[172] = 0;
	attr.stManual.YRatioLut[173] = 0;
	attr.stManual.YRatioLut[174] = 0;
	attr.stManual.YRatioLut[175] = 0;
	attr.stManual.YRatioLut[176] = 0;
	attr.stManual.YRatioLut[177] = 0;
	attr.stManual.YRatioLut[178] = 0;
	attr.stManual.YRatioLut[179] = 0;
	attr.stManual.YRatioLut[180] = 0;
	attr.stManual.YRatioLut[181] = 0;
	attr.stManual.YRatioLut[182] = 0;
	attr.stManual.YRatioLut[183] = 0;
	attr.stManual.YRatioLut[184] = 0;
	attr.stManual.YRatioLut[185] = 0;
	attr.stManual.YRatioLut[186] = 0;
	attr.stManual.YRatioLut[187] = 0;
	attr.stManual.YRatioLut[188] = 0;
	attr.stManual.YRatioLut[189] = 0;
	attr.stManual.YRatioLut[190] = 0;
	attr.stManual.YRatioLut[191] = 0;
	attr.stManual.YRatioLut[192] = 0;
	attr.stManual.YRatioLut[193] = 0;
	attr.stManual.YRatioLut[194] = 0;
	attr.stManual.YRatioLut[195] = 0;
	attr.stManual.YRatioLut[196] = 0;
	attr.stManual.YRatioLut[197] = 0;
	attr.stManual.YRatioLut[198] = 0;
	attr.stManual.YRatioLut[199] = 0;
	attr.stManual.YRatioLut[200] = 0;
	attr.stManual.YRatioLut[201] = 0;
	attr.stManual.YRatioLut[202] = 0;
	attr.stManual.YRatioLut[203] = 0;
	attr.stManual.YRatioLut[204] = 0;
	attr.stManual.YRatioLut[205] = 0;
	attr.stManual.YRatioLut[206] = 0;
	attr.stManual.YRatioLut[207] = 0;
	attr.stManual.YRatioLut[208] = 0;
	attr.stManual.YRatioLut[209] = 0;
	attr.stManual.YRatioLut[210] = 0;
	attr.stManual.YRatioLut[211] = 0;
	attr.stManual.YRatioLut[212] = 0;
	attr.stManual.YRatioLut[213] = 0;
	attr.stManual.YRatioLut[214] = 0;
	attr.stManual.YRatioLut[215] = 0;
	attr.stManual.YRatioLut[216] = 0;
	attr.stManual.YRatioLut[217] = 0;
	attr.stManual.YRatioLut[218] = 0;
	attr.stManual.YRatioLut[219] = 0;
	attr.stManual.YRatioLut[220] = 0;
	attr.stManual.YRatioLut[221] = 0;
	attr.stManual.YRatioLut[222] = 0;
	attr.stManual.YRatioLut[223] = 0;
	attr.stManual.YRatioLut[224] = 0;
	attr.stManual.YRatioLut[225] = 0;
	attr.stManual.YRatioLut[226] = 0;
	attr.stManual.YRatioLut[227] = 0;
	attr.stManual.YRatioLut[228] = 0;
	attr.stManual.YRatioLut[229] = 0;
	attr.stManual.YRatioLut[230] = 0;
	attr.stManual.YRatioLut[231] = 0;
	attr.stManual.YRatioLut[232] = 0;
	attr.stManual.YRatioLut[233] = 0;
	attr.stManual.YRatioLut[234] = 0;
	attr.stManual.YRatioLut[235] = 0;
	attr.stManual.YRatioLut[236] = 0;
	attr.stManual.YRatioLut[237] = 0;
	attr.stManual.YRatioLut[238] = 0;
	attr.stManual.YRatioLut[239] = 0;
	attr.stManual.YRatioLut[240] = 0;
	attr.stManual.YRatioLut[241] = 0;
	attr.stManual.YRatioLut[242] = 0;
	attr.stManual.YRatioLut[243] = 0;
	attr.stManual.YRatioLut[244] = 0;
	attr.stManual.YRatioLut[245] = 0;
	attr.stManual.YRatioLut[246] = 0;
	attr.stManual.YRatioLut[247] = 0;
	attr.stManual.YRatioLut[248] = 0;
	attr.stManual.YRatioLut[249] = 0;
	attr.stManual.YRatioLut[250] = 0;
	attr.stManual.YRatioLut[251] = 0;
	attr.stManual.YRatioLut[252] = 0;
	attr.stManual.YRatioLut[253] = 0;
	attr.stManual.YRatioLut[254] = 0;
	attr.stManual.YRatioLut[255] = 0;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test minimum value failed at VD wait\n");
		return ret;
	}
	// set maximum value
	attr.stManual.YRatioLut[0] = 2047;
	attr.stManual.YRatioLut[1] = 2047;
	attr.stManual.YRatioLut[2] = 2047;
	attr.stManual.YRatioLut[3] = 2047;
	attr.stManual.YRatioLut[4] = 2047;
	attr.stManual.YRatioLut[5] = 2047;
	attr.stManual.YRatioLut[6] = 2047;
	attr.stManual.YRatioLut[7] = 2047;
	attr.stManual.YRatioLut[8] = 2047;
	attr.stManual.YRatioLut[9] = 2047;
	attr.stManual.YRatioLut[10] = 2047;
	attr.stManual.YRatioLut[11] = 2047;
	attr.stManual.YRatioLut[12] = 2047;
	attr.stManual.YRatioLut[13] = 2047;
	attr.stManual.YRatioLut[14] = 2047;
	attr.stManual.YRatioLut[15] = 2047;
	attr.stManual.YRatioLut[16] = 2047;
	attr.stManual.YRatioLut[17] = 2047;
	attr.stManual.YRatioLut[18] = 2047;
	attr.stManual.YRatioLut[19] = 2047;
	attr.stManual.YRatioLut[20] = 2047;
	attr.stManual.YRatioLut[21] = 2047;
	attr.stManual.YRatioLut[22] = 2047;
	attr.stManual.YRatioLut[23] = 2047;
	attr.stManual.YRatioLut[24] = 2047;
	attr.stManual.YRatioLut[25] = 2047;
	attr.stManual.YRatioLut[26] = 2047;
	attr.stManual.YRatioLut[27] = 2047;
	attr.stManual.YRatioLut[28] = 2047;
	attr.stManual.YRatioLut[29] = 2047;
	attr.stManual.YRatioLut[30] = 2047;
	attr.stManual.YRatioLut[31] = 2047;
	attr.stManual.YRatioLut[32] = 2047;
	attr.stManual.YRatioLut[33] = 2047;
	attr.stManual.YRatioLut[34] = 2047;
	attr.stManual.YRatioLut[35] = 2047;
	attr.stManual.YRatioLut[36] = 2047;
	attr.stManual.YRatioLut[37] = 2047;
	attr.stManual.YRatioLut[38] = 2047;
	attr.stManual.YRatioLut[39] = 2047;
	attr.stManual.YRatioLut[40] = 2047;
	attr.stManual.YRatioLut[41] = 2047;
	attr.stManual.YRatioLut[42] = 2047;
	attr.stManual.YRatioLut[43] = 2047;
	attr.stManual.YRatioLut[44] = 2047;
	attr.stManual.YRatioLut[45] = 2047;
	attr.stManual.YRatioLut[46] = 2047;
	attr.stManual.YRatioLut[47] = 2047;
	attr.stManual.YRatioLut[48] = 2047;
	attr.stManual.YRatioLut[49] = 2047;
	attr.stManual.YRatioLut[50] = 2047;
	attr.stManual.YRatioLut[51] = 2047;
	attr.stManual.YRatioLut[52] = 2047;
	attr.stManual.YRatioLut[53] = 2047;
	attr.stManual.YRatioLut[54] = 2047;
	attr.stManual.YRatioLut[55] = 2047;
	attr.stManual.YRatioLut[56] = 2047;
	attr.stManual.YRatioLut[57] = 2047;
	attr.stManual.YRatioLut[58] = 2047;
	attr.stManual.YRatioLut[59] = 2047;
	attr.stManual.YRatioLut[60] = 2047;
	attr.stManual.YRatioLut[61] = 2047;
	attr.stManual.YRatioLut[62] = 2047;
	attr.stManual.YRatioLut[63] = 2047;
	attr.stManual.YRatioLut[64] = 2047;
	attr.stManual.YRatioLut[65] = 2047;
	attr.stManual.YRatioLut[66] = 2047;
	attr.stManual.YRatioLut[67] = 2047;
	attr.stManual.YRatioLut[68] = 2047;
	attr.stManual.YRatioLut[69] = 2047;
	attr.stManual.YRatioLut[70] = 2047;
	attr.stManual.YRatioLut[71] = 2047;
	attr.stManual.YRatioLut[72] = 2047;
	attr.stManual.YRatioLut[73] = 2047;
	attr.stManual.YRatioLut[74] = 2047;
	attr.stManual.YRatioLut[75] = 2047;
	attr.stManual.YRatioLut[76] = 2047;
	attr.stManual.YRatioLut[77] = 2047;
	attr.stManual.YRatioLut[78] = 2047;
	attr.stManual.YRatioLut[79] = 2047;
	attr.stManual.YRatioLut[80] = 2047;
	attr.stManual.YRatioLut[81] = 2047;
	attr.stManual.YRatioLut[82] = 2047;
	attr.stManual.YRatioLut[83] = 2047;
	attr.stManual.YRatioLut[84] = 2047;
	attr.stManual.YRatioLut[85] = 2047;
	attr.stManual.YRatioLut[86] = 2047;
	attr.stManual.YRatioLut[87] = 2047;
	attr.stManual.YRatioLut[88] = 2047;
	attr.stManual.YRatioLut[89] = 2047;
	attr.stManual.YRatioLut[90] = 2047;
	attr.stManual.YRatioLut[91] = 2047;
	attr.stManual.YRatioLut[92] = 2047;
	attr.stManual.YRatioLut[93] = 2047;
	attr.stManual.YRatioLut[94] = 2047;
	attr.stManual.YRatioLut[95] = 2047;
	attr.stManual.YRatioLut[96] = 2047;
	attr.stManual.YRatioLut[97] = 2047;
	attr.stManual.YRatioLut[98] = 2047;
	attr.stManual.YRatioLut[99] = 2047;
	attr.stManual.YRatioLut[100] = 2047;
	attr.stManual.YRatioLut[101] = 2047;
	attr.stManual.YRatioLut[102] = 2047;
	attr.stManual.YRatioLut[103] = 2047;
	attr.stManual.YRatioLut[104] = 2047;
	attr.stManual.YRatioLut[105] = 2047;
	attr.stManual.YRatioLut[106] = 2047;
	attr.stManual.YRatioLut[107] = 2047;
	attr.stManual.YRatioLut[108] = 2047;
	attr.stManual.YRatioLut[109] = 2047;
	attr.stManual.YRatioLut[110] = 2047;
	attr.stManual.YRatioLut[111] = 2047;
	attr.stManual.YRatioLut[112] = 2047;
	attr.stManual.YRatioLut[113] = 2047;
	attr.stManual.YRatioLut[114] = 2047;
	attr.stManual.YRatioLut[115] = 2047;
	attr.stManual.YRatioLut[116] = 2047;
	attr.stManual.YRatioLut[117] = 2047;
	attr.stManual.YRatioLut[118] = 2047;
	attr.stManual.YRatioLut[119] = 2047;
	attr.stManual.YRatioLut[120] = 2047;
	attr.stManual.YRatioLut[121] = 2047;
	attr.stManual.YRatioLut[122] = 2047;
	attr.stManual.YRatioLut[123] = 2047;
	attr.stManual.YRatioLut[124] = 2047;
	attr.stManual.YRatioLut[125] = 2047;
	attr.stManual.YRatioLut[126] = 2047;
	attr.stManual.YRatioLut[127] = 2047;
	attr.stManual.YRatioLut[128] = 2047;
	attr.stManual.YRatioLut[129] = 2047;
	attr.stManual.YRatioLut[130] = 2047;
	attr.stManual.YRatioLut[131] = 2047;
	attr.stManual.YRatioLut[132] = 2047;
	attr.stManual.YRatioLut[133] = 2047;
	attr.stManual.YRatioLut[134] = 2047;
	attr.stManual.YRatioLut[135] = 2047;
	attr.stManual.YRatioLut[136] = 2047;
	attr.stManual.YRatioLut[137] = 2047;
	attr.stManual.YRatioLut[138] = 2047;
	attr.stManual.YRatioLut[139] = 2047;
	attr.stManual.YRatioLut[140] = 2047;
	attr.stManual.YRatioLut[141] = 2047;
	attr.stManual.YRatioLut[142] = 2047;
	attr.stManual.YRatioLut[143] = 2047;
	attr.stManual.YRatioLut[144] = 2047;
	attr.stManual.YRatioLut[145] = 2047;
	attr.stManual.YRatioLut[146] = 2047;
	attr.stManual.YRatioLut[147] = 2047;
	attr.stManual.YRatioLut[148] = 2047;
	attr.stManual.YRatioLut[149] = 2047;
	attr.stManual.YRatioLut[150] = 2047;
	attr.stManual.YRatioLut[151] = 2047;
	attr.stManual.YRatioLut[152] = 2047;
	attr.stManual.YRatioLut[153] = 2047;
	attr.stManual.YRatioLut[154] = 2047;
	attr.stManual.YRatioLut[155] = 2047;
	attr.stManual.YRatioLut[156] = 2047;
	attr.stManual.YRatioLut[157] = 2047;
	attr.stManual.YRatioLut[158] = 2047;
	attr.stManual.YRatioLut[159] = 2047;
	attr.stManual.YRatioLut[160] = 2047;
	attr.stManual.YRatioLut[161] = 2047;
	attr.stManual.YRatioLut[162] = 2047;
	attr.stManual.YRatioLut[163] = 2047;
	attr.stManual.YRatioLut[164] = 2047;
	attr.stManual.YRatioLut[165] = 2047;
	attr.stManual.YRatioLut[166] = 2047;
	attr.stManual.YRatioLut[167] = 2047;
	attr.stManual.YRatioLut[168] = 2047;
	attr.stManual.YRatioLut[169] = 2047;
	attr.stManual.YRatioLut[170] = 2047;
	attr.stManual.YRatioLut[171] = 2047;
	attr.stManual.YRatioLut[172] = 2047;
	attr.stManual.YRatioLut[173] = 2047;
	attr.stManual.YRatioLut[174] = 2047;
	attr.stManual.YRatioLut[175] = 2047;
	attr.stManual.YRatioLut[176] = 2047;
	attr.stManual.YRatioLut[177] = 2047;
	attr.stManual.YRatioLut[178] = 2047;
	attr.stManual.YRatioLut[179] = 2047;
	attr.stManual.YRatioLut[180] = 2047;
	attr.stManual.YRatioLut[181] = 2047;
	attr.stManual.YRatioLut[182] = 2047;
	attr.stManual.YRatioLut[183] = 2047;
	attr.stManual.YRatioLut[184] = 2047;
	attr.stManual.YRatioLut[185] = 2047;
	attr.stManual.YRatioLut[186] = 2047;
	attr.stManual.YRatioLut[187] = 2047;
	attr.stManual.YRatioLut[188] = 2047;
	attr.stManual.YRatioLut[189] = 2047;
	attr.stManual.YRatioLut[190] = 2047;
	attr.stManual.YRatioLut[191] = 2047;
	attr.stManual.YRatioLut[192] = 2047;
	attr.stManual.YRatioLut[193] = 2047;
	attr.stManual.YRatioLut[194] = 2047;
	attr.stManual.YRatioLut[195] = 2047;
	attr.stManual.YRatioLut[196] = 2047;
	attr.stManual.YRatioLut[197] = 2047;
	attr.stManual.YRatioLut[198] = 2047;
	attr.stManual.YRatioLut[199] = 2047;
	attr.stManual.YRatioLut[200] = 2047;
	attr.stManual.YRatioLut[201] = 2047;
	attr.stManual.YRatioLut[202] = 2047;
	attr.stManual.YRatioLut[203] = 2047;
	attr.stManual.YRatioLut[204] = 2047;
	attr.stManual.YRatioLut[205] = 2047;
	attr.stManual.YRatioLut[206] = 2047;
	attr.stManual.YRatioLut[207] = 2047;
	attr.stManual.YRatioLut[208] = 2047;
	attr.stManual.YRatioLut[209] = 2047;
	attr.stManual.YRatioLut[210] = 2047;
	attr.stManual.YRatioLut[211] = 2047;
	attr.stManual.YRatioLut[212] = 2047;
	attr.stManual.YRatioLut[213] = 2047;
	attr.stManual.YRatioLut[214] = 2047;
	attr.stManual.YRatioLut[215] = 2047;
	attr.stManual.YRatioLut[216] = 2047;
	attr.stManual.YRatioLut[217] = 2047;
	attr.stManual.YRatioLut[218] = 2047;
	attr.stManual.YRatioLut[219] = 2047;
	attr.stManual.YRatioLut[220] = 2047;
	attr.stManual.YRatioLut[221] = 2047;
	attr.stManual.YRatioLut[222] = 2047;
	attr.stManual.YRatioLut[223] = 2047;
	attr.stManual.YRatioLut[224] = 2047;
	attr.stManual.YRatioLut[225] = 2047;
	attr.stManual.YRatioLut[226] = 2047;
	attr.stManual.YRatioLut[227] = 2047;
	attr.stManual.YRatioLut[228] = 2047;
	attr.stManual.YRatioLut[229] = 2047;
	attr.stManual.YRatioLut[230] = 2047;
	attr.stManual.YRatioLut[231] = 2047;
	attr.stManual.YRatioLut[232] = 2047;
	attr.stManual.YRatioLut[233] = 2047;
	attr.stManual.YRatioLut[234] = 2047;
	attr.stManual.YRatioLut[235] = 2047;
	attr.stManual.YRatioLut[236] = 2047;
	attr.stManual.YRatioLut[237] = 2047;
	attr.stManual.YRatioLut[238] = 2047;
	attr.stManual.YRatioLut[239] = 2047;
	attr.stManual.YRatioLut[240] = 2047;
	attr.stManual.YRatioLut[241] = 2047;
	attr.stManual.YRatioLut[242] = 2047;
	attr.stManual.YRatioLut[243] = 2047;
	attr.stManual.YRatioLut[244] = 2047;
	attr.stManual.YRatioLut[245] = 2047;
	attr.stManual.YRatioLut[246] = 2047;
	attr.stManual.YRatioLut[247] = 2047;
	attr.stManual.YRatioLut[248] = 2047;
	attr.stManual.YRatioLut[249] = 2047;
	attr.stManual.YRatioLut[250] = 2047;
	attr.stManual.YRatioLut[251] = 2047;
	attr.stManual.YRatioLut[252] = 2047;
	attr.stManual.YRatioLut[253] = 2047;
	attr.stManual.YRatioLut[254] = 2047;
	attr.stManual.YRatioLut[255] = 2047;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test maximum value failed at VD wait\n");
		return ret;
	}
	// set random value
	attr.stManual.YRatioLut[0] = rand_range(0, 2047);
	attr.stManual.YRatioLut[1] = rand_range(0, 2047);
	attr.stManual.YRatioLut[2] = rand_range(0, 2047);
	attr.stManual.YRatioLut[3] = rand_range(0, 2047);
	attr.stManual.YRatioLut[4] = rand_range(0, 2047);
	attr.stManual.YRatioLut[5] = rand_range(0, 2047);
	attr.stManual.YRatioLut[6] = rand_range(0, 2047);
	attr.stManual.YRatioLut[7] = rand_range(0, 2047);
	attr.stManual.YRatioLut[8] = rand_range(0, 2047);
	attr.stManual.YRatioLut[9] = rand_range(0, 2047);
	attr.stManual.YRatioLut[10] = rand_range(0, 2047);
	attr.stManual.YRatioLut[11] = rand_range(0, 2047);
	attr.stManual.YRatioLut[12] = rand_range(0, 2047);
	attr.stManual.YRatioLut[13] = rand_range(0, 2047);
	attr.stManual.YRatioLut[14] = rand_range(0, 2047);
	attr.stManual.YRatioLut[15] = rand_range(0, 2047);
	attr.stManual.YRatioLut[16] = rand_range(0, 2047);
	attr.stManual.YRatioLut[17] = rand_range(0, 2047);
	attr.stManual.YRatioLut[18] = rand_range(0, 2047);
	attr.stManual.YRatioLut[19] = rand_range(0, 2047);
	attr.stManual.YRatioLut[20] = rand_range(0, 2047);
	attr.stManual.YRatioLut[21] = rand_range(0, 2047);
	attr.stManual.YRatioLut[22] = rand_range(0, 2047);
	attr.stManual.YRatioLut[23] = rand_range(0, 2047);
	attr.stManual.YRatioLut[24] = rand_range(0, 2047);
	attr.stManual.YRatioLut[25] = rand_range(0, 2047);
	attr.stManual.YRatioLut[26] = rand_range(0, 2047);
	attr.stManual.YRatioLut[27] = rand_range(0, 2047);
	attr.stManual.YRatioLut[28] = rand_range(0, 2047);
	attr.stManual.YRatioLut[29] = rand_range(0, 2047);
	attr.stManual.YRatioLut[30] = rand_range(0, 2047);
	attr.stManual.YRatioLut[31] = rand_range(0, 2047);
	attr.stManual.YRatioLut[32] = rand_range(0, 2047);
	attr.stManual.YRatioLut[33] = rand_range(0, 2047);
	attr.stManual.YRatioLut[34] = rand_range(0, 2047);
	attr.stManual.YRatioLut[35] = rand_range(0, 2047);
	attr.stManual.YRatioLut[36] = rand_range(0, 2047);
	attr.stManual.YRatioLut[37] = rand_range(0, 2047);
	attr.stManual.YRatioLut[38] = rand_range(0, 2047);
	attr.stManual.YRatioLut[39] = rand_range(0, 2047);
	attr.stManual.YRatioLut[40] = rand_range(0, 2047);
	attr.stManual.YRatioLut[41] = rand_range(0, 2047);
	attr.stManual.YRatioLut[42] = rand_range(0, 2047);
	attr.stManual.YRatioLut[43] = rand_range(0, 2047);
	attr.stManual.YRatioLut[44] = rand_range(0, 2047);
	attr.stManual.YRatioLut[45] = rand_range(0, 2047);
	attr.stManual.YRatioLut[46] = rand_range(0, 2047);
	attr.stManual.YRatioLut[47] = rand_range(0, 2047);
	attr.stManual.YRatioLut[48] = rand_range(0, 2047);
	attr.stManual.YRatioLut[49] = rand_range(0, 2047);
	attr.stManual.YRatioLut[50] = rand_range(0, 2047);
	attr.stManual.YRatioLut[51] = rand_range(0, 2047);
	attr.stManual.YRatioLut[52] = rand_range(0, 2047);
	attr.stManual.YRatioLut[53] = rand_range(0, 2047);
	attr.stManual.YRatioLut[54] = rand_range(0, 2047);
	attr.stManual.YRatioLut[55] = rand_range(0, 2047);
	attr.stManual.YRatioLut[56] = rand_range(0, 2047);
	attr.stManual.YRatioLut[57] = rand_range(0, 2047);
	attr.stManual.YRatioLut[58] = rand_range(0, 2047);
	attr.stManual.YRatioLut[59] = rand_range(0, 2047);
	attr.stManual.YRatioLut[60] = rand_range(0, 2047);
	attr.stManual.YRatioLut[61] = rand_range(0, 2047);
	attr.stManual.YRatioLut[62] = rand_range(0, 2047);
	attr.stManual.YRatioLut[63] = rand_range(0, 2047);
	attr.stManual.YRatioLut[64] = rand_range(0, 2047);
	attr.stManual.YRatioLut[65] = rand_range(0, 2047);
	attr.stManual.YRatioLut[66] = rand_range(0, 2047);
	attr.stManual.YRatioLut[67] = rand_range(0, 2047);
	attr.stManual.YRatioLut[68] = rand_range(0, 2047);
	attr.stManual.YRatioLut[69] = rand_range(0, 2047);
	attr.stManual.YRatioLut[70] = rand_range(0, 2047);
	attr.stManual.YRatioLut[71] = rand_range(0, 2047);
	attr.stManual.YRatioLut[72] = rand_range(0, 2047);
	attr.stManual.YRatioLut[73] = rand_range(0, 2047);
	attr.stManual.YRatioLut[74] = rand_range(0, 2047);
	attr.stManual.YRatioLut[75] = rand_range(0, 2047);
	attr.stManual.YRatioLut[76] = rand_range(0, 2047);
	attr.stManual.YRatioLut[77] = rand_range(0, 2047);
	attr.stManual.YRatioLut[78] = rand_range(0, 2047);
	attr.stManual.YRatioLut[79] = rand_range(0, 2047);
	attr.stManual.YRatioLut[80] = rand_range(0, 2047);
	attr.stManual.YRatioLut[81] = rand_range(0, 2047);
	attr.stManual.YRatioLut[82] = rand_range(0, 2047);
	attr.stManual.YRatioLut[83] = rand_range(0, 2047);
	attr.stManual.YRatioLut[84] = rand_range(0, 2047);
	attr.stManual.YRatioLut[85] = rand_range(0, 2047);
	attr.stManual.YRatioLut[86] = rand_range(0, 2047);
	attr.stManual.YRatioLut[87] = rand_range(0, 2047);
	attr.stManual.YRatioLut[88] = rand_range(0, 2047);
	attr.stManual.YRatioLut[89] = rand_range(0, 2047);
	attr.stManual.YRatioLut[90] = rand_range(0, 2047);
	attr.stManual.YRatioLut[91] = rand_range(0, 2047);
	attr.stManual.YRatioLut[92] = rand_range(0, 2047);
	attr.stManual.YRatioLut[93] = rand_range(0, 2047);
	attr.stManual.YRatioLut[94] = rand_range(0, 2047);
	attr.stManual.YRatioLut[95] = rand_range(0, 2047);
	attr.stManual.YRatioLut[96] = rand_range(0, 2047);
	attr.stManual.YRatioLut[97] = rand_range(0, 2047);
	attr.stManual.YRatioLut[98] = rand_range(0, 2047);
	attr.stManual.YRatioLut[99] = rand_range(0, 2047);
	attr.stManual.YRatioLut[100] = rand_range(0, 2047);
	attr.stManual.YRatioLut[101] = rand_range(0, 2047);
	attr.stManual.YRatioLut[102] = rand_range(0, 2047);
	attr.stManual.YRatioLut[103] = rand_range(0, 2047);
	attr.stManual.YRatioLut[104] = rand_range(0, 2047);
	attr.stManual.YRatioLut[105] = rand_range(0, 2047);
	attr.stManual.YRatioLut[106] = rand_range(0, 2047);
	attr.stManual.YRatioLut[107] = rand_range(0, 2047);
	attr.stManual.YRatioLut[108] = rand_range(0, 2047);
	attr.stManual.YRatioLut[109] = rand_range(0, 2047);
	attr.stManual.YRatioLut[110] = rand_range(0, 2047);
	attr.stManual.YRatioLut[111] = rand_range(0, 2047);
	attr.stManual.YRatioLut[112] = rand_range(0, 2047);
	attr.stManual.YRatioLut[113] = rand_range(0, 2047);
	attr.stManual.YRatioLut[114] = rand_range(0, 2047);
	attr.stManual.YRatioLut[115] = rand_range(0, 2047);
	attr.stManual.YRatioLut[116] = rand_range(0, 2047);
	attr.stManual.YRatioLut[117] = rand_range(0, 2047);
	attr.stManual.YRatioLut[118] = rand_range(0, 2047);
	attr.stManual.YRatioLut[119] = rand_range(0, 2047);
	attr.stManual.YRatioLut[120] = rand_range(0, 2047);
	attr.stManual.YRatioLut[121] = rand_range(0, 2047);
	attr.stManual.YRatioLut[122] = rand_range(0, 2047);
	attr.stManual.YRatioLut[123] = rand_range(0, 2047);
	attr.stManual.YRatioLut[124] = rand_range(0, 2047);
	attr.stManual.YRatioLut[125] = rand_range(0, 2047);
	attr.stManual.YRatioLut[126] = rand_range(0, 2047);
	attr.stManual.YRatioLut[127] = rand_range(0, 2047);
	attr.stManual.YRatioLut[128] = rand_range(0, 2047);
	attr.stManual.YRatioLut[129] = rand_range(0, 2047);
	attr.stManual.YRatioLut[130] = rand_range(0, 2047);
	attr.stManual.YRatioLut[131] = rand_range(0, 2047);
	attr.stManual.YRatioLut[132] = rand_range(0, 2047);
	attr.stManual.YRatioLut[133] = rand_range(0, 2047);
	attr.stManual.YRatioLut[134] = rand_range(0, 2047);
	attr.stManual.YRatioLut[135] = rand_range(0, 2047);
	attr.stManual.YRatioLut[136] = rand_range(0, 2047);
	attr.stManual.YRatioLut[137] = rand_range(0, 2047);
	attr.stManual.YRatioLut[138] = rand_range(0, 2047);
	attr.stManual.YRatioLut[139] = rand_range(0, 2047);
	attr.stManual.YRatioLut[140] = rand_range(0, 2047);
	attr.stManual.YRatioLut[141] = rand_range(0, 2047);
	attr.stManual.YRatioLut[142] = rand_range(0, 2047);
	attr.stManual.YRatioLut[143] = rand_range(0, 2047);
	attr.stManual.YRatioLut[144] = rand_range(0, 2047);
	attr.stManual.YRatioLut[145] = rand_range(0, 2047);
	attr.stManual.YRatioLut[146] = rand_range(0, 2047);
	attr.stManual.YRatioLut[147] = rand_range(0, 2047);
	attr.stManual.YRatioLut[148] = rand_range(0, 2047);
	attr.stManual.YRatioLut[149] = rand_range(0, 2047);
	attr.stManual.YRatioLut[150] = rand_range(0, 2047);
	attr.stManual.YRatioLut[151] = rand_range(0, 2047);
	attr.stManual.YRatioLut[152] = rand_range(0, 2047);
	attr.stManual.YRatioLut[153] = rand_range(0, 2047);
	attr.stManual.YRatioLut[154] = rand_range(0, 2047);
	attr.stManual.YRatioLut[155] = rand_range(0, 2047);
	attr.stManual.YRatioLut[156] = rand_range(0, 2047);
	attr.stManual.YRatioLut[157] = rand_range(0, 2047);
	attr.stManual.YRatioLut[158] = rand_range(0, 2047);
	attr.stManual.YRatioLut[159] = rand_range(0, 2047);
	attr.stManual.YRatioLut[160] = rand_range(0, 2047);
	attr.stManual.YRatioLut[161] = rand_range(0, 2047);
	attr.stManual.YRatioLut[162] = rand_range(0, 2047);
	attr.stManual.YRatioLut[163] = rand_range(0, 2047);
	attr.stManual.YRatioLut[164] = rand_range(0, 2047);
	attr.stManual.YRatioLut[165] = rand_range(0, 2047);
	attr.stManual.YRatioLut[166] = rand_range(0, 2047);
	attr.stManual.YRatioLut[167] = rand_range(0, 2047);
	attr.stManual.YRatioLut[168] = rand_range(0, 2047);
	attr.stManual.YRatioLut[169] = rand_range(0, 2047);
	attr.stManual.YRatioLut[170] = rand_range(0, 2047);
	attr.stManual.YRatioLut[171] = rand_range(0, 2047);
	attr.stManual.YRatioLut[172] = rand_range(0, 2047);
	attr.stManual.YRatioLut[173] = rand_range(0, 2047);
	attr.stManual.YRatioLut[174] = rand_range(0, 2047);
	attr.stManual.YRatioLut[175] = rand_range(0, 2047);
	attr.stManual.YRatioLut[176] = rand_range(0, 2047);
	attr.stManual.YRatioLut[177] = rand_range(0, 2047);
	attr.stManual.YRatioLut[178] = rand_range(0, 2047);
	attr.stManual.YRatioLut[179] = rand_range(0, 2047);
	attr.stManual.YRatioLut[180] = rand_range(0, 2047);
	attr.stManual.YRatioLut[181] = rand_range(0, 2047);
	attr.stManual.YRatioLut[182] = rand_range(0, 2047);
	attr.stManual.YRatioLut[183] = rand_range(0, 2047);
	attr.stManual.YRatioLut[184] = rand_range(0, 2047);
	attr.stManual.YRatioLut[185] = rand_range(0, 2047);
	attr.stManual.YRatioLut[186] = rand_range(0, 2047);
	attr.stManual.YRatioLut[187] = rand_range(0, 2047);
	attr.stManual.YRatioLut[188] = rand_range(0, 2047);
	attr.stManual.YRatioLut[189] = rand_range(0, 2047);
	attr.stManual.YRatioLut[190] = rand_range(0, 2047);
	attr.stManual.YRatioLut[191] = rand_range(0, 2047);
	attr.stManual.YRatioLut[192] = rand_range(0, 2047);
	attr.stManual.YRatioLut[193] = rand_range(0, 2047);
	attr.stManual.YRatioLut[194] = rand_range(0, 2047);
	attr.stManual.YRatioLut[195] = rand_range(0, 2047);
	attr.stManual.YRatioLut[196] = rand_range(0, 2047);
	attr.stManual.YRatioLut[197] = rand_range(0, 2047);
	attr.stManual.YRatioLut[198] = rand_range(0, 2047);
	attr.stManual.YRatioLut[199] = rand_range(0, 2047);
	attr.stManual.YRatioLut[200] = rand_range(0, 2047);
	attr.stManual.YRatioLut[201] = rand_range(0, 2047);
	attr.stManual.YRatioLut[202] = rand_range(0, 2047);
	attr.stManual.YRatioLut[203] = rand_range(0, 2047);
	attr.stManual.YRatioLut[204] = rand_range(0, 2047);
	attr.stManual.YRatioLut[205] = rand_range(0, 2047);
	attr.stManual.YRatioLut[206] = rand_range(0, 2047);
	attr.stManual.YRatioLut[207] = rand_range(0, 2047);
	attr.stManual.YRatioLut[208] = rand_range(0, 2047);
	attr.stManual.YRatioLut[209] = rand_range(0, 2047);
	attr.stManual.YRatioLut[210] = rand_range(0, 2047);
	attr.stManual.YRatioLut[211] = rand_range(0, 2047);
	attr.stManual.YRatioLut[212] = rand_range(0, 2047);
	attr.stManual.YRatioLut[213] = rand_range(0, 2047);
	attr.stManual.YRatioLut[214] = rand_range(0, 2047);
	attr.stManual.YRatioLut[215] = rand_range(0, 2047);
	attr.stManual.YRatioLut[216] = rand_range(0, 2047);
	attr.stManual.YRatioLut[217] = rand_range(0, 2047);
	attr.stManual.YRatioLut[218] = rand_range(0, 2047);
	attr.stManual.YRatioLut[219] = rand_range(0, 2047);
	attr.stManual.YRatioLut[220] = rand_range(0, 2047);
	attr.stManual.YRatioLut[221] = rand_range(0, 2047);
	attr.stManual.YRatioLut[222] = rand_range(0, 2047);
	attr.stManual.YRatioLut[223] = rand_range(0, 2047);
	attr.stManual.YRatioLut[224] = rand_range(0, 2047);
	attr.stManual.YRatioLut[225] = rand_range(0, 2047);
	attr.stManual.YRatioLut[226] = rand_range(0, 2047);
	attr.stManual.YRatioLut[227] = rand_range(0, 2047);
	attr.stManual.YRatioLut[228] = rand_range(0, 2047);
	attr.stManual.YRatioLut[229] = rand_range(0, 2047);
	attr.stManual.YRatioLut[230] = rand_range(0, 2047);
	attr.stManual.YRatioLut[231] = rand_range(0, 2047);
	attr.stManual.YRatioLut[232] = rand_range(0, 2047);
	attr.stManual.YRatioLut[233] = rand_range(0, 2047);
	attr.stManual.YRatioLut[234] = rand_range(0, 2047);
	attr.stManual.YRatioLut[235] = rand_range(0, 2047);
	attr.stManual.YRatioLut[236] = rand_range(0, 2047);
	attr.stManual.YRatioLut[237] = rand_range(0, 2047);
	attr.stManual.YRatioLut[238] = rand_range(0, 2047);
	attr.stManual.YRatioLut[239] = rand_range(0, 2047);
	attr.stManual.YRatioLut[240] = rand_range(0, 2047);
	attr.stManual.YRatioLut[241] = rand_range(0, 2047);
	attr.stManual.YRatioLut[242] = rand_range(0, 2047);
	attr.stManual.YRatioLut[243] = rand_range(0, 2047);
	attr.stManual.YRatioLut[244] = rand_range(0, 2047);
	attr.stManual.YRatioLut[245] = rand_range(0, 2047);
	attr.stManual.YRatioLut[246] = rand_range(0, 2047);
	attr.stManual.YRatioLut[247] = rand_range(0, 2047);
	attr.stManual.YRatioLut[248] = rand_range(0, 2047);
	attr.stManual.YRatioLut[249] = rand_range(0, 2047);
	attr.stManual.YRatioLut[250] = rand_range(0, 2047);
	attr.stManual.YRatioLut[251] = rand_range(0, 2047);
	attr.stManual.YRatioLut[252] = rand_range(0, 2047);
	attr.stManual.YRatioLut[253] = rand_range(0, 2047);
	attr.stManual.YRatioLut[254] = rand_range(0, 2047);
	attr.stManual.YRatioLut[255] = rand_range(0, 2047);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed\n");
		return ret;
	}
	ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 200);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("test random value failed at VD wait\n");
		return ret;
	}
	// set abnormal value test
	attr.stManual.YRatioLut[0] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[1] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[2] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[3] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[4] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[5] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[6] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[7] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[8] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[9] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[10] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[11] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[12] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[13] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[14] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[15] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[16] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[17] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[18] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[19] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[20] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[21] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[22] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[23] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[24] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[25] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[26] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[27] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[28] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[29] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[30] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[31] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[32] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[33] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[34] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[35] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[36] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[37] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[38] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[39] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[40] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[41] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[42] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[43] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[44] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[45] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[46] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[47] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[48] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[49] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[50] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[51] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[52] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[53] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[54] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[55] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[56] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[57] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[58] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[59] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[60] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[61] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[62] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[63] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[64] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[65] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[66] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[67] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[68] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[69] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[70] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[71] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[72] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[73] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[74] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[75] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[76] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[77] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[78] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[79] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[80] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[81] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[82] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[83] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[84] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[85] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[86] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[87] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[88] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[89] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[90] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[91] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[92] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[93] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[94] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[95] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[96] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[97] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[98] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[99] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[100] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[101] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[102] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[103] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[104] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[105] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[106] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[107] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[108] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[109] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[110] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[111] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[112] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[113] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[114] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[115] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[116] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[117] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[118] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[119] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[120] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[121] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[122] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[123] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[124] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[125] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[126] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[127] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[128] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[129] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[130] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[131] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[132] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[133] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[134] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[135] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[136] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[137] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[138] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[139] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[140] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[141] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[142] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[143] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[144] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[145] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[146] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[147] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[148] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[149] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[150] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[151] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[152] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[153] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[154] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[155] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[156] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[157] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[158] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[159] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[160] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[161] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[162] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[163] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[164] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[165] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[166] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[167] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[168] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[169] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[170] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[171] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[172] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[173] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[174] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[175] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[176] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[177] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[178] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[179] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[180] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[181] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[182] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[183] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[184] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[185] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[186] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[187] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[188] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[189] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[190] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[191] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[192] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[193] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[194] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[195] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[196] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[197] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[198] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[199] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[200] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[201] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[202] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[203] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[204] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[205] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[206] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[207] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[208] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[209] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[210] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[211] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[212] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[213] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[214] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[215] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[216] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[217] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[218] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[219] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[220] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[221] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[222] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[223] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[224] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[225] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[226] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[227] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[228] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[229] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[230] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[231] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[232] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[233] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[234] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[235] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[236] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[237] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[238] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[239] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[240] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[241] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[242] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[243] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[244] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[245] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[246] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[247] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[248] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[249] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[250] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[251] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[252] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[253] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[254] = rand_range(2048, 65535);
	attr.stManual.YRatioLut[255] = rand_range(2048, 65535);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	attr.stManual.YRatioLut[0] = 128;
	attr.stManual.YRatioLut[1] = 128;
	attr.stManual.YRatioLut[2] = 128;
	attr.stManual.YRatioLut[3] = 128;
	attr.stManual.YRatioLut[4] = 128;
	attr.stManual.YRatioLut[5] = 128;
	attr.stManual.YRatioLut[6] = 128;
	attr.stManual.YRatioLut[7] = 128;
	attr.stManual.YRatioLut[8] = 128;
	attr.stManual.YRatioLut[9] = 128;
	attr.stManual.YRatioLut[10] = 128;
	attr.stManual.YRatioLut[11] = 128;
	attr.stManual.YRatioLut[12] = 128;
	attr.stManual.YRatioLut[13] = 128;
	attr.stManual.YRatioLut[14] = 128;
	attr.stManual.YRatioLut[15] = 128;
	attr.stManual.YRatioLut[16] = 128;
	attr.stManual.YRatioLut[17] = 128;
	attr.stManual.YRatioLut[18] = 128;
	attr.stManual.YRatioLut[19] = 128;
	attr.stManual.YRatioLut[20] = 128;
	attr.stManual.YRatioLut[21] = 128;
	attr.stManual.YRatioLut[22] = 128;
	attr.stManual.YRatioLut[23] = 128;
	attr.stManual.YRatioLut[24] = 128;
	attr.stManual.YRatioLut[25] = 128;
	attr.stManual.YRatioLut[26] = 128;
	attr.stManual.YRatioLut[27] = 128;
	attr.stManual.YRatioLut[28] = 128;
	attr.stManual.YRatioLut[29] = 128;
	attr.stManual.YRatioLut[30] = 128;
	attr.stManual.YRatioLut[31] = 128;
	attr.stManual.YRatioLut[32] = 128;
	attr.stManual.YRatioLut[33] = 128;
	attr.stManual.YRatioLut[34] = 128;
	attr.stManual.YRatioLut[35] = 128;
	attr.stManual.YRatioLut[36] = 128;
	attr.stManual.YRatioLut[37] = 128;
	attr.stManual.YRatioLut[38] = 128;
	attr.stManual.YRatioLut[39] = 128;
	attr.stManual.YRatioLut[40] = 128;
	attr.stManual.YRatioLut[41] = 128;
	attr.stManual.YRatioLut[42] = 128;
	attr.stManual.YRatioLut[43] = 128;
	attr.stManual.YRatioLut[44] = 128;
	attr.stManual.YRatioLut[45] = 128;
	attr.stManual.YRatioLut[46] = 128;
	attr.stManual.YRatioLut[47] = 128;
	attr.stManual.YRatioLut[48] = 128;
	attr.stManual.YRatioLut[49] = 128;
	attr.stManual.YRatioLut[50] = 128;
	attr.stManual.YRatioLut[51] = 128;
	attr.stManual.YRatioLut[52] = 128;
	attr.stManual.YRatioLut[53] = 128;
	attr.stManual.YRatioLut[54] = 128;
	attr.stManual.YRatioLut[55] = 128;
	attr.stManual.YRatioLut[56] = 128;
	attr.stManual.YRatioLut[57] = 128;
	attr.stManual.YRatioLut[58] = 128;
	attr.stManual.YRatioLut[59] = 128;
	attr.stManual.YRatioLut[60] = 128;
	attr.stManual.YRatioLut[61] = 128;
	attr.stManual.YRatioLut[62] = 128;
	attr.stManual.YRatioLut[63] = 128;
	attr.stManual.YRatioLut[64] = 128;
	attr.stManual.YRatioLut[65] = 128;
	attr.stManual.YRatioLut[66] = 128;
	attr.stManual.YRatioLut[67] = 128;
	attr.stManual.YRatioLut[68] = 128;
	attr.stManual.YRatioLut[69] = 128;
	attr.stManual.YRatioLut[70] = 128;
	attr.stManual.YRatioLut[71] = 128;
	attr.stManual.YRatioLut[72] = 128;
	attr.stManual.YRatioLut[73] = 128;
	attr.stManual.YRatioLut[74] = 128;
	attr.stManual.YRatioLut[75] = 128;
	attr.stManual.YRatioLut[76] = 128;
	attr.stManual.YRatioLut[77] = 128;
	attr.stManual.YRatioLut[78] = 128;
	attr.stManual.YRatioLut[79] = 128;
	attr.stManual.YRatioLut[80] = 128;
	attr.stManual.YRatioLut[81] = 128;
	attr.stManual.YRatioLut[82] = 128;
	attr.stManual.YRatioLut[83] = 128;
	attr.stManual.YRatioLut[84] = 128;
	attr.stManual.YRatioLut[85] = 128;
	attr.stManual.YRatioLut[86] = 128;
	attr.stManual.YRatioLut[87] = 128;
	attr.stManual.YRatioLut[88] = 128;
	attr.stManual.YRatioLut[89] = 128;
	attr.stManual.YRatioLut[90] = 128;
	attr.stManual.YRatioLut[91] = 128;
	attr.stManual.YRatioLut[92] = 128;
	attr.stManual.YRatioLut[93] = 128;
	attr.stManual.YRatioLut[94] = 128;
	attr.stManual.YRatioLut[95] = 128;
	attr.stManual.YRatioLut[96] = 128;
	attr.stManual.YRatioLut[97] = 128;
	attr.stManual.YRatioLut[98] = 128;
	attr.stManual.YRatioLut[99] = 128;
	attr.stManual.YRatioLut[100] = 128;
	attr.stManual.YRatioLut[101] = 128;
	attr.stManual.YRatioLut[102] = 128;
	attr.stManual.YRatioLut[103] = 128;
	attr.stManual.YRatioLut[104] = 128;
	attr.stManual.YRatioLut[105] = 128;
	attr.stManual.YRatioLut[106] = 128;
	attr.stManual.YRatioLut[107] = 128;
	attr.stManual.YRatioLut[108] = 128;
	attr.stManual.YRatioLut[109] = 128;
	attr.stManual.YRatioLut[110] = 128;
	attr.stManual.YRatioLut[111] = 128;
	attr.stManual.YRatioLut[112] = 128;
	attr.stManual.YRatioLut[113] = 128;
	attr.stManual.YRatioLut[114] = 128;
	attr.stManual.YRatioLut[115] = 128;
	attr.stManual.YRatioLut[116] = 128;
	attr.stManual.YRatioLut[117] = 128;
	attr.stManual.YRatioLut[118] = 128;
	attr.stManual.YRatioLut[119] = 128;
	attr.stManual.YRatioLut[120] = 128;
	attr.stManual.YRatioLut[121] = 128;
	attr.stManual.YRatioLut[122] = 128;
	attr.stManual.YRatioLut[123] = 128;
	attr.stManual.YRatioLut[124] = 128;
	attr.stManual.YRatioLut[125] = 128;
	attr.stManual.YRatioLut[126] = 128;
	attr.stManual.YRatioLut[127] = 128;
	attr.stManual.YRatioLut[128] = 128;
	attr.stManual.YRatioLut[129] = 128;
	attr.stManual.YRatioLut[130] = 128;
	attr.stManual.YRatioLut[131] = 128;
	attr.stManual.YRatioLut[132] = 128;
	attr.stManual.YRatioLut[133] = 128;
	attr.stManual.YRatioLut[134] = 128;
	attr.stManual.YRatioLut[135] = 128;
	attr.stManual.YRatioLut[136] = 128;
	attr.stManual.YRatioLut[137] = 128;
	attr.stManual.YRatioLut[138] = 128;
	attr.stManual.YRatioLut[139] = 128;
	attr.stManual.YRatioLut[140] = 128;
	attr.stManual.YRatioLut[141] = 128;
	attr.stManual.YRatioLut[142] = 128;
	attr.stManual.YRatioLut[143] = 128;
	attr.stManual.YRatioLut[144] = 128;
	attr.stManual.YRatioLut[145] = 128;
	attr.stManual.YRatioLut[146] = 128;
	attr.stManual.YRatioLut[147] = 128;
	attr.stManual.YRatioLut[148] = 128;
	attr.stManual.YRatioLut[149] = 128;
	attr.stManual.YRatioLut[150] = 128;
	attr.stManual.YRatioLut[151] = 128;
	attr.stManual.YRatioLut[152] = 128;
	attr.stManual.YRatioLut[153] = 128;
	attr.stManual.YRatioLut[154] = 128;
	attr.stManual.YRatioLut[155] = 128;
	attr.stManual.YRatioLut[156] = 128;
	attr.stManual.YRatioLut[157] = 128;
	attr.stManual.YRatioLut[158] = 128;
	attr.stManual.YRatioLut[159] = 128;
	attr.stManual.YRatioLut[160] = 128;
	attr.stManual.YRatioLut[161] = 128;
	attr.stManual.YRatioLut[162] = 128;
	attr.stManual.YRatioLut[163] = 128;
	attr.stManual.YRatioLut[164] = 128;
	attr.stManual.YRatioLut[165] = 128;
	attr.stManual.YRatioLut[166] = 128;
	attr.stManual.YRatioLut[167] = 128;
	attr.stManual.YRatioLut[168] = 128;
	attr.stManual.YRatioLut[169] = 128;
	attr.stManual.YRatioLut[170] = 128;
	attr.stManual.YRatioLut[171] = 128;
	attr.stManual.YRatioLut[172] = 128;
	attr.stManual.YRatioLut[173] = 128;
	attr.stManual.YRatioLut[174] = 128;
	attr.stManual.YRatioLut[175] = 128;
	attr.stManual.YRatioLut[176] = 128;
	attr.stManual.YRatioLut[177] = 128;
	attr.stManual.YRatioLut[178] = 128;
	attr.stManual.YRatioLut[179] = 128;
	attr.stManual.YRatioLut[180] = 128;
	attr.stManual.YRatioLut[181] = 128;
	attr.stManual.YRatioLut[182] = 128;
	attr.stManual.YRatioLut[183] = 128;
	attr.stManual.YRatioLut[184] = 128;
	attr.stManual.YRatioLut[185] = 128;
	attr.stManual.YRatioLut[186] = 128;
	attr.stManual.YRatioLut[187] = 128;
	attr.stManual.YRatioLut[188] = 128;
	attr.stManual.YRatioLut[189] = 128;
	attr.stManual.YRatioLut[190] = 128;
	attr.stManual.YRatioLut[191] = 128;
	attr.stManual.YRatioLut[192] = 128;
	attr.stManual.YRatioLut[193] = 128;
	attr.stManual.YRatioLut[194] = 128;
	attr.stManual.YRatioLut[195] = 128;
	attr.stManual.YRatioLut[196] = 128;
	attr.stManual.YRatioLut[197] = 128;
	attr.stManual.YRatioLut[198] = 128;
	attr.stManual.YRatioLut[199] = 128;
	attr.stManual.YRatioLut[200] = 128;
	attr.stManual.YRatioLut[201] = 128;
	attr.stManual.YRatioLut[202] = 128;
	attr.stManual.YRatioLut[203] = 128;
	attr.stManual.YRatioLut[204] = 128;
	attr.stManual.YRatioLut[205] = 128;
	attr.stManual.YRatioLut[206] = 128;
	attr.stManual.YRatioLut[207] = 128;
	attr.stManual.YRatioLut[208] = 128;
	attr.stManual.YRatioLut[209] = 128;
	attr.stManual.YRatioLut[210] = 128;
	attr.stManual.YRatioLut[211] = 128;
	attr.stManual.YRatioLut[212] = 128;
	attr.stManual.YRatioLut[213] = 128;
	attr.stManual.YRatioLut[214] = 128;
	attr.stManual.YRatioLut[215] = 128;
	attr.stManual.YRatioLut[216] = 128;
	attr.stManual.YRatioLut[217] = 128;
	attr.stManual.YRatioLut[218] = 128;
	attr.stManual.YRatioLut[219] = 128;
	attr.stManual.YRatioLut[220] = 128;
	attr.stManual.YRatioLut[221] = 128;
	attr.stManual.YRatioLut[222] = 128;
	attr.stManual.YRatioLut[223] = 128;
	attr.stManual.YRatioLut[224] = 128;
	attr.stManual.YRatioLut[225] = 128;
	attr.stManual.YRatioLut[226] = 128;
	attr.stManual.YRatioLut[227] = 128;
	attr.stManual.YRatioLut[228] = 128;
	attr.stManual.YRatioLut[229] = 128;
	attr.stManual.YRatioLut[230] = 128;
	attr.stManual.YRatioLut[231] = 128;
	attr.stManual.YRatioLut[232] = 128;
	attr.stManual.YRatioLut[233] = 128;
	attr.stManual.YRatioLut[234] = 128;
	attr.stManual.YRatioLut[235] = 128;
	attr.stManual.YRatioLut[236] = 128;
	attr.stManual.YRatioLut[237] = 128;
	attr.stManual.YRatioLut[238] = 128;
	attr.stManual.YRatioLut[239] = 128;
	attr.stManual.YRatioLut[240] = 128;
	attr.stManual.YRatioLut[241] = 128;
	attr.stManual.YRatioLut[242] = 128;
	attr.stManual.YRatioLut[243] = 128;
	attr.stManual.YRatioLut[244] = 128;
	attr.stManual.YRatioLut[245] = 128;
	attr.stManual.YRatioLut[246] = 128;
	attr.stManual.YRatioLut[247] = 128;
	attr.stManual.YRatioLut[248] = 128;
	attr.stManual.YRatioLut[249] = 128;
	attr.stManual.YRatioLut[250] = 128;
	attr.stManual.YRatioLut[251] = 128;
	attr.stManual.YRatioLut[252] = 128;
	attr.stManual.YRatioLut[253] = 128;
	attr.stManual.YRatioLut[254] = 128;
	attr.stManual.YRatioLut[255] = 128;
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_YRatioLut_manual(void)
{
	reg_api_test_fn(test_CA_CAAttr_YRatioLut_manual);
}

static int test_CA_CAAttr_YRatioLut_auto(VI_PIPE ViPipe)
{
	CVI_S32 ret = CVI_SUCCESS;
	ISP_CA_ATTR_S attr;

	ret = CVI_ISP_GetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_GetCAAttr failed\n");
		return ret;
	}
	// check default value test
	CVI_U16 default_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_0, attr.stAuto.YRatioLut[0], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_1, attr.stAuto.YRatioLut[1], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_2, attr.stAuto.YRatioLut[2], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_3, attr.stAuto.YRatioLut[3], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_4, attr.stAuto.YRatioLut[4], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_5, attr.stAuto.YRatioLut[5], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_6, attr.stAuto.YRatioLut[6], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_7, attr.stAuto.YRatioLut[7], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_8, attr.stAuto.YRatioLut[8], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_9, attr.stAuto.YRatioLut[9], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_10, attr.stAuto.YRatioLut[10], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_11, attr.stAuto.YRatioLut[11], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_12, attr.stAuto.YRatioLut[12], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_13, attr.stAuto.YRatioLut[13], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_14, attr.stAuto.YRatioLut[14], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_15, attr.stAuto.YRatioLut[15], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_16, attr.stAuto.YRatioLut[16], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_17[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_17, attr.stAuto.YRatioLut[17], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_18[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_18, attr.stAuto.YRatioLut[18], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_19[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_19, attr.stAuto.YRatioLut[19], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_20[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_20, attr.stAuto.YRatioLut[20], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_21[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_21, attr.stAuto.YRatioLut[21], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_22[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_22, attr.stAuto.YRatioLut[22], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_23[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_23, attr.stAuto.YRatioLut[23], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_24[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_24, attr.stAuto.YRatioLut[24], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_25[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_25, attr.stAuto.YRatioLut[25], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_26[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_26, attr.stAuto.YRatioLut[26], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_27[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_27, attr.stAuto.YRatioLut[27], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_28[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_28, attr.stAuto.YRatioLut[28], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_29[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_29, attr.stAuto.YRatioLut[29], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_30[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_30, attr.stAuto.YRatioLut[30], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_31[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_31, attr.stAuto.YRatioLut[31], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_32[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_32, attr.stAuto.YRatioLut[32], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_33[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_33, attr.stAuto.YRatioLut[33], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_34[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_34, attr.stAuto.YRatioLut[34], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_35[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_35, attr.stAuto.YRatioLut[35], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_36[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_36, attr.stAuto.YRatioLut[36], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_37[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_37, attr.stAuto.YRatioLut[37], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_38[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_38, attr.stAuto.YRatioLut[38], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_39[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_39, attr.stAuto.YRatioLut[39], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_40[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_40, attr.stAuto.YRatioLut[40], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_41[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_41, attr.stAuto.YRatioLut[41], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_42[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_42, attr.stAuto.YRatioLut[42], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_43[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_43, attr.stAuto.YRatioLut[43], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_44[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_44, attr.stAuto.YRatioLut[44], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_45[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_45, attr.stAuto.YRatioLut[45], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_46[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_46, attr.stAuto.YRatioLut[46], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_47[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_47, attr.stAuto.YRatioLut[47], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_48[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_48, attr.stAuto.YRatioLut[48], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_49[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_49, attr.stAuto.YRatioLut[49], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_50[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_50, attr.stAuto.YRatioLut[50], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_51[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_51, attr.stAuto.YRatioLut[51], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_52[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_52, attr.stAuto.YRatioLut[52], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_53[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_53, attr.stAuto.YRatioLut[53], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_54[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_54, attr.stAuto.YRatioLut[54], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_55[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_55, attr.stAuto.YRatioLut[55], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_56[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_56, attr.stAuto.YRatioLut[56], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_57[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_57, attr.stAuto.YRatioLut[57], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_58[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_58, attr.stAuto.YRatioLut[58], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_59[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_59, attr.stAuto.YRatioLut[59], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_60[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_60, attr.stAuto.YRatioLut[60], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_61[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_61, attr.stAuto.YRatioLut[61], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_62[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_62, attr.stAuto.YRatioLut[62], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_63[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_63, attr.stAuto.YRatioLut[63], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_64[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_64, attr.stAuto.YRatioLut[64], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_65[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_65, attr.stAuto.YRatioLut[65], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_66[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_66, attr.stAuto.YRatioLut[66], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_67[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_67, attr.stAuto.YRatioLut[67], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_68[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_68, attr.stAuto.YRatioLut[68], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_69[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_69, attr.stAuto.YRatioLut[69], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_70[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_70, attr.stAuto.YRatioLut[70], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_71[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_71, attr.stAuto.YRatioLut[71], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_72[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_72, attr.stAuto.YRatioLut[72], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_73[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_73, attr.stAuto.YRatioLut[73], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_74[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_74, attr.stAuto.YRatioLut[74], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_75[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_75, attr.stAuto.YRatioLut[75], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_76[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_76, attr.stAuto.YRatioLut[76], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_77[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_77, attr.stAuto.YRatioLut[77], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_78[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_78, attr.stAuto.YRatioLut[78], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_79[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_79, attr.stAuto.YRatioLut[79], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_80[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_80, attr.stAuto.YRatioLut[80], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_81[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_81, attr.stAuto.YRatioLut[81], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_82[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_82, attr.stAuto.YRatioLut[82], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_83[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_83, attr.stAuto.YRatioLut[83], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_84[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_84, attr.stAuto.YRatioLut[84], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_85[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_85, attr.stAuto.YRatioLut[85], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_86[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_86, attr.stAuto.YRatioLut[86], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_87[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_87, attr.stAuto.YRatioLut[87], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_88[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_88, attr.stAuto.YRatioLut[88], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_89[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_89, attr.stAuto.YRatioLut[89], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_90[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_90, attr.stAuto.YRatioLut[90], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_91[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_91, attr.stAuto.YRatioLut[91], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_92[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_92, attr.stAuto.YRatioLut[92], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_93[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_93, attr.stAuto.YRatioLut[93], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_94[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_94, attr.stAuto.YRatioLut[94], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_95[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_95, attr.stAuto.YRatioLut[95], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_96[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_96, attr.stAuto.YRatioLut[96], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_97[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_97, attr.stAuto.YRatioLut[97], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_98[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_98, attr.stAuto.YRatioLut[98], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_99[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_99, attr.stAuto.YRatioLut[99], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_100[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_100, attr.stAuto.YRatioLut[100], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_101[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_101, attr.stAuto.YRatioLut[101], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_102[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_102, attr.stAuto.YRatioLut[102], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_103[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_103, attr.stAuto.YRatioLut[103], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_104[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_104, attr.stAuto.YRatioLut[104], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_105[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_105, attr.stAuto.YRatioLut[105], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_106[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_106, attr.stAuto.YRatioLut[106], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_107[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_107, attr.stAuto.YRatioLut[107], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_108[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_108, attr.stAuto.YRatioLut[108], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_109[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_109, attr.stAuto.YRatioLut[109], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_110[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_110, attr.stAuto.YRatioLut[110], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_111[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_111, attr.stAuto.YRatioLut[111], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_112[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_112, attr.stAuto.YRatioLut[112], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_113[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_113, attr.stAuto.YRatioLut[113], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_114[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_114, attr.stAuto.YRatioLut[114], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_115[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_115, attr.stAuto.YRatioLut[115], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_116[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_116, attr.stAuto.YRatioLut[116], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_117[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_117, attr.stAuto.YRatioLut[117], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_118[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_118, attr.stAuto.YRatioLut[118], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_119[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_119, attr.stAuto.YRatioLut[119], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_120[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_120, attr.stAuto.YRatioLut[120], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_121[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_121, attr.stAuto.YRatioLut[121], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_122[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_122, attr.stAuto.YRatioLut[122], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_123[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_123, attr.stAuto.YRatioLut[123], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_124[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_124, attr.stAuto.YRatioLut[124], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_125[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_125, attr.stAuto.YRatioLut[125], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_126[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_126, attr.stAuto.YRatioLut[126], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_127[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_127, attr.stAuto.YRatioLut[127], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_128[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_128, attr.stAuto.YRatioLut[128], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_129[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_129, attr.stAuto.YRatioLut[129], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_130[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_130, attr.stAuto.YRatioLut[130], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_131[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_131, attr.stAuto.YRatioLut[131], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_132[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_132, attr.stAuto.YRatioLut[132], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_133[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_133, attr.stAuto.YRatioLut[133], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_134[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_134, attr.stAuto.YRatioLut[134], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_135[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_135, attr.stAuto.YRatioLut[135], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_136[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_136, attr.stAuto.YRatioLut[136], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_137[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_137, attr.stAuto.YRatioLut[137], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_138[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_138, attr.stAuto.YRatioLut[138], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_139[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_139, attr.stAuto.YRatioLut[139], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_140[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_140, attr.stAuto.YRatioLut[140], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_141[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_141, attr.stAuto.YRatioLut[141], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_142[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_142, attr.stAuto.YRatioLut[142], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_143[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_143, attr.stAuto.YRatioLut[143], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_144[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_144, attr.stAuto.YRatioLut[144], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_145[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_145, attr.stAuto.YRatioLut[145], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_146[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_146, attr.stAuto.YRatioLut[146], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_147[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_147, attr.stAuto.YRatioLut[147], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_148[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_148, attr.stAuto.YRatioLut[148], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_149[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_149, attr.stAuto.YRatioLut[149], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_150[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_150, attr.stAuto.YRatioLut[150], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_151[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_151, attr.stAuto.YRatioLut[151], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_152[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_152, attr.stAuto.YRatioLut[152], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_153[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_153, attr.stAuto.YRatioLut[153], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_154[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_154, attr.stAuto.YRatioLut[154], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_155[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_155, attr.stAuto.YRatioLut[155], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_156[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_156, attr.stAuto.YRatioLut[156], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_157[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_157, attr.stAuto.YRatioLut[157], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_158[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_158, attr.stAuto.YRatioLut[158], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_159[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_159, attr.stAuto.YRatioLut[159], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_160[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_160, attr.stAuto.YRatioLut[160], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_161[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_161, attr.stAuto.YRatioLut[161], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_162[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_162, attr.stAuto.YRatioLut[162], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_163[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_163, attr.stAuto.YRatioLut[163], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_164[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_164, attr.stAuto.YRatioLut[164], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_165[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_165, attr.stAuto.YRatioLut[165], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_166[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_166, attr.stAuto.YRatioLut[166], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_167[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_167, attr.stAuto.YRatioLut[167], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_168[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_168, attr.stAuto.YRatioLut[168], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_169[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_169, attr.stAuto.YRatioLut[169], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_170[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_170, attr.stAuto.YRatioLut[170], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_171[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_171, attr.stAuto.YRatioLut[171], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_172[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_172, attr.stAuto.YRatioLut[172], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_173[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_173, attr.stAuto.YRatioLut[173], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_174[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_174, attr.stAuto.YRatioLut[174], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_175[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_175, attr.stAuto.YRatioLut[175], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_176[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_176, attr.stAuto.YRatioLut[176], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_177[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_177, attr.stAuto.YRatioLut[177], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_178[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_178, attr.stAuto.YRatioLut[178], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_179[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_179, attr.stAuto.YRatioLut[179], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_180[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_180, attr.stAuto.YRatioLut[180], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_181[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_181, attr.stAuto.YRatioLut[181], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_182[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_182, attr.stAuto.YRatioLut[182], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_183[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_183, attr.stAuto.YRatioLut[183], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_184[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_184, attr.stAuto.YRatioLut[184], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_185[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_185, attr.stAuto.YRatioLut[185], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_186[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_186, attr.stAuto.YRatioLut[186], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_187[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_187, attr.stAuto.YRatioLut[187], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_188[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_188, attr.stAuto.YRatioLut[188], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_189[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_189, attr.stAuto.YRatioLut[189], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_190[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_190, attr.stAuto.YRatioLut[190], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_191[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_191, attr.stAuto.YRatioLut[191], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_192[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_192, attr.stAuto.YRatioLut[192], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_193[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_193, attr.stAuto.YRatioLut[193], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_194[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_194, attr.stAuto.YRatioLut[194], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_195[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_195, attr.stAuto.YRatioLut[195], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_196[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_196, attr.stAuto.YRatioLut[196], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_197[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_197, attr.stAuto.YRatioLut[197], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_198[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_198, attr.stAuto.YRatioLut[198], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_199[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_199, attr.stAuto.YRatioLut[199], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_200[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_200, attr.stAuto.YRatioLut[200], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_201[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_201, attr.stAuto.YRatioLut[201], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_202[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_202, attr.stAuto.YRatioLut[202], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_203[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_203, attr.stAuto.YRatioLut[203], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_204[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_204, attr.stAuto.YRatioLut[204], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_205[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_205, attr.stAuto.YRatioLut[205], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_206[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_206, attr.stAuto.YRatioLut[206], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_207[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_207, attr.stAuto.YRatioLut[207], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_208[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_208, attr.stAuto.YRatioLut[208], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_209[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_209, attr.stAuto.YRatioLut[209], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_210[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_210, attr.stAuto.YRatioLut[210], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_211[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_211, attr.stAuto.YRatioLut[211], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_212[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_212, attr.stAuto.YRatioLut[212], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_213[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_213, attr.stAuto.YRatioLut[213], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_214[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_214, attr.stAuto.YRatioLut[214], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_215[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_215, attr.stAuto.YRatioLut[215], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_216[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_216, attr.stAuto.YRatioLut[216], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_217[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_217, attr.stAuto.YRatioLut[217], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_218[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_218, attr.stAuto.YRatioLut[218], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_219[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_219, attr.stAuto.YRatioLut[219], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_220[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_220, attr.stAuto.YRatioLut[220], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_221[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_221, attr.stAuto.YRatioLut[221], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_222[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_222, attr.stAuto.YRatioLut[222], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_223[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_223, attr.stAuto.YRatioLut[223], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_224[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_224, attr.stAuto.YRatioLut[224], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_225[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_225, attr.stAuto.YRatioLut[225], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_226[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_226, attr.stAuto.YRatioLut[226], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_227[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_227, attr.stAuto.YRatioLut[227], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_228[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_228, attr.stAuto.YRatioLut[228], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_229[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_229, attr.stAuto.YRatioLut[229], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_230[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_230, attr.stAuto.YRatioLut[230], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_231[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_231, attr.stAuto.YRatioLut[231], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_232[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_232, attr.stAuto.YRatioLut[232], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_233[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_233, attr.stAuto.YRatioLut[233], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_234[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_234, attr.stAuto.YRatioLut[234], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_235[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_235, attr.stAuto.YRatioLut[235], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_236[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_236, attr.stAuto.YRatioLut[236], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_237[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_237, attr.stAuto.YRatioLut[237], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_238[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_238, attr.stAuto.YRatioLut[238], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_239[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_239, attr.stAuto.YRatioLut[239], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_240[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_240, attr.stAuto.YRatioLut[240], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_241[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_241, attr.stAuto.YRatioLut[241], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_242[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_242, attr.stAuto.YRatioLut[242], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_243[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_243, attr.stAuto.YRatioLut[243], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_244[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_244, attr.stAuto.YRatioLut[244], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_245[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_245, attr.stAuto.YRatioLut[245], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_246[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_246, attr.stAuto.YRatioLut[246], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_247[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_247, attr.stAuto.YRatioLut[247], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_248[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_248, attr.stAuto.YRatioLut[248], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_249[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_249, attr.stAuto.YRatioLut[249], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_250[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_250, attr.stAuto.YRatioLut[250], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_251[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_251, attr.stAuto.YRatioLut[251], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_252[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_252, attr.stAuto.YRatioLut[252], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_253[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_253, attr.stAuto.YRatioLut[253], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_254[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_254, attr.stAuto.YRatioLut[254], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	CVI_U16 default_val_255[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	if (memcmp(default_val_255, attr.stAuto.YRatioLut[255], sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM) != 0) {
		TEST_FAIL("test default value failed\n");
		return CVI_FAILURE;
	}
	// set minimum value
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[0][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[1][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[2][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[3][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[4][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[5][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[6][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[7][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[8][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[9][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[10][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[11][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[12][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[13][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[14][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[15][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[16][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[17][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[18][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[19][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[20][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[21][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[22][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[23][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[24][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[25][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[26][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[27][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[28][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[29][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[30][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[31][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[32][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[33][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[34][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[35][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[36][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[37][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[38][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[39][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[40][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[41][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[42][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[43][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[44][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[45][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[46][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[47][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[48][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[49][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[50][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[51][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[52][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[53][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[54][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[55][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[56][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[57][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[58][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[59][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[60][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[61][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[62][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[63][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[64][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[65][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[66][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[67][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[68][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[69][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[70][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[71][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[72][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[73][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[74][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[75][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[76][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[77][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[78][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[79][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[80][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[81][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[82][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[83][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[84][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[85][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[86][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[87][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[88][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[89][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[90][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[91][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[92][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[93][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[94][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[95][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[96][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[97][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[98][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[99][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[100][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[101][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[102][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[103][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[104][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[105][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[106][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[107][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[108][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[109][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[110][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[111][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[112][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[113][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[114][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[115][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[116][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[117][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[118][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[119][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[120][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[121][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[122][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[123][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[124][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[125][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[126][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[127][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[128][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[129][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[130][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[131][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[132][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[133][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[134][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[135][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[136][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[137][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[138][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[139][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[140][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[141][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[142][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[143][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[144][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[145][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[146][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[147][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[148][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[149][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[150][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[151][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[152][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[153][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[154][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[155][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[156][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[157][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[158][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[159][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[160][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[161][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[162][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[163][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[164][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[165][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[166][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[167][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[168][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[169][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[170][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[171][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[172][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[173][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[174][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[175][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[176][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[177][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[178][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[179][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[180][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[181][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[182][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[183][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[184][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[185][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[186][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[187][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[188][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[189][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[190][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[191][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[192][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[193][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[194][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[195][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[196][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[197][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[198][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[199][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[200][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[201][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[202][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[203][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[204][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[205][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[206][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[207][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[208][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[209][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[210][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[211][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[212][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[213][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[214][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[215][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[216][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[217][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[218][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[219][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[220][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[221][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[222][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[223][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[224][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[225][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[226][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[227][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[228][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[229][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[230][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[231][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[232][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[233][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[234][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[235][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[236][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[237][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[238][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[239][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[240][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[241][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[242][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[243][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[244][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[245][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[246][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[247][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[248][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[249][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[250][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[251][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[252][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[253][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[254][j] = 0;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[255][j] = 0;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.YRatioLut[0][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[1][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[2][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[3][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[4][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[5][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[6][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[7][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[8][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[9][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[10][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[11][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[12][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[13][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[14][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[15][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[16][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[17][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[18][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[19][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[20][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[21][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[22][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[23][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[24][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[25][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[26][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[27][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[28][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[29][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[30][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[31][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[32][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[33][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[34][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[35][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[36][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[37][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[38][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[39][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[40][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[41][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[42][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[43][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[44][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[45][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[46][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[47][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[48][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[49][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[50][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[51][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[52][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[53][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[54][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[55][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[56][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[57][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[58][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[59][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[60][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[61][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[62][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[63][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[64][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[65][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[66][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[67][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[68][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[69][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[70][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[71][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[72][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[73][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[74][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[75][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[76][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[77][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[78][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[79][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[80][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[81][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[82][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[83][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[84][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[85][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[86][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[87][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[88][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[89][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[90][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[91][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[92][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[93][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[94][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[95][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[96][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[97][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[98][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[99][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[100][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[101][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[102][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[103][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[104][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[105][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[106][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[107][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[108][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[109][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[110][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[111][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[112][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[113][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[114][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[115][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[116][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[117][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[118][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[119][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[120][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[121][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[122][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[123][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[124][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[125][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[126][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[127][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[128][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[129][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[130][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[131][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[132][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[133][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[134][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[135][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[136][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[137][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[138][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[139][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[140][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[141][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[142][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[143][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[144][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[145][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[146][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[147][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[148][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[149][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[150][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[151][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[152][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[153][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[154][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[155][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[156][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[157][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[158][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[159][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[160][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[161][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[162][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[163][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[164][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[165][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[166][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[167][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[168][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[169][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[170][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[171][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[172][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[173][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[174][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[175][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[176][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[177][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[178][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[179][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[180][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[181][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[182][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[183][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[184][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[185][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[186][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[187][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[188][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[189][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[190][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[191][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[192][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[193][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[194][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[195][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[196][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[197][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[198][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[199][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[200][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[201][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[202][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[203][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[204][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[205][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[206][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[207][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[208][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[209][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[210][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[211][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[212][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[213][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[214][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[215][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[216][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[217][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[218][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[219][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[220][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[221][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[222][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[223][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[224][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[225][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[226][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[227][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[228][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[229][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[230][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[231][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[232][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[233][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[234][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[235][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[236][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[237][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[238][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[239][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[240][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[241][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[242][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[243][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[244][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[245][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[246][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[247][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[248][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[249][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[250][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[251][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[252][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[253][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[254][j] = 2047;
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[255][j] = 2047;
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.YRatioLut[0][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[1][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[2][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[3][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[4][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[5][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[6][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[7][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[8][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[9][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[10][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[11][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[12][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[13][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[14][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[15][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[16][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[17][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[18][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[19][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[20][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[21][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[22][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[23][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[24][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[25][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[26][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[27][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[28][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[29][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[30][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[31][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[32][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[33][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[34][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[35][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[36][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[37][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[38][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[39][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[40][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[41][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[42][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[43][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[44][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[45][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[46][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[47][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[48][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[49][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[50][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[51][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[52][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[53][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[54][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[55][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[56][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[57][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[58][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[59][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[60][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[61][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[62][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[63][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[64][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[65][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[66][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[67][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[68][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[69][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[70][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[71][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[72][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[73][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[74][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[75][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[76][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[77][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[78][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[79][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[80][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[81][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[82][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[83][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[84][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[85][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[86][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[87][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[88][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[89][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[90][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[91][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[92][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[93][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[94][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[95][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[96][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[97][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[98][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[99][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[100][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[101][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[102][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[103][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[104][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[105][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[106][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[107][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[108][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[109][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[110][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[111][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[112][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[113][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[114][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[115][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[116][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[117][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[118][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[119][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[120][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[121][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[122][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[123][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[124][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[125][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[126][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[127][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[128][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[129][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[130][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[131][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[132][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[133][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[134][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[135][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[136][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[137][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[138][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[139][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[140][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[141][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[142][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[143][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[144][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[145][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[146][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[147][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[148][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[149][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[150][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[151][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[152][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[153][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[154][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[155][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[156][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[157][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[158][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[159][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[160][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[161][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[162][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[163][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[164][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[165][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[166][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[167][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[168][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[169][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[170][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[171][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[172][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[173][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[174][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[175][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[176][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[177][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[178][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[179][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[180][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[181][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[182][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[183][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[184][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[185][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[186][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[187][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[188][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[189][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[190][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[191][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[192][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[193][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[194][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[195][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[196][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[197][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[198][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[199][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[200][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[201][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[202][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[203][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[204][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[205][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[206][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[207][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[208][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[209][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[210][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[211][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[212][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[213][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[214][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[215][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[216][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[217][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[218][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[219][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[220][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[221][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[222][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[223][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[224][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[225][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[226][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[227][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[228][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[229][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[230][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[231][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[232][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[233][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[234][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[235][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[236][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[237][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[238][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[239][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[240][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[241][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[242][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[243][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[244][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[245][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[246][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[247][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[248][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[249][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[250][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[251][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[252][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[253][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[254][j] = rand_range(0, 2047);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[255][j] = rand_range(0, 2047);
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
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
		attr.stAuto.YRatioLut[0][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[1][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[2][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[3][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[4][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[5][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[6][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[7][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[8][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[9][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[10][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[11][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[12][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[13][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[14][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[15][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[16][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[17][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[18][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[19][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[20][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[21][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[22][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[23][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[24][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[25][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[26][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[27][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[28][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[29][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[30][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[31][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[32][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[33][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[34][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[35][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[36][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[37][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[38][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[39][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[40][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[41][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[42][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[43][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[44][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[45][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[46][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[47][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[48][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[49][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[50][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[51][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[52][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[53][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[54][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[55][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[56][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[57][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[58][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[59][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[60][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[61][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[62][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[63][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[64][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[65][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[66][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[67][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[68][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[69][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[70][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[71][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[72][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[73][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[74][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[75][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[76][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[77][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[78][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[79][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[80][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[81][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[82][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[83][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[84][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[85][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[86][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[87][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[88][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[89][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[90][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[91][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[92][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[93][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[94][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[95][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[96][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[97][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[98][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[99][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[100][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[101][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[102][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[103][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[104][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[105][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[106][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[107][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[108][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[109][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[110][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[111][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[112][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[113][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[114][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[115][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[116][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[117][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[118][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[119][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[120][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[121][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[122][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[123][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[124][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[125][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[126][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[127][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[128][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[129][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[130][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[131][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[132][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[133][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[134][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[135][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[136][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[137][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[138][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[139][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[140][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[141][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[142][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[143][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[144][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[145][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[146][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[147][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[148][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[149][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[150][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[151][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[152][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[153][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[154][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[155][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[156][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[157][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[158][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[159][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[160][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[161][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[162][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[163][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[164][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[165][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[166][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[167][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[168][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[169][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[170][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[171][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[172][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[173][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[174][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[175][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[176][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[177][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[178][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[179][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[180][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[181][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[182][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[183][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[184][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[185][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[186][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[187][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[188][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[189][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[190][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[191][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[192][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[193][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[194][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[195][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[196][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[197][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[198][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[199][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[200][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[201][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[202][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[203][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[204][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[205][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[206][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[207][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[208][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[209][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[210][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[211][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[212][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[213][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[214][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[215][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[216][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[217][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[218][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[219][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[220][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[221][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[222][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[223][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[224][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[225][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[226][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[227][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[228][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[229][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[230][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[231][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[232][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[233][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[234][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[235][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[236][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[237][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[238][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[239][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[240][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[241][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[242][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[243][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[244][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[245][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[246][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[247][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[248][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[249][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[250][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[251][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[252][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[253][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[254][j] = rand_range(2048, 65535);
	}
	for (int j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; ++j) {
		attr.stAuto.YRatioLut[255][j] = rand_range(2048, 65535);
	}
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret == CVI_SUCCESS) {
		TEST_FAIL("test abnormal value should fail\n");
		return CVI_FAILURE;
	}
	// restore default value
	CVI_U16 restore_val_0[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[0], restore_val_0, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_1[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[1], restore_val_1, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_2[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[2], restore_val_2, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_3[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[3], restore_val_3, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_4[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[4], restore_val_4, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_5[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[5], restore_val_5, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_6[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[6], restore_val_6, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_7[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[7], restore_val_7, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_8[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[8], restore_val_8, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_9[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[9], restore_val_9, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_10[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[10], restore_val_10, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_11[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[11], restore_val_11, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_12[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[12], restore_val_12, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_13[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[13], restore_val_13, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_14[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[14], restore_val_14, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_15[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[15], restore_val_15, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_16[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[16], restore_val_16, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_17[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[17], restore_val_17, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_18[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[18], restore_val_18, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_19[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[19], restore_val_19, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_20[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[20], restore_val_20, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_21[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[21], restore_val_21, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_22[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[22], restore_val_22, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_23[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[23], restore_val_23, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_24[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[24], restore_val_24, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_25[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[25], restore_val_25, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_26[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[26], restore_val_26, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_27[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[27], restore_val_27, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_28[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[28], restore_val_28, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_29[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[29], restore_val_29, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_30[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[30], restore_val_30, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_31[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[31], restore_val_31, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_32[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[32], restore_val_32, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_33[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[33], restore_val_33, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_34[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[34], restore_val_34, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_35[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[35], restore_val_35, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_36[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[36], restore_val_36, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_37[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[37], restore_val_37, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_38[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[38], restore_val_38, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_39[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[39], restore_val_39, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_40[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[40], restore_val_40, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_41[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[41], restore_val_41, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_42[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[42], restore_val_42, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_43[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[43], restore_val_43, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_44[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[44], restore_val_44, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_45[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[45], restore_val_45, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_46[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[46], restore_val_46, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_47[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[47], restore_val_47, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_48[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[48], restore_val_48, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_49[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[49], restore_val_49, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_50[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[50], restore_val_50, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_51[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[51], restore_val_51, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_52[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[52], restore_val_52, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_53[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[53], restore_val_53, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_54[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[54], restore_val_54, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_55[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[55], restore_val_55, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_56[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[56], restore_val_56, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_57[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[57], restore_val_57, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_58[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[58], restore_val_58, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_59[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[59], restore_val_59, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_60[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[60], restore_val_60, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_61[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[61], restore_val_61, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_62[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[62], restore_val_62, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_63[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[63], restore_val_63, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_64[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[64], restore_val_64, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_65[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[65], restore_val_65, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_66[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[66], restore_val_66, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_67[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[67], restore_val_67, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_68[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[68], restore_val_68, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_69[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[69], restore_val_69, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_70[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[70], restore_val_70, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_71[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[71], restore_val_71, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_72[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[72], restore_val_72, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_73[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[73], restore_val_73, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_74[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[74], restore_val_74, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_75[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[75], restore_val_75, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_76[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[76], restore_val_76, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_77[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[77], restore_val_77, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_78[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[78], restore_val_78, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_79[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[79], restore_val_79, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_80[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[80], restore_val_80, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_81[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[81], restore_val_81, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_82[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[82], restore_val_82, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_83[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[83], restore_val_83, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_84[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[84], restore_val_84, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_85[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[85], restore_val_85, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_86[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[86], restore_val_86, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_87[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[87], restore_val_87, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_88[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[88], restore_val_88, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_89[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[89], restore_val_89, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_90[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[90], restore_val_90, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_91[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[91], restore_val_91, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_92[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[92], restore_val_92, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_93[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[93], restore_val_93, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_94[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[94], restore_val_94, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_95[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[95], restore_val_95, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_96[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[96], restore_val_96, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_97[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[97], restore_val_97, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_98[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[98], restore_val_98, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_99[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[99], restore_val_99, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_100[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[100], restore_val_100, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_101[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[101], restore_val_101, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_102[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[102], restore_val_102, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_103[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[103], restore_val_103, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_104[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[104], restore_val_104, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_105[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[105], restore_val_105, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_106[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[106], restore_val_106, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_107[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[107], restore_val_107, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_108[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[108], restore_val_108, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_109[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[109], restore_val_109, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_110[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[110], restore_val_110, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_111[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[111], restore_val_111, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_112[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[112], restore_val_112, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_113[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[113], restore_val_113, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_114[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[114], restore_val_114, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_115[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[115], restore_val_115, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_116[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[116], restore_val_116, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_117[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[117], restore_val_117, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_118[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[118], restore_val_118, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_119[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[119], restore_val_119, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_120[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[120], restore_val_120, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_121[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[121], restore_val_121, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_122[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[122], restore_val_122, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_123[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[123], restore_val_123, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_124[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[124], restore_val_124, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_125[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[125], restore_val_125, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_126[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[126], restore_val_126, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_127[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[127], restore_val_127, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_128[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[128], restore_val_128, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_129[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[129], restore_val_129, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_130[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[130], restore_val_130, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_131[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[131], restore_val_131, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_132[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[132], restore_val_132, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_133[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[133], restore_val_133, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_134[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[134], restore_val_134, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_135[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[135], restore_val_135, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_136[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[136], restore_val_136, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_137[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[137], restore_val_137, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_138[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[138], restore_val_138, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_139[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[139], restore_val_139, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_140[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[140], restore_val_140, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_141[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[141], restore_val_141, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_142[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[142], restore_val_142, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_143[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[143], restore_val_143, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_144[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[144], restore_val_144, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_145[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[145], restore_val_145, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_146[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[146], restore_val_146, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_147[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[147], restore_val_147, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_148[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[148], restore_val_148, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_149[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[149], restore_val_149, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_150[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[150], restore_val_150, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_151[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[151], restore_val_151, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_152[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[152], restore_val_152, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_153[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[153], restore_val_153, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_154[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[154], restore_val_154, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_155[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[155], restore_val_155, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_156[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[156], restore_val_156, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_157[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[157], restore_val_157, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_158[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[158], restore_val_158, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_159[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[159], restore_val_159, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_160[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[160], restore_val_160, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_161[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[161], restore_val_161, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_162[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[162], restore_val_162, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_163[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[163], restore_val_163, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_164[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[164], restore_val_164, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_165[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[165], restore_val_165, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_166[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[166], restore_val_166, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_167[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[167], restore_val_167, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_168[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[168], restore_val_168, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_169[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[169], restore_val_169, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_170[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[170], restore_val_170, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_171[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[171], restore_val_171, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_172[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[172], restore_val_172, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_173[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[173], restore_val_173, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_174[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[174], restore_val_174, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_175[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[175], restore_val_175, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_176[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[176], restore_val_176, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_177[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[177], restore_val_177, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_178[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[178], restore_val_178, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_179[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[179], restore_val_179, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_180[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[180], restore_val_180, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_181[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[181], restore_val_181, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_182[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[182], restore_val_182, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_183[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[183], restore_val_183, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_184[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[184], restore_val_184, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_185[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[185], restore_val_185, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_186[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[186], restore_val_186, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_187[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[187], restore_val_187, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_188[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[188], restore_val_188, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_189[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[189], restore_val_189, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_190[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[190], restore_val_190, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_191[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[191], restore_val_191, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_192[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[192], restore_val_192, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_193[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[193], restore_val_193, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_194[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[194], restore_val_194, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_195[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[195], restore_val_195, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_196[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[196], restore_val_196, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_197[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[197], restore_val_197, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_198[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[198], restore_val_198, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_199[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[199], restore_val_199, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_200[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[200], restore_val_200, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_201[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[201], restore_val_201, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_202[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[202], restore_val_202, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_203[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[203], restore_val_203, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_204[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[204], restore_val_204, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_205[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[205], restore_val_205, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_206[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[206], restore_val_206, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_207[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[207], restore_val_207, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_208[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[208], restore_val_208, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_209[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[209], restore_val_209, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_210[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[210], restore_val_210, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_211[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[211], restore_val_211, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_212[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[212], restore_val_212, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_213[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[213], restore_val_213, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_214[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[214], restore_val_214, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_215[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[215], restore_val_215, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_216[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[216], restore_val_216, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_217[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[217], restore_val_217, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_218[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[218], restore_val_218, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_219[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[219], restore_val_219, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_220[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[220], restore_val_220, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_221[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[221], restore_val_221, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_222[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[222], restore_val_222, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_223[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[223], restore_val_223, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_224[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[224], restore_val_224, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_225[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[225], restore_val_225, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_226[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[226], restore_val_226, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_227[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[227], restore_val_227, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_228[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[228], restore_val_228, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_229[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[229], restore_val_229, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_230[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[230], restore_val_230, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_231[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[231], restore_val_231, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_232[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[232], restore_val_232, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_233[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[233], restore_val_233, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_234[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[234], restore_val_234, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_235[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[235], restore_val_235, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_236[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[236], restore_val_236, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_237[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[237], restore_val_237, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_238[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[238], restore_val_238, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_239[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[239], restore_val_239, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_240[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[240], restore_val_240, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_241[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[241], restore_val_241, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_242[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[242], restore_val_242, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_243[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[243], restore_val_243, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_244[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[244], restore_val_244, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_245[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[245], restore_val_245, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_246[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[246], restore_val_246, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_247[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[247], restore_val_247, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_248[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[248], restore_val_248, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_249[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[249], restore_val_249, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_250[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[250], restore_val_250, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_251[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[251], restore_val_251, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_252[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[252], restore_val_252, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_253[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[253], restore_val_253, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_254[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[254], restore_val_254, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	CVI_U16 restore_val_255[ISP_AUTO_ISO_STRENGTH_NUM] = { 128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128 };
	memcpy(attr.stAuto.YRatioLut[255], restore_val_255, sizeof(CVI_U16) * ISP_AUTO_ISO_STRENGTH_NUM);
	ret = CVI_ISP_SetCAAttr(ViPipe, &attr);
	if (ret != CVI_SUCCESS) {
		TEST_FAIL("restore default value failed\n");
		return ret;
	}
	return CVI_SUCCESS;
}

static void __attribute__((constructor)) reg_test_CA_CAAttr_YRatioLut_auto(void)
{
	reg_api_test_fn(test_CA_CAAttr_YRatioLut_auto);
}
