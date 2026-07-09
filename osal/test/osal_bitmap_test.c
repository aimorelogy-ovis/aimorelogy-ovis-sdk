#include <osal_test.h>

void osal_bitmap_test(void)
{
	unsigned long bitmap[2] = {0};
	int bit, result;
	int total_tests = 12; // total test nums
	int failed_index = 0;
	const char *failed_tests[13] = {0}; // array size is 1 more than total test nums

	osal_test_log("Testing bitmap operations...\n");

	// test osal_bitmap_set_bit
	osal_bitmap_set_bit(5, bitmap);
	result = osal_bitmap_test_bit(5, bitmap);
	osal_test_log("osal_bitmap_set_bit: %d\n", result);
	if (result != 1)
		failed_tests[failed_index++] = "osal_bitmap_set_bit";

	// test osal_bitmap_clear_bit
	osal_test_log("Testing osal_bitmap_clear_bit...\n");
	osal_bitmap_clear_bit(5, bitmap);
	result = osal_bitmap_test_bit(5, bitmap);
	osal_test_log("osal_bitmap_clear_bit: %d\n", result);
	if (result != 0)
		failed_tests[failed_index++] = "osal_bitmap_clear_bit";

	// test osal_bitmap_change_bit
	osal_test_log("Testing osal_bitmap_change_bit...\n");
	osal_bitmap_change_bit(3, bitmap);
	result = osal_bitmap_test_bit(3, bitmap);
	osal_test_log("osal_bitmap_change_bit (set): %d\n", result);
	if (result != 1)
		failed_tests[failed_index++] = "osal_bitmap_change_bit (set)";

	// test osal_bitmap_change_bit (clear)
	osal_bitmap_change_bit(3, bitmap);
	result = osal_bitmap_test_bit(3, bitmap);
	osal_test_log("osal_bitmap_change_bit (clear): %d\n", result);
	if (result != 0)
		failed_tests[failed_index++] = "osal_bitmap_change_bit (clear)";

	// test osal_bitmap_test_and_set_bit
	osal_test_log("Testing osal_bitmap_test_and_set_bit...\n");
	result = osal_bitmap_test_and_set_bit(7, bitmap);
	osal_test_log("osal_bitmap_test_and_set_bit: %d\n", result);
	if (!(result == 0 && osal_bitmap_test_bit(7, bitmap) == 1))
		failed_tests[failed_index++] = "osal_bitmap_test_and_set_bit";

	// test osal_bitmap_test_and_clear_bit
	osal_test_log("Testing osal_bitmap_test_and_clear_bit...\n");
	result = osal_bitmap_test_and_clear_bit(7, bitmap);
	osal_test_log("osal_bitmap_test_and_clear_bit: %d\n", result);
	if (!(result == 1 && osal_bitmap_test_bit(7, bitmap) == 0))
		failed_tests[failed_index++] = "osal_bitmap_test_and_clear_bit";

	// test osal_bitmap_test_and_change_bit
	osal_test_log("Testing osal_bitmap_test_and_change_bit...\n");
	result = osal_bitmap_test_and_change_bit(10, bitmap);
	osal_test_log("osal_bitmap_test_and_change_bit (set): %d\n", result);
	if (!(result == 0 && osal_bitmap_test_bit(10, bitmap) == 1))
		failed_tests[failed_index++] = "osal_bitmap_test_and_change_bit (set)";

	// test osal_bitmap_test_and_change_bit (clear)
	result = osal_bitmap_test_and_change_bit(10, bitmap);
	osal_test_log("osal_bitmap_test_and_change_bit (clear): %d\n", result);
	if (!(result == 1 && osal_bitmap_test_bit(10, bitmap) == 0))
		failed_tests[failed_index++] = "osal_bitmap_test_and_change_bit (clear)";

	// test osal_bitmap_find_first_zero_bit
	osal_test_log("Testing osal_bitmap_find_first_zero_bit...\n");
	bit = osal_bitmap_find_first_zero_bit(bitmap, sizeof(bitmap) * 8);
	osal_test_log("osal_bitmap_find_first_zero_bit: %d\n", bit);
	if (bit != 0)
		failed_tests[failed_index++] = "osal_bitmap_find_first_zero_bit";

	// test osal_bitmap_find_first_bit
	osal_test_log("Testing osal_bitmap_find_first_bit...\n");
	osal_bitmap_set_bit(15, bitmap);
	bit = osal_bitmap_find_first_bit(bitmap, sizeof(bitmap) * 8);
	osal_test_log("osal_bitmap_find_first_bit: %d\n", bit);
	if (bit != 15)
		failed_tests[failed_index++] = "osal_bitmap_find_first_bit";

	// test osal_bitmap_find_next_zero_bit
	osal_test_log("Testing osal_bitmap_find_next_zero_bit...\n");
	bit = osal_bitmap_find_next_zero_bit(bitmap, sizeof(bitmap) * 8, 15);
	osal_test_log("osal_bitmap_find_next_zero_bit: %d\n", bit);
	if (bit != 16)
		failed_tests[failed_index++] = "osal_bitmap_find_next_zero_bit";

	// test osal_bitmap_find_next_bit
	osal_test_log("Testing osal_bitmap_find_next_bit...\n");
	bit = osal_bitmap_find_next_bit(bitmap, sizeof(bitmap) * 8, 10);
	osal_test_log("osal_bitmap_find_next_bit: %d\n", bit);
	if (bit != 15)
		failed_tests[failed_index++] = "osal_bitmap_find_next_bit";

	// output test results
	if (failed_index == 0) {
		osal_test_log("[success] All bitmap tests(set, clear, change, test_and_set, test_and_clear, test_and_change, find_first_zero_bit, find_first_bit, find_next_zero_bit, find_next_bit) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}
}
OSAL_CMD_EXPORT(osal_bitmap_test, Test bitmap operations);