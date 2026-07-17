#include "ovis_manager.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int base64_value(char value)
{
	if (value >= 'A' && value <= 'Z') return value - 'A';
	if (value >= 'a' && value <= 'z') return value - 'a' + 26;
	if (value >= '0' && value <= '9') return value - '0' + 52;
	if (value == '+') return 62;
	if (value == '/') return 63;
	return -1;
}

static int base64_decode(const char *input, char *output, size_t size)
{
	unsigned int accumulator = 0;
	int bits = 0;
	size_t used = 0;

	while (*input && *input != '=') {
		int value = base64_value(*input++);
		if (value < 0) return -1;
		accumulator = (accumulator << 6) | (unsigned int)value;
		bits += 6;
		if (bits >= 8) {
			bits -= 8;
			if (used + 1 >= size) return -1;
			output[used++] = (char)((accumulator >> bits) & 0xff);
		}
	}
	output[used] = '\0';
	return 0;
}

int auth_check(const char *authorization)
{
	char decoded[256];
	char expected[256];
	char username[128];
	char password[128];
	FILE *file;

	if (strncmp(authorization, "Basic ", 6) != 0 || base64_decode(authorization + 6, decoded, sizeof(decoded)) != 0)
		return 0;
	file = fopen(OVIS_ACCOUNT_FILE, "r");
	if (file == NULL) return 0;
	if (fscanf(file, "username=%127[^\n]\npassword=%127[^\n]", username, password) != 2) {
		fclose(file);
		return 0;
	}
	fclose(file);
	if (strchr(username, ':') != NULL || strchr(password, ':') != NULL)
		return 0;
	snprintf(expected, sizeof(expected), "%s:%s", username, password);
	return strlen(decoded) == strlen(expected) && memcmp(decoded, expected, strlen(expected)) == 0;
}

void audit_log(const char *operation, const char *result)
{
	time_t now = time(NULL);
	struct tm timestamp;
	char text[32];
	FILE *file;

	localtime_r(&now, &timestamp);
	strftime(text, sizeof(text), "%Y-%m-%dT%H:%M:%S%z", &timestamp);
	file = fopen(OVIS_AUDIT_LOG, "a");
	if (file == NULL) return;
	fprintf(file, "%s operation=%s result=%s\n", text, operation, result);
	fflush(file);
	fclose(file);
}

void json_escape(const char *src, char *dst, size_t size)
{
	size_t used = 0;
	while (*src && used + 2 < size) {
		unsigned char value = (unsigned char)*src++;
		if (value == '"' || value == '\\') {
			dst[used++] = '\\'; dst[used++] = (char)value;
		} else if (value == '\n' || value == '\r' || value == '\t') {
			dst[used++] = '\\'; dst[used++] = value == '\n' ? 'n' : value == '\r' ? 'r' : 't';
		} else if (value >= 0x20) {
			dst[used++] = (char)value;
		}
	}
	dst[used] = '\0';
}
