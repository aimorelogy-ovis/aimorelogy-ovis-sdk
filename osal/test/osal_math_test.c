#include <osal_test.h>

void osal_math_test(void)
{
	unsigned long long u64_dividend = 1234567890123456789ULL;
	unsigned int u32_divisor = 12345;
	long long s64_dividend = -1234567890123456789LL;
	int s32_divisor = -12345;
	unsigned long long u64_large_divisor = 987654321987654321ULL;
	long long s64_large_divisor = -987654321987654321LL;

	unsigned long long u64_result;
	long long s64_result;
	unsigned long long u64_rem_result;
	long long s64_rem_result;
	unsigned int random_int;

	int total_tests = 8; // total test nums
	int failed_index = 0;
	const char *failed_tests[9] = {0}; // array size is 1 more than total test nums

	osal_test_log("Testing osal_div_* interfaces...\n");

	// test osal_div_u64
	osal_test_log("Testing osal_div_u64...\n");
	u64_result = osal_div_u64(u64_dividend, u32_divisor);
	if (u64_result != u64_dividend / u32_divisor)
		failed_tests[failed_index++] = "osal_div_u64";

	// test osal_div_s64
	osal_test_log("Testing osal_div_s64...\n");
	s64_result = osal_div_s64(s64_dividend, s32_divisor);
	if (s64_result != s64_dividend / s32_divisor)
		failed_tests[failed_index++] = "osal_div_s64";

	// test osal_div64_u64
	osal_test_log("Testing osal_div64_u64...\n");
	u64_result = osal_div64_u64(u64_dividend, u64_large_divisor);
	if (u64_result != u64_dividend / u64_large_divisor)
		failed_tests[failed_index++] = "osal_div64_u64";

	// test osal_div64_s64
	osal_test_log("Testing osal_div64_s64...\n");
	s64_result = osal_div64_s64(s64_dividend, s64_large_divisor);
	if (s64_result != s64_dividend / s64_large_divisor)
		failed_tests[failed_index++] = "osal_div64_s64";

	// test osal_div_u64_rem
	osal_test_log("Testing osal_div_u64_rem...\n");
	u64_rem_result = osal_div_u64_rem(u64_dividend, u32_divisor);
	if (u64_rem_result != u64_dividend % u32_divisor)
		failed_tests[failed_index++] = "osal_div_u64_rem";

	// test osal_div_s64_rem
	osal_test_log("Testing osal_div_s64_rem...\n");
	s64_rem_result = osal_div_s64_rem(s64_dividend, s32_divisor);
	if (s64_rem_result != s64_dividend % s32_divisor)
		failed_tests[failed_index++] = "osal_div_s64_rem";

	// test osal_div64_u64_rem
	osal_test_log("Testing osal_div64_u64_rem...\n");
	u64_rem_result = osal_div64_u64_rem(u64_dividend, u64_large_divisor);
	if (u64_rem_result != u64_dividend % u64_large_divisor)
		failed_tests[failed_index++] = "osal_div64_u64_rem";

	// test osal_get_random_int
	osal_test_log("Testing osal_get_random_int...\n");
	random_int = osal_get_random_int();
	if (random_int == 0) // assume 0 is invalid random number
		failed_tests[failed_index++] = "osal_get_random_int";

	// output test results
	if (failed_index == 0) {
		osal_test_log("[success] All math operation tests(osal_div_u64, osal_div_s64, osal_div64_u64, osal_div64_s64, osal_div_u64_rem, osal_div_s64_rem, osal_div64_u64_rem, osal_get_random_int) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_div_* and osal_get_random_int test completed.\n");
}
OSAL_CMD_EXPORT(osal_math_test, Test math operations);
