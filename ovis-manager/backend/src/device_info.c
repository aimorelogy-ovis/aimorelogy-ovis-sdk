#include "ovis_manager.h"

#include <ctype.h>
#include <stdint.h>
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

int device_private_network_id(char *identity, size_t size)
{
	char device_id[128];
	uint64_t hash = UINT64_C(14695981039346656037);
	unsigned char bytes[6];
	size_t index;
	int written;

	if (identity == NULL || size < sizeof("00:00:00:00:00:00"))
		return -1;

	read_device_id(device_id, sizeof(device_id));
	for (index = 0; device_id[index] != '\0'; index++) {
		hash ^= (unsigned char)device_id[index];
		hash *= UINT64_C(1099511628211);
	}
	for (index = 0; index < sizeof(bytes); index++)
		bytes[index] = (unsigned char)(hash >> ((sizeof(bytes) - 1 - index) * 8));

	/* Use a locally administered unicast identity derived from the persistent
	 * device ID. It remains stable across reboots without claiming an OUI. */
	bytes[0] = (unsigned char)((bytes[0] | 0x02U) & 0xfeU);
	written = snprintf(identity, size, "%02X:%02X:%02X:%02X:%02X:%02X",
		bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
	return written == 17 ? 0 : -1;
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
