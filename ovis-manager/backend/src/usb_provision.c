#include "ovis_manager.h"

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/usb/ch9.h>
#include <linux/usb/functionfs.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define OVIS_WEBUSB_INTERFACE_SUBCLASS 0x4f
#define OVIS_WEBUSB_INTERFACE_PROTOCOL 0x01
#define OVIS_WEBUSB_REQUEST_GET_INFO 0x01
#define OVIS_WEBUSB_REQUEST_QUIESCE_NCM 0x02
#define OVIS_WEBUSB_REQUEST_SET_SUBNET 0x03
#define OVIS_WEBUSB_REQUEST_COMMIT 0x04
#define OVIS_WEBUSB_REQUEST_ABORT 0x05
#define OVIS_WEBUSB_MAX_RESPONSE 128
#define OVIS_WEBUSB_ACTION_SCRIPT "/etc/init.d/S77ncm"

struct ovis_webusb_descriptors {
	struct usb_functionfs_descs_head_v2 header;
	__le32 fs_count;
	__le32 hs_count;
	__le32 os_count;
	struct usb_interface_descriptor fs_interface;
	struct usb_endpoint_descriptor_no_audio fs_out;
	struct usb_endpoint_descriptor_no_audio fs_in;
	struct usb_interface_descriptor hs_interface;
	struct usb_endpoint_descriptor_no_audio hs_out;
	struct usb_endpoint_descriptor_no_audio hs_in;
	struct usb_os_desc_header os_header;
	struct usb_ext_compat_desc os_compat;
} __attribute__((packed));

struct ovis_webusb_strings {
	struct usb_functionfs_strings_head header;
	__le16 language;
	char interface_name[sizeof("OVIS WebUSB Control")];
} __attribute__((packed));

static int write_full(int fd, const void *buffer, size_t size)
{
	const unsigned char *data = buffer;

	while (size > 0) {
		ssize_t written = write(fd, data, size);
		if (written < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (written == 0)
			return -1;
		data += written;
		size -= (size_t)written;
	}
	return 0;
}

static int read_device_id(char *device_id, size_t size)
{
	FILE *file = fopen(OVIS_DEVICE_ID_FILE, "r");

	if (file == NULL)
		return -1;
	if (fgets(device_id, (int)size, file) == NULL) {
		fclose(file);
		return -1;
	}
	fclose(file);
	device_id[strcspn(device_id, "\r\n")] = '\0';
	return strncmp(device_id, "OVIS-1842-", 10) == 0 ? 0 : -1;
}

static int read_subnet(const char *path)
{
	char line[16];
	char *end;
	long value;
	FILE *file = fopen(path, "r");

	if (file == NULL)
		return -1;
	if (fgets(line, sizeof(line), file) == NULL) {
		fclose(file);
		return -1;
	}
	fclose(file);
	errno = 0;
	value = strtol(line, &end, 10);
	while (*end == '\r' || *end == '\n' || *end == ' ' || *end == '\t')
		end++;
	return errno == 0 && *end == '\0' && value >= 0 && value <= 255 ?
		(int)value : -1;
}

static int write_pending_subnet(unsigned char subnet)
{
	char temporary[256];
	FILE *file;
	int result = 0;

	snprintf(temporary, sizeof(temporary), "%s.tmp",
		OVIS_NCM_PENDING_SUBNET_FILE);
	file = fopen(temporary, "w");
	if (file == NULL)
		return -1;
	if (fprintf(file, "%u\n", subnet) < 0 || fflush(file) != 0 ||
	    fsync(fileno(file)) != 0)
		result = -1;
	if (fclose(file) != 0)
		result = -1;
	if (result != 0 || chmod(temporary, 0600) != 0) {
		unlink(temporary);
		return -1;
	}
	if (rename(temporary, OVIS_NCM_PENDING_SUBNET_FILE) != 0) {
		unlink(temporary);
		return -1;
	}
	return 0;
}

static int sync_state_directory(void)
{
	int fd = open("/mnt/cfg/ovis-manager", O_RDONLY | O_CLOEXEC);
	int result;

	if (fd < 0)
		return -1;
	result = fsync(fd);
	if (close(fd) != 0)
		result = -1;
	return result;
}

static int commit_pending_subnet(void)
{
	if (read_subnet(OVIS_NCM_PENDING_SUBNET_FILE) < 0)
		return -1;
	if (rename(OVIS_NCM_PENDING_SUBNET_FILE, OVIS_NCM_SUBNET_FILE) != 0)
		return -1;
	if (sync_state_directory() != 0)
		perror("sync OVIS state directory");
	return 0;
}

static int run_action(const char *action, int delayed)
{
	pid_t pid;

	if (!delayed)
		return system(OVIS_WEBUSB_ACTION_SCRIPT " webusb-quiesce") == 0 ? 0 : -1;
	pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0) {
		struct timespec delay = { .tv_sec = 1, .tv_nsec = 0 };

		signal(SIGCHLD, SIG_DFL);
		nanosleep(&delay, NULL);
		execl(OVIS_WEBUSB_ACTION_SCRIPT, OVIS_WEBUSB_ACTION_SCRIPT, action,
			(char *)NULL);
		_exit(127);
	}
	return 0;
}

static int write_descriptors(int fd)
{
	struct ovis_webusb_descriptors descriptors;
	struct ovis_webusb_strings strings;

	memset(&descriptors, 0, sizeof(descriptors));
	descriptors.header.magic = htole32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2);
	descriptors.header.length = htole32(sizeof(descriptors));
	descriptors.header.flags = htole32(FUNCTIONFS_HAS_FS_DESC |
		FUNCTIONFS_HAS_HS_DESC | FUNCTIONFS_HAS_MS_OS_DESC);
	descriptors.fs_count = htole32(3);
	descriptors.hs_count = htole32(3);
	descriptors.os_count = htole32(1);
	descriptors.fs_interface.bLength = sizeof(descriptors.fs_interface);
	descriptors.fs_interface.bDescriptorType = USB_DT_INTERFACE;
	descriptors.fs_interface.bInterfaceNumber = 0;
	descriptors.fs_interface.bNumEndpoints = 2;
	descriptors.fs_interface.bInterfaceClass = USB_CLASS_VENDOR_SPEC;
	descriptors.fs_interface.bInterfaceSubClass = OVIS_WEBUSB_INTERFACE_SUBCLASS;
	descriptors.fs_interface.bInterfaceProtocol = OVIS_WEBUSB_INTERFACE_PROTOCOL;
	descriptors.fs_interface.iInterface = 1;
	descriptors.fs_out.bLength = sizeof(descriptors.fs_out);
	descriptors.fs_out.bDescriptorType = USB_DT_ENDPOINT;
	descriptors.fs_out.bEndpointAddress = USB_DIR_OUT | 1;
	descriptors.fs_out.bmAttributes = USB_ENDPOINT_XFER_BULK;
	descriptors.fs_out.wMaxPacketSize = htole16(64);
	descriptors.fs_in = descriptors.fs_out;
	descriptors.fs_in.bEndpointAddress = USB_DIR_IN | 1;
	descriptors.hs_interface = descriptors.fs_interface;
	descriptors.hs_out = descriptors.fs_out;
	descriptors.hs_out.wMaxPacketSize = htole16(512);
	descriptors.hs_in = descriptors.fs_in;
	descriptors.hs_in.wMaxPacketSize = htole16(512);
	descriptors.os_header.interface = 0;
	descriptors.os_header.dwLength = htole32(sizeof(descriptors.os_header) +
		sizeof(descriptors.os_compat));
	descriptors.os_header.bcdVersion = htole16(1);
	descriptors.os_header.wIndex = htole16(4);
	descriptors.os_header.bCount = 1;
	descriptors.os_compat.bFirstInterfaceNumber = 0;
	descriptors.os_compat.Reserved1 = 1;
	memcpy(descriptors.os_compat.CompatibleID, "WINUSB", sizeof("WINUSB") - 1);
	if (write_full(fd, &descriptors, sizeof(descriptors)) != 0)
		return -1;

	memset(&strings, 0, sizeof(strings));
	strings.header.magic = htole32(FUNCTIONFS_STRINGS_MAGIC);
	strings.header.length = htole32(sizeof(strings));
	strings.header.str_count = htole32(1);
	strings.header.lang_count = htole32(1);
	strings.language = htole16(0x0409);
	memcpy(strings.interface_name, "OVIS WebUSB Control",
		sizeof("OVIS WebUSB Control"));
	return write_full(fd, &strings, sizeof(strings));
}

static int send_info(int fd, unsigned short requested_length)
{
	char device_id[128];
	char response[OVIS_WEBUSB_MAX_RESPONSE];
	int subnet = read_subnet(OVIS_NCM_SUBNET_FILE);
	int pending_subnet = read_subnet(OVIS_NCM_PENDING_SUBNET_FILE);
	int length;

	if (read_device_id(device_id, sizeof(device_id)) != 0)
		return -1;
	length = snprintf(response, sizeof(response),
		"{\"protocol\":2,\"device_id\":\"%s\",\"subnet\":%d,"
		"\"pending_subnet\":%d,\"ncm_active\":%s}",
		device_id, subnet, pending_subnet,
		access(OVIS_NCM_ACTIVE_FILE, F_OK) == 0 ? "true" : "false");
	if (length < 0 || (size_t)length >= sizeof(response))
		return -1;
	if ((unsigned short)length > requested_length)
		length = requested_length;
	return write_full(fd, response, (size_t)length);
}

static int receive_command_data(int fd, unsigned short length,
	unsigned char *value)
{
	unsigned char buffer[64];
	ssize_t count;

	if (length == 0 || length > sizeof(buffer))
		return -1;
	do {
		count = read(fd, buffer, length);
	} while (count < 0 && errno == EINTR);
	if (count != length)
		return -1;
	*value = buffer[0];
	return 0;
}

static void handle_setup(int fd, const struct usb_ctrlrequest *setup)
{
	unsigned short length = le16toh(setup->wLength);
	unsigned char value = 0;
	int direction_in = (setup->bRequestType & USB_DIR_IN) != 0;

	if ((setup->bRequestType & USB_TYPE_MASK) != USB_TYPE_VENDOR ||
	    (setup->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
		return;
	if (direction_in) {
		if (setup->bRequest == OVIS_WEBUSB_REQUEST_GET_INFO)
			send_info(fd, length);
		return;
	}
	if (receive_command_data(fd, length, &value) != 0)
		return;
	switch (setup->bRequest) {
	case OVIS_WEBUSB_REQUEST_QUIESCE_NCM:
		run_action("webusb-quiesce", 0);
		break;
	case OVIS_WEBUSB_REQUEST_SET_SUBNET:
		if (write_pending_subnet(value) == 0) {
			fprintf(stderr, "OVIS WebUSB pending subnet: %u\n", value);
			fflush(stderr);
		} else {
			perror("write pending OVIS subnet");
		}
		break;
	case OVIS_WEBUSB_REQUEST_COMMIT:
		if (commit_pending_subnet() == 0) {
			fprintf(stderr, "OVIS WebUSB subnet committed\n");
			fflush(stderr);
			if (run_action("webusb-commit", 1) != 0)
				perror("schedule OVIS reboot");
		} else {
			perror("commit OVIS subnet");
		}
		break;
	case OVIS_WEBUSB_REQUEST_ABORT:
		unlink(OVIS_NCM_PENDING_SUBNET_FILE);
		break;
	default:
		break;
	}
}

int usb_provision_run(void)
{
	struct usb_functionfs_event events[8];
	int ready_fd;
	int fd;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	unlink(OVIS_WEBUSB_READY_FILE);
	fd = open(OVIS_WEBUSB_EP0, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		perror("open WebUSB ep0");
		return 1;
	}
	if (write_descriptors(fd) != 0) {
		perror("write WebUSB descriptors");
		close(fd);
		return 1;
	}
	ready_fd = open(OVIS_WEBUSB_READY_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (ready_fd < 0 || close(ready_fd) != 0) {
		perror("create WebUSB ready file");
		close(fd);
		return 1;
	}
	for (;;) {
		ssize_t count = read(fd, events, sizeof(events));
		size_t index;

		if (count < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		if (count == 0)
			break;
		for (index = 0; index < (size_t)count / sizeof(events[0]); index++) {
			if (events[index].type == FUNCTIONFS_SETUP)
				handle_setup(fd, &events[index].u.setup);
		}
	}
	unlink(OVIS_WEBUSB_READY_FILE);
	close(fd);
	return 1;
}
