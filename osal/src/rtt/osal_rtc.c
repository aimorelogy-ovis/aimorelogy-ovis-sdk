#include <time.h>
#include <stdbool.h>
#include "osal.h"
#include "osal_inner.h"
#include "osal_rtc.h"

static void print_osal_rtc_time(const osal_rtc_time *tm)
{
	if (tm == NULL) {
		osal_log("osal_rtc_time is NULL\n");
		return;
	}

	osal_log("osal_rtc_time:\n");
	osal_log("  tm_sec   : %d\n", tm->tm_sec);
	osal_log("  tm_min   : %d\n", tm->tm_min);
	osal_log("  tm_hour  : %d\n", tm->tm_hour);
	osal_log("  tm_mday  : %d\n", tm->tm_mday);
	osal_log("  tm_mon   : %d (1-based, Jan=1)\n", tm->tm_mon);
	osal_log("  tm_year  : %d\n", tm->tm_year);
	osal_log("  tm_wday  : %d (0=Sunday, 6=Saturday)\n", tm->tm_wday);
	osal_log("  tm_yday  : %d (0=Jan 1)\n", tm->tm_yday);
	osal_log("  tm_isdst : %d\n", tm->tm_isdst);
}

static void print_tm(const struct tm *timeval) {
    if (timeval == NULL) {
        osal_log("Invalid timeval pointer!\n");
        return;
    }

    osal_log("struct tm:\n");
    osal_log("  tm_sec   (seconds)       : %d\n", timeval->tm_sec);
    osal_log("  tm_min   (minutes)       : %d\n", timeval->tm_min);
    osal_log("  tm_hour  (hours)         : %d\n", timeval->tm_hour);
    osal_log("  tm_mday  (day of month)  : %d\n", timeval->tm_mday);
    osal_log("  tm_mon   (month)         : %d (0 = January)\n", timeval->tm_mon );
    osal_log("  tm_year  (year)          : %d (since 1900, actual year = %d)\n",
           timeval->tm_year, timeval->tm_year + 1900);
    osal_log("  tm_wday  (day of week)   : %d (0 = Sunday)\n", timeval->tm_wday);
    osal_log("  tm_yday  (day of year)   : %d (0 = January 1st)\n", timeval->tm_yday);
    osal_log("  tm_isdst (DST flag)      : %d (1 = DST, 0 = no DST, -1 = unknown)\n",
           timeval->tm_isdst);
}

void osal_rtc_time_to_tm(unsigned long time, osal_rtc_time *tm)
{
	if (tm == NULL) {
		return;
	}

	time_t time2change = (time_t)time;
	struct tm *timeval;
	timeval = gmtime(&time2change); // get UTC time
	tm->tm_year = timeval->tm_year;
	tm->tm_mon = timeval->tm_mon;
	tm->tm_wday = timeval->tm_wday;
	tm->tm_yday = timeval->tm_yday;
	tm->tm_mday = timeval->tm_mday;
	tm->tm_hour = timeval->tm_hour;
	tm->tm_min = timeval->tm_min;
	tm->tm_sec = timeval->tm_sec;
	tm->tm_isdst = timeval->tm_isdst;
	/* time correction */
	tm->tm_year += 1900;
	tm->tm_mon += 1;
}
// EXPORT_SYMBOL(osal_rtc_time_to_tm);

void osal_rtc_tm_to_time(const osal_rtc_time *tm, unsigned long *time)
{
	if (tm == NULL || time == NULL) {
		return;
	}

	// Convert osal_rtc_time to struct tm
	struct tm t;
	t.tm_sec = tm->tm_sec;
	t.tm_min = tm->tm_min;
	t.tm_hour = tm->tm_hour;
	t.tm_mday = tm->tm_mday;
	t.tm_mon = tm->tm_mon - 1;
	t.tm_year = tm->tm_year -
		    1900; // Convert from full year to years since 1900
    t.tm_wday = tm->tm_wday;
    t.tm_yday = tm->tm_yday;
	t.tm_isdst = tm->tm_isdst;

	// Convert struct tm to timestamp
	time_t result = timegm(&t);
	if (result == -1) {
		*time = 0; // Invalid timestamp
		return;
	}

	*time = (unsigned long)result;
}
// EXPORT_SYMBOL(osal_rtc_tm_to_time);

int osal_rtc_valid_tm(const osal_rtc_time *tm)
{
	if (tm == NULL) {
		return OSAL_FAILURE;
	}

	// Check ranges of each field
	if (tm->tm_sec < 0 || tm->tm_sec > 59) {
		return OSAL_FAILURE;
	}
	if (tm->tm_min < 0 || tm->tm_min > 59) {
		return OSAL_FAILURE;
	}
	if (tm->tm_hour < 0 || tm->tm_hour > 23) {
		return OSAL_FAILURE;
	}
	if (tm->tm_mday < 1 || tm->tm_mday > 31) {
		return OSAL_FAILURE;
	}
	if (tm->tm_mon < 0 || tm->tm_mon > 11) {
		return OSAL_FAILURE;
	}
	if (tm->tm_year < 1970) { // Year must be >= 1970
		return OSAL_FAILURE;
	}

	// Additional checks for day of the month
	struct tm t;
	t.tm_sec = tm->tm_sec;
	t.tm_min = tm->tm_min;
	t.tm_hour = tm->tm_hour;
	t.tm_mday = tm->tm_mday;
	t.tm_mon = tm->tm_mon;
	t.tm_year = tm->tm_year - 1900;
	t.tm_mon = tm->tm_mon - 1;
	t.tm_yday = tm->tm_yday;
	t.tm_wday = tm->tm_wday;
	t.tm_isdst = -1; // Let the system determine DST

	if (timegm(&t) == -1) {
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_rtc_valid_tm);