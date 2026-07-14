#ifndef OVIS_MANAGER_H
#define OVIS_MANAGER_H

#include <stddef.h>
#include <sys/types.h>

#define OVIS_VERSION "1.0.0"
#ifndef OVIS_DEFAULT_PORT
#define OVIS_DEFAULT_PORT 8080
#endif
#ifndef OVIS_BIND_ADDRESS
#define OVIS_BIND_ADDRESS "0.0.0.0"
#endif
#define OVIS_MAX_REQUEST_SIZE (16 * 1024)
#define OVIS_MAX_BODY_SIZE (8 * 1024)
#define OVIS_WWW_ROOT "/usr/share/ovis-manager/www"
#define OVIS_SERVICE_SCRIPT "/etc/init.d/S99ipcamera"
#define OVIS_PID_FILE "/var/run/ipcamera.pid"
#define OVIS_CONFIG_DIR "/mnt/cfg/ipcamera"
#define OVIS_CONFIG_FILE OVIS_CONFIG_DIR "/param_config.ini"
#define OVIS_CONFIG_BACKUP OVIS_CONFIG_FILE ".bak"
#define OVIS_DEFAULT_CONFIG "/usr/share/ipcamera/param_config.ini"
#define OVIS_ACCOUNT_FILE "/mnt/cfg/ovis-manager/ovis.account"
#define OVIS_AUDIT_LOG "/mnt/cfg/ovis-manager/audit.log"

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
	char *body;
	size_t body_len;
};

int http_server_run(unsigned short port);
int service_get_status(char *json, size_t size);
int service_run_action(enum service_action action, char *output, size_t size);
unsigned long task_submit(enum service_action action);
int task_get(unsigned long id, char *json, size_t size);
int config_ensure_runtime(char *error, size_t error_size);
int config_read_json(char *json, size_t size);
int config_update_json(const char *body, char *error, size_t error_size);
int config_reset(char *error, size_t error_size);
int config_validate_file(const char *path, char *error, size_t error_size);
int auth_check(const char *authorization);
void audit_log(const char *operation, const char *result);
void json_escape(const char *src, char *dst, size_t size);

#endif
