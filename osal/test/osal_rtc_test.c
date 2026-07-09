#include <osal_rtc.h>
#include <osal_test.h>
#include <osal_addr.h>

void osal_rtc_test(void){

	unsigned long test_timestamp = 1681010400; // 2023-04-09 3:20:00 UTC Sunday
	unsigned long test_timestamp1;
	osal_rtc_time result;
	int test_failed = 0;
	osal_test_log("Running test_osal_rtc_time_to_tm...\n");

	osal_rtc_time_to_tm(test_timestamp, &result);

	if (result.tm_sec != 0) {
		osal_test_log("Case failed: tm_sec expected 0, got %d\n", result.tm_sec);
		test_failed = 1;
	}
	if (result.tm_min != 20) {
		osal_test_log("Case failed: tm_min expected 20, got %d\n", result.tm_min);
		test_failed = 1;
	}
	if (result.tm_hour != 3) {
		osal_test_log("Case failed: tm_hour expected 3, got %d\n", result.tm_hour);
		test_failed = 1;
	}
	if (result.tm_mday != 9) {
		osal_test_log("Case failed: tm_mday expected 9, got %d\n", result.tm_mday);
		test_failed = 1;
	}
	if (result.tm_mon != 4) {
		osal_test_log("Case failed: tm_mon expected 4, got %d\n", result.tm_mon);
		test_failed = 1;
	}
	if (result.tm_year != 2023) {
		osal_test_log("Case failed: tm_year expected 2023, got %d\n", result.tm_year);
		test_failed = 1;
	}
	if (result.tm_wday != 0) {
		osal_test_log("Case failed: tm_wday expected 0, got %d\n", result.tm_wday);
		test_failed = 1;
	}
	if (result.tm_isdst != 0) {
		osal_test_log("Case failed: tm_isdst expected 0, got %d\n", result.tm_isdst);
		test_failed = 1;
	}

    test_timestamp = 1681210430; // 2023-04-11 10:53:50 UTC Tuesday
    osal_memset(&result, 0, sizeof(result));
    osal_rtc_time_to_tm(test_timestamp, &result);

    if (result.tm_sec != 50) {
        osal_test_log("Case failed: tm_sec expected 50, got %d\n", result.tm_sec);
        test_failed = 1;
    }
    if (result.tm_min != 53) {
        osal_test_log("Case failed: tm_min expected 53, got %d\n", result.tm_min);
        test_failed = 1;
    }
    if (result.tm_hour != 10) {
        osal_test_log("Case failed: tm_hour expected 10, got %d\n", result.tm_hour);
        test_failed = 1;
    }
    if (result.tm_mday != 11) {
        osal_test_log("Case failed: tm_mday expected 11, got %d\n", result.tm_mday);
        test_failed = 1;
    }
    if (result.tm_mon != 4) {
        osal_test_log("Case failed: tm_mon expected 4, got %d\n", result.tm_mon);
        test_failed = 1;
    }
    if (result.tm_year != 2023) {
        osal_test_log("Case failed: tm_year expected 2023, got %d\n", result.tm_year);
        test_failed = 1;
    }
    if (result.tm_wday != 2) {
        osal_test_log("Case failed: tm_wday expected 2, got %d\n", result.tm_wday);
        test_failed = 1;
    }
    if (result.tm_isdst != 0) {
        osal_test_log("Case failed: tm_isdst expected 0, got %d\n", result.tm_isdst);
        test_failed = 1;
    }

	if (!test_failed) {
		osal_test_log("test_osal_rtc_time_to_tm passed!\n");
	}

    osal_rtc_tm_to_time(&result, &test_timestamp1);

    if (test_timestamp1 != test_timestamp) {
        osal_test_log("Case failed: timestamp expected %lu, got %lu\n", test_timestamp, test_timestamp1);
        test_failed = 1;
    } else {
        osal_test_log("test_osal_rtc_tm_to_time passed!\n");
    }

    if (osal_rtc_valid_tm(&result) == OSAL_SUCCESS) {
        osal_test_log("test_osal_rtc_valid_tm passed!\n");
    } else {
        osal_test_log("test_osal_rtc_valid_tm fail!\n");
        test_failed = 1;
    }

    if (test_failed) {
        osal_test_log("Some test cases failed!\n");
    } else {
        osal_test_log("All test cases passed!\n");
    }
}
OSAL_CMD_EXPORT(osal_rtc_test, Test rtc operations);