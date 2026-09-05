#ifndef OVIS_MANAGER_H
#define OVIS_MANAGER_H

#include <stddef.h>
#include <sys/types.h>

#define OVIS_VERSION "1.0.0"
#define OVIS_API_VERSION 1
#ifndef OVIS_DEFAULT_PORT
#define OVIS_DEFAULT_PORT 8080
#endif
#ifndef OVIS_BIND_ADDRESS
#define OVIS_BIND_ADDRESS "192.168.42.1"
#endif
#ifndef OVIS_BIND_ADDRESS_FILE
#define OVIS_BIND_ADDRESS_FILE "/var/run/ovis-ncm-address"
#endif
#ifndef OVIS_ALLOWED_ORIGIN
#define OVIS_ALLOWED_ORIGIN "https://ovis.aimorelogy.com"
#endif
#ifndef OVIS_DEV_ORIGIN
#define OVIS_DEV_ORIGIN "http://localhost:5173"
#endif
#ifndef OVIS_DEV_ORIGIN_ALT
#define OVIS_DEV_ORIGIN_ALT "http://127.0.0.1:5173"
#endif
#ifndef OVIS_FIRMWARE_VERSION
#define OVIS_FIRMWARE_VERSION "1.0.0"
#endif
#ifndef OVIS_PRIVATE_NETWORK_NAME
#define OVIS_PRIVATE_NETWORK_NAME "ovis-camera"
#endif
#define OVIS_MAX_REQUEST_SIZE (136 * 1024)
#define OVIS_MAX_BODY_SIZE (128 * 1024)
#ifndef OVIS_MODEL_DIR
#define OVIS_MODEL_DIR "/mnt/system/ovis-models"
#endif
#define OVIS_MODEL_IMPORT_DIR OVIS_MODEL_DIR "/.imports"
#define OVIS_MODEL_STORE_DIR OVIS_MODEL_DIR "/models"
#define OVIS_MODEL_MAX_FILE_SIZE (16 * 1024 * 1024)
#define OVIS_MODEL_RESERVED_BYTES (2 * 1024 * 1024)
#define OVIS_AI_MIN_WIDTH 160
#define OVIS_AI_MIN_HEIGHT 96
#define OVIS_AI_OBJECT_DEFAULT_WIDTH 448
#define OVIS_AI_OBJECT_DEFAULT_HEIGHT 256
#define OVIS_AI_OBJECT_FRAME_MAX_WIDTH 1920
#define OVIS_AI_OBJECT_FRAME_MAX_HEIGHT 1080
#define OVIS_AI_FACE_MAX_WIDTH 768
#define OVIS_AI_FACE_MAX_HEIGHT 432
#define OVIS_AI_MOTION_MAX_WIDTH 640
#define OVIS_AI_MOTION_MAX_HEIGHT 360
#define OVIS_AI_HUMAN_POSE_MAX_WIDTH 640
#define OVIS_AI_HUMAN_POSE_MAX_HEIGHT 384
#define OVIS_AI_TRACK_DET_WIDTH 640
#define OVIS_AI_TRACK_DET_HEIGHT 384
#define OVIS_AI_TRACK_SOT_WIDTH 1920
#define OVIS_AI_TRACK_SOT_HEIGHT 1080
#define OVIS_AI_FRAME_RATE_AUTO (-1)
#ifndef OVIS_SERVICE_SCRIPT
#define OVIS_SERVICE_SCRIPT "/etc/init.d/S99z_ipcamera"
#endif
#ifndef OVIS_USB_SERVICE_SCRIPT
#define OVIS_USB_SERVICE_SCRIPT "/etc/init.d/S77ncm"
#endif
#ifndef OVIS_PID_FILE
#define OVIS_PID_FILE "/var/run/ipcamera.pid"
#endif
#ifndef OVIS_CONFIG_DIR
#define OVIS_CONFIG_DIR "/mnt/cfg/ipcamera"
#endif
#ifndef OVIS_CONFIG_FILE
#define OVIS_CONFIG_FILE OVIS_CONFIG_DIR "/param_config.ini"
#endif
#ifndef OVIS_CONFIG_PENDING
#define OVIS_CONFIG_PENDING OVIS_CONFIG_FILE ".pending"
#endif
#ifndef OVIS_CONFIG_BACKUP
#define OVIS_CONFIG_BACKUP OVIS_CONFIG_FILE ".bak"
#endif
#ifndef OVIS_DEFAULT_CONFIG
#define OVIS_DEFAULT_CONFIG "/usr/share/ipcamera/param_config.ini"
#endif
#ifndef OVIS_AI_BNR_MODEL
#define OVIS_AI_BNR_MODEL "/usr/share/ipcamera/cv184x/bmodel_0326_blc1_patch2.bmodel"
#endif
#ifndef OVIS_AI_BNR_PQ_BIN
#define OVIS_AI_BNR_PQ_BIN "/mnt/data/ai-bnr/cvi_sdr_bin"
#endif
#define OVIS_ACCOUNT_FILE "/mnt/cfg/ovis-manager/ovis.account"
#define OVIS_DEVICE_ID_FILE "/mnt/cfg/ovis-manager/device-id"
#define OVIS_NCM_SUBNET_FILE "/mnt/cfg/ovis-manager/ncm-subnet"
#define OVIS_NCM_PENDING_SUBNET_FILE "/mnt/cfg/ovis-manager/ncm-subnet.pending"
#define OVIS_NCM_ACTIVE_FILE "/var/run/ovis-ncm-address"
#define OVIS_WEBUSB_EP0 "/dev/usb-ffs/ovis/ep0"
#define OVIS_WEBUSB_READY_FILE "/var/run/ovis-webusb-ready"
#define OVIS_AUDIT_LOG "/mnt/cfg/ovis-manager/audit.log"
#define OVIS_SC235HAI_30FPS_SNS_TYPE "0X20B83320"
#define OVIS_SC235HAI_60FPS_SNS_TYPE "0X20B83420"
#define OVIS_DISPLAY_MODE "720x480_60"
#define OVIS_DISPLAY_PANEL_TYPE "PANEL_BT656_MS7024_720x480_60"
#define OVIS_DISPLAY_WIDTH 720
#define OVIS_DISPLAY_HEIGHT 480
#define OVIS_DISPLAY_FPS 60
#define OVIS_DISPLAY_VPSS_GROUP 7
#define OVIS_DISPLAY_VB_POOL 9

enum service_action {
	SERVICE_START,
	SERVICE_STOP,
	SERVICE_RESTART,
	SERVICE_STATUS,
};

struct http_request {
	char method[8];
	char path[256];
	char authorization[256];
	char csrf[64];
	char origin[256];
	char content_type[128];
	char content_range[128];
	char transfer_encoding[64];
	char *body;
	size_t body_len;
	size_t content_length;
};

int http_server_run(unsigned short port);
int usb_provision_run(void);
int service_get_status(char *json, size_t size);
int service_run_action(enum service_action action, char *output, size_t size);
int usb_schedule_output_reboot(char *output, size_t size);
int service_reload_overlay(char *output, size_t size);
int config_get_output_flags(int *rtsp_enabled, int *uvc_enabled);
unsigned long task_submit(enum service_action action);
unsigned long config_task_submit_apply(const char *revision);
unsigned long config_task_submit_reset(void);
int task_is_busy(void);
int task_get(unsigned long id, char *json, size_t size);
int config_ensure_runtime(char *error, size_t error_size);
int config_read_json(char *json, size_t size);
int config_capabilities_json(char *json, size_t size);
int config_ai_bnr_supported(void);
int config_validate_json(const char *body, char *json, size_t size,
	char *error, size_t error_size);
int config_stage_json(const char *body, char *json, size_t size,
	char *error, size_t error_size);
int config_apply_staged(const char *revision, char *message, size_t message_size,
	int *rolled_back);
int config_apply_defaults(char *message, size_t message_size, int *rolled_back);
int config_validate_file(const char *path, char *error, size_t error_size);
int tracking_status_json(char *json, size_t size);
int tracking_target_set(const char *body, char *error, size_t error_size);
int tracking_target_clear(char *error, size_t error_size);
int config_stage_detection_model(const char *model_id, const char *model_path,
	const char *model_config_path, double threshold, int processing_width,
	int processing_height, int enabled,
	char revision[17], char *error, size_t error_size);
int config_rebase_backup_away_from_model(const char *id,
	char *error, size_t error_size);
int model_importers_json(char *json, size_t size);
int model_list_json(char *json, size_t size);
int model_get_json(const char *id, char *json, size_t size,
	char *error, size_t error_size);
int model_import_create(const char *body, char *json, size_t size,
	char *error, size_t error_size);
int model_import_write_content(const char *id, int fd, const void *initial,
	size_t initial_size, size_t content_length, char *json, size_t size,
	char *error, size_t error_size);
int model_import_status(const char *id, char *json, size_t size,
	char *error, size_t error_size);
int model_import_commit(const char *id, char *json, size_t size,
	char *error, size_t error_size);
int model_import_cancel(const char *id, char *error, size_t error_size);
int model_delete(const char *id, char *error, size_t error_size);
int model_deployment_get_json(const char *id, char *json, size_t size,
	char *error, size_t error_size);
int model_deployment_update(const char *id, const char *body,
	char *json, size_t size, char *error, size_t error_size);
int model_activate(const char *id, const char *body, char *json, size_t size,
	char *error, size_t error_size);
int model_deactivate(const char *id, char *json, size_t size,
	char *error, size_t error_size);
int device_info_json(char *json, size_t size);
int device_private_network_id(char *identity, size_t size);
int auth_check(const char *authorization);
void audit_log(const char *operation, const char *result);
void json_escape(const char *src, char *dst, size_t size);
int startup_run_logged(const char *path, char **command);

#endif
