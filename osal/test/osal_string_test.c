#include <osal_test.h>

#define TEST_NUM 35

void osal_string_test(void)
{
	int total_tests = TEST_NUM; // Total number of tests
	int failed_index = 0;
	const char *failed_tests[TEST_NUM] = { 0 }; // Store names of failed tests

	osal_test_log("Starting osal_string test...\n");

	/* test osal_memncmp */
	{
		char buf1[] = "hello world";
		char buf2[] = "hello RT-Thread";
		int result = osal_memncmp(buf1, buf2, 5);
		if (result != 0) {
			failed_tests[failed_index++] =
				"osal_memncmp(buf1, buf2, 5)";
			osal_test_log(
				"[failed] osal_memncmp(buf1, buf2, 5): actual = %d, expected = 0\n",
				result);
		}

		result = osal_memncmp(buf1, buf2, 7);
		if (result == 0) {
			failed_tests[failed_index++] =
				"osal_memncmp(buf1, buf2, 7)";
			osal_test_log(
				"[failed] osal_memncmp(buf1, buf2, 7): actual = %d, expected != 0\n",
				result);
		}
	}

	/* test osal_strcmp */
	{
		const char *s1 = "hello";
		const char *s2 = "hello";
		const char *s3 = "world";
		int result = osal_strcmp(s1, s2);
		if (result != 0) {
			failed_tests[failed_index++] = "osal_strcmp(s1, s2)";
			osal_test_log(
				"[failed] osal_strcmp(s1, s2): actual = %d, expected = 0\n",
				result);
		}

		result = osal_strcmp(s1, s3);
		if (result == 0) {
			failed_tests[failed_index++] = "osal_strcmp(s1, s3)";
			osal_test_log(
				"[failed] osal_strcmp(s1, s3): actual = %d, expected != 0\n",
				result);
		}
	}

	/* test osal_strncmp */
	{
		const char *s1 = "hello";
		const char *s2 = "helix";
		int result = osal_strncmp(s1, s2, 3);
		if (result != 0) {
			failed_tests[failed_index++] =
				"osal_strncmp(s1, s2, 3)";
			osal_test_log(
				"[failed] osal_strncmp(s1, s2, 3): actual = %d, expected = 0\n",
				result);
		}

		result = osal_strncmp(s1, s2, 5);
		if (result == 0) {
			failed_tests[failed_index++] =
				"osal_strncmp(s1, s2, 5)";
			osal_test_log(
				"[failed] osal_strncmp(s1, s2, 5): actual = %d, expected != 0\n",
				result);
		}
	}

	/* test osal_strcasecmp */
	{
		const char *s1 = "Hello";
		const char *s2 = "hello";
		int result = osal_strcasecmp(s1, s2);
		if (result != 0) {
			failed_tests[failed_index++] =
				"osal_strcasecmp(s1, s2)";
			osal_test_log(
				"[failed] osal_strcasecmp(s1, s2): actual = %d, expected = 0\n",
				result);
		}
	}

	/* test osal_strncasecmp */
	{
		const char *s1 = "Hello";
		const char *s2 = "HELLO";
		const char *s3 = "HELLO_WORLD";
		int result = osal_strncasecmp(s1, s2, 5);
		if (result != 0) {
			failed_tests[failed_index++] =
				"osal_strncasecmp(s1, s2, 5)";
			osal_test_log(
				"[failed] osal_strncasecmp(s1, s2, 5): actual = %d, expected = 0\n",
				result);
		}

		result = osal_strncasecmp(s1, s3, 7);
		if (result == 0) {
			failed_tests[failed_index++] =
				"osal_strncasecmp(s1, s3, 7)";
			osal_test_log(
				"[failed] osal_strncasecmp(s1, s3, 7): actual = %d, expected != 0\n",
				result);
		}
	}

	/* test osal_strchr */
	{
		const char *s = "hello world";
		char *result = osal_strchr(s, 'o');
		if (result != s + 4) {
			failed_tests[failed_index++] = "osal_strchr(s, 'o')";
			osal_test_log(
				"[failed] osal_strchr(s, 'o'): actual = %p, expected = %p\n",
				result, s + 4);
		}

		result = osal_strchr(s, 'z');
		if (result != NULL) {
			failed_tests[failed_index++] = "osal_strchr(s, 'z')";
			osal_test_log(
				"[failed] osal_strchr(s, 'z'): actual = %p, expected = NULL\n",
				result);
		}
	}

	/* test osal_strnchr */
	{
		const char *s = "hello world";
		char *result = osal_strnchr(s, 5, 'o');
		if (result != s + 4) {
			failed_tests[failed_index++] =
				"osal_strnchr(s, 5, 'o')";
			osal_test_log(
				"[failed] osal_strnchr(s, 5, 'o'): actual = %p, expected = %p\n",
				result, s + 4);
		}

		result = osal_strnchr(s, 8, 'o');
		if (result != s + 4) {
			failed_tests[failed_index++] =
				"osal_strnchr(s, 8, 'o')";
			osal_test_log(
				"[failed] osal_strnchr(s, 8, 'o'): actual = %p, expected = %p\n",
				result, s + 4);
		}
	}

	/* test osal_strrchr */
	{
		const char *s = "hello world";
		char *result = osal_strrchr(s, 'o');
		if (result != s + 7) {
			failed_tests[failed_index++] = "osal_strrchr(s, 'o')";
			osal_test_log(
				"[failed] osal_strrchr(s, 'o'): actual = %p, expected = %p\n",
				result, s + 7);
		}

		result = osal_strrchr(s, 'z');
		if (result != NULL) {
			failed_tests[failed_index++] = "osal_strrchr(s, 'z')";
			osal_test_log(
				"[failed] osal_strrchr(s, 'z'): actual = %p, expected = NULL\n",
				result);
		}
	}

	/* test osal_strstr */
	{
		const char *s1 = "hello world";
		const char *s2 = "world";
		const char *s3 = "RT-Thread";
		char *result = osal_strstr(s1, s2);
		if (result != s1 + 6) {
			failed_tests[failed_index++] = "osal_strstr(s1, s2)";
			osal_test_log(
				"[failed] osal_strstr(s1, s2): actual = %p, expected = %p\n",
				result, s1 + 6);
		}

		result = osal_strstr(s1, s3);
		if (result != NULL) {
			failed_tests[failed_index++] = "osal_strstr(s1, s3)";
			osal_test_log(
				"[failed] osal_strstr(s1, s3): actual = %p, expected = NULL\n",
				result);
		}
	}

	/* test osal_strnstr */
	{
		const char *haystack = "hello, world!";
		const char *needle = "hello";
		size_t len = 10;

		char *result = osal_strnstr(haystack, needle, len);

		if (!result) {
			failed_tests[failed_index++] =
				"osal_strnstr(haystack, needle, len)";
			osal_test_log(
				"'%s' not found in the first %zu characters of '%s'.\n",
				needle, len, haystack);
		}
	}

	/* test osal_strlen */
	{
		const char *s1 = "hello world";
		int n = osal_strlen(s1);
		if (n != 11) {
			failed_tests[failed_index++] = "osal_strlen(s1)";
			osal_test_log("[fail] osal_strlen\n");
		}
	}

	/* test osal_strnlen */
	{
		const char *s1 = "hello world";
		size_t len = 100;
		int n = osal_strnlen(s1, len);
		if (n != 11) {
			failed_tests[failed_index++] = "osal_strnlen(s1, len)";
			osal_test_log("[fail] osal_strlen\n");
		}
	}

	/* test osal_strpbrk */
	{
		const char *str1 = "hello, world";
		const char *str2 = "aeiou";

		char *result = osal_strpbrk(str1, str2);

		if (!result) {
			failed_tests[failed_index++] =
				"osal_strpbrk(str1, str2)";
			osal_test_log("[fail] osal_strpbrk\n");
		}
	}

	/* test osal_strsep */
	{
		char str[] = "hello,world,this,is,C";
		char *delim = ",";
		char *ptr = str;
		char *token;
		char *expected[] = { "hello", "world", "this", "is", "C" };
		int i = 0;

		while ((token = osal_strsep(&ptr, delim)) != NULL) {
			if (osal_strcmp(token, expected[i]) != 0) {
				failed_tests[failed_index++] = "osal_strsep token comparison";
				osal_test_log("[fail] osal_strsep: token[%d] expected '%s', got '%s'\n",
							 i, expected[i], token);
				break;
			}
			i++;
		}

		if (i != 5) {
			failed_tests[failed_index++] = "osal_strsep(&ptr, delim)";
			osal_test_log("[fail] osal_strsep: expected 5 tokens, got %d tokens\n", i);
		}
	}

	/* test osal_strspn */
	{
		const char *s1 = "abcde12345";
		const char *accept1 = "abcde";
		const char *s3 = "12345";
		const char *accept3 = "abc";
		size_t result1 = osal_strspn(s1, accept1);
		size_t result3 = osal_strspn(s3, accept3);
		if (result1 != 5) {
			failed_tests[failed_index++] = "strspn(s1, accept1)";
			osal_test_log("[fail] osal_strspn\n");
		}

		if (result3 != 0) {
			failed_tests[failed_index++] = "strspn(s1, accept1)";
			osal_test_log("[fail] osal_strspn\n");
		}
	}

	/* test osal_memcmp */
	{
		char buffer1[] = "abcdef";
		char buffer2[] = "abcdef";
		char buffer3[] = "abcdee";
		char buffer4[] = "abcdef";
		char buffer5[] = "abcdeg";
		char buffer6[] = "abcdef";
		size_t len = 6;
		if (osal_memcmp(buffer1, buffer2, len) != 0) {
			failed_tests[failed_index++] =
				"osal_memcmp(buffer1, buffer2, len)";
			osal_test_log("[fail] osal_memcmp\n");
		}

		if (osal_memcmp(buffer3, buffer4, len) >= 0) {
			failed_tests[failed_index++] =
				"osal_memcmp(buffer1, buffer2, len)";
			osal_test_log("[fail] osal_memcmp\n");
		}

		if (osal_memcmp(buffer5, buffer6, len) < 0) {
			failed_tests[failed_index++] =
				"osal_memcmp(buffer1, buffer2, len)";
			osal_test_log("[fail] osal_memcmp\n");
		}
	}

	/* test osal_memchr */
	{
		char buffer1[] = "abcdef";
		size_t len = 6;
		char *result1 = osal_memchr(buffer1, 'c', len);
		if (*result1 != 'c') {
			failed_tests[failed_index++] =
				"memchr(buffer1, 'c', len)";
			osal_test_log("[fail] osal_memchr\n");
		}
	}

	/* test osal_memchr_inv */
	{
		char buffer1[] = { 0x00, 0x00, 0x01, 0x00 };
		size_t len = 4;
		void *result1 = osal_memchr_inv(buffer1, 0x00, len);

		if (result1 == NULL) {
			failed_tests[failed_index++] = "osal_memchr_inv result NULL check";
			osal_test_log("[fail] osal_memchr_inv: result is NULL, expected non-NULL\n");
		} else if (*(unsigned char *)result1 != 0x01) {
			failed_tests[failed_index++] = "osal_memchr_inv(buffer1, 0x00, len)";
			osal_test_log("[fail] osal_memchr_inv: expected 0x01, got 0x%02x\n",
						 *(unsigned char *)result1);
		}
	}

	/* test osal_strtoull */
	{
		char *endptr;

		const char *str1 = "12345";
		size_t len = 10;
		unsigned long long result1 = osal_strtoull(str1, &endptr, len);
		if (result1 != 12345 || *endptr != '\0') {
			failed_tests[failed_index++] =
				"osal_strtoull(str1, &endptr, len)";
			osal_test_log("[fail] osal_strtoull\n");
		}
	}

	/* test osal_strtoul */
	{
		char *endptr;

		const char *str1 = "12345";
		size_t len = 10;
		unsigned long result1 = osal_strtoul(str1, &endptr, len);
		if (result1 != 12345 || *endptr != '\0') {
			failed_tests[failed_index++] =
				"osal_strtoul(str1, &endptr, len)";
			osal_test_log("[fail] osal_strtoul\n");
		}
	}

	/* test osal_srtol */
	{
		char *endptr;
		long result;
		size_t len = 10;

		result = osal_strtol("12345", &endptr, len);
		if (result != 12345 || *endptr != '\0') {
			failed_tests[failed_index++] =
				"osal_srtol(str1, &endptr, len)";
			osal_test_log("[fail] osal_srtol\n");
		}
	}

	/* test osal_strtoll*/
	{
		char *endptr;
		long long result;
		size_t len = 10;

		result = osal_strtoll("123456789", &endptr, len);
		if (result != 123456789LL || *endptr != '\0') {
			failed_tests[failed_index++] =
				"osal_strtoll(\"123456789\", &endptr, len)";
			osal_test_log("[fail] osal_strtoll\n");
		}
	}

	/* test osal_strcpy */
	{
		char dest[100];
		char *result;

		result = osal_strcpy(dest, "Hello, World!");
		if (osal_strcmp(dest, "Hello, World!") != 0 || result != dest) {
			failed_tests[failed_index++] =
				"osal_strcpy(dest, \"Hello, World!\")";
			osal_test_log("[fail] osal_strcpy\n");
		}
	}

	/* test osal_strncpy */
	{
		char dest[100] = { 0 };
		char *result;
		size_t len = 10;

		result = osal_strncpy(dest, "Hello", len);
		if ((osal_strcmp(dest, "Hello") != 0) || result != dest) {
			failed_tests[failed_index++] =
				"osal_strcmp(dest, \"Hello\")";
			osal_test_log("[fail] osal_strncpy\n");
		}
	}
	/* test */
	if (failed_index == 0) {
		osal_test_log("[success] All string operation tests passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index,
			      total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_string test completed.\n");
}
OSAL_CMD_EXPORT(osal_string_test, Test string operations);