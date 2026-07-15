#include "ovis_manager.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int device_id_is_valid(const char *device_id)
{
	size_t length;
	size_t index;

	if (device_id == NULL || strncmp(device_id, "OVIS-", 5) != 0)
		return 0;
	length = strlen(device_id);
	if (length < 13 || length >= 128)
		return 0;
	for (index = 0; index < length; index++) {
		unsigned char value = (unsigned char)device_id[index];

		if (!isalnum(value) && value != '-')
			return 0;
	}
	return 1;
}

static void read_device_id(char *device_id, size_t size)
{
	FILE *file;

	snprintf(device_id, size, "%s", "OVIS-1842-UNPROVISIONED");
	file = fopen(OVIS_DEVICE_ID_FILE, "r");
	if (file == NULL)
		return;
	if (fgets(device_id, (int)size, file) != NULL) {
		device_id[strcspn(device_id, "\r\n")] = '\0';
		if (!device_id_is_valid(device_id))
			snprintf(device_id, size, "%s", "OVIS-1842-UNPROVISIONED");
	}
	fclose(file);
}

int device_info_json(char *json, size_t size)
{
	char device_id[128];
	char escaped_id[256];
	int written;

	if (json == NULL || size == 0)
		return -1;
	read_device_id(device_id, sizeof(device_id));
	json_escape(device_id, escaped_id, sizeof(escaped_id));
	written = snprintf(json, size,
		"{\"protocol\":\"ovis-device\",\"api_version\":%d,"
		"\"device_id\":\"%s\",\"name\":\"OVIS Camera\","
		"\"model\":\"OVIS\",\"serial\":\"%s\","
		"\"firmware_version\":\"%s\",\"manager_version\":\"%s\"}",
		OVIS_API_VERSION, escaped_id, escaped_id,
		OVIS_FIRMWARE_VERSION, OVIS_VERSION);
	return written >= 0 && (size_t)written < size ? 0 : -1;
}
