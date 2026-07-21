#include "ovis_manager.h"
#include "cJSON.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define OVIS_NETWORK_RESET_COMMAND "/etc/init.d/S77ncm network-reset"
#define OVIS_NETWORK_RESET_GRACE_MS 1000

static pthread_mutex_t network_reset_lock = PTHREAD_MUTEX_INITIALIZER;
static int network_reset_scheduled;

static int send_all(int fd, const void *data, size_t size)
{
	const char *buffer = data;
	while (size > 0) {
		ssize_t sent = send(fd, buffer, size, 0);
		if (sent <= 0) return -1;
		buffer += sent;
		size -= (size_t)sent;
	}
	return 0;
}

static void *network_reset_worker(void *arg)
{
	struct timespec remaining = {
		.tv_sec = OVIS_NETWORK_RESET_GRACE_MS / 1000,
		.tv_nsec = (OVIS_NETWORK_RESET_GRACE_MS % 1000) * 1000000L,
	};
	int result;

	(void)arg;
	while (nanosleep(&remaining, &remaining) != 0 && errno == EINTR)
		;
	result = system(OVIS_NETWORK_RESET_COMMAND);
	audit_log("device.network.reset", result == 0 ? "success" : "failed");
	pthread_mutex_lock(&network_reset_lock);
	network_reset_scheduled = 0;
	pthread_mutex_unlock(&network_reset_lock);
	return NULL;
}

static int schedule_network_reset(void)
{
	pthread_t thread;

	pthread_mutex_lock(&network_reset_lock);
	if (network_reset_scheduled) {
		pthread_mutex_unlock(&network_reset_lock);
		return 1;
	}
	network_reset_scheduled = 1;
	if (pthread_create(&thread, NULL, network_reset_worker, NULL) != 0) {
		network_reset_scheduled = 0;
		pthread_mutex_unlock(&network_reset_lock);
		return -1;
	}
	pthread_detach(thread);
	pthread_mutex_unlock(&network_reset_lock);
	return 0;
}

static int origin_is_allowed(const char *origin)
{
	const char *allowed_origins[] = {
		OVIS_ALLOWED_ORIGIN,
		OVIS_DEV_ORIGIN,
		OVIS_DEV_ORIGIN_ALT,
	};
	size_t index;

	if (origin == NULL || origin[0] == '\0')
		return 1;
	for (index = 0; index < sizeof(allowed_origins) / sizeof(allowed_origins[0]); index++) {
		if (allowed_origins[index][0] != '\0' && strcmp(origin, allowed_origins[index]) == 0)
			return 1;
	}
	return 0;
}

static void send_response(int fd, int status, const char *type, const char *body,
	const struct http_request *request)
{
	char header[1536];
	char cors[768] =
		"Vary: Origin, Access-Control-Request-Method, Access-Control-Request-Headers, "
		"Access-Control-Request-Private-Network\r\n";
	char private_network_id[18];
	const char *origin = request == NULL ? NULL : request->origin;
	const char *reason = status == 200 ? "OK" : status == 201 ? "Created" :
		status == 202 ? "Accepted" :
		status == 204 ? "No Content" :
		status == 400 ? "Bad Request" : status == 401 ? "Unauthorized" :
		status == 403 ? "Forbidden" : status == 404 ? "Not Found" :
		status == 409 ? "Conflict" : "Internal Server Error";
	size_t length = strlen(body);

	if (origin != NULL && origin[0] != '\0' && origin_is_allowed(origin)) {
		if (device_private_network_id(private_network_id,
		    sizeof(private_network_id)) != 0)
			snprintf(private_network_id, sizeof(private_network_id),
				"02:18:42:00:00:01");
		snprintf(cors, sizeof(cors),
			"Access-Control-Allow-Origin: %s\r\n"
			"Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
			"Access-Control-Allow-Headers: Authorization, Content-Type, X-OVIS-CSRF\r\n"
			"Access-Control-Allow-Private-Network: true\r\n"
			"Private-Network-Access-Name: %s\r\n"
			"Private-Network-Access-ID: %s\r\n"
			"Access-Control-Max-Age: 600\r\n"
			"Vary: Origin, Access-Control-Request-Method, Access-Control-Request-Headers, "
			"Access-Control-Request-Private-Network\r\n",
			origin, OVIS_PRIVATE_NETWORK_NAME, private_network_id);
	}

	snprintf(header, sizeof(header),
		"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
		"Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
		"Connection: close\r\n%s%s\r\n", status, reason, type, length, cors,
		status == 401 ? "WWW-Authenticate: Basic realm=\"OVIS Manager\"\r\n" : "");
	send_all(fd, header, strlen(header));
	send_all(fd, body, length);
}

static void send_json_error(int fd, int status, const char *message,
	const struct http_request *request)
{
	char escaped[512];
	char body[640];
	json_escape(message, escaped, sizeof(escaped));
	snprintf(body, sizeof(body), "{\"error\":\"%s\"}", escaped);
	send_response(fd, status, "application/json; charset=utf-8", body, request);
}

static int parse_request(char *buffer, size_t length, struct http_request *request,
	int headers_only)
{
	char headers[8192];
	char *header_end = strstr(buffer, "\r\n\r\n");
	char *line, *save;
	size_t header_length;
	size_t content_length = 0;

	if (header_end == NULL) return -1;
	header_length = (size_t)(header_end - buffer) + 4;
	if (header_length >= sizeof(headers)) return -1;
	memcpy(headers, buffer, header_length);
	headers[header_length] = '\0';
	memset(request, 0, sizeof(*request));
	line = strtok_r(headers, "\r\n", &save);
	if (line == NULL || sscanf(line, "%7s %255s", request->method, request->path) != 2)
		return -1;
	while ((line = strtok_r(NULL, "\r\n", &save)) != NULL) {
		char *colon = strchr(line, ':');
		char *value;
		if (!colon) continue;
		*colon = '\0';
		value = colon + 1;
		while (*value == ' ' || *value == '\t') value++;
		if (strcasecmp(line, "Content-Length") == 0)
			content_length = strtoul(value, NULL, 10);
		else if (strcasecmp(line, "Content-Type") == 0)
			snprintf(request->content_type, sizeof(request->content_type), "%s", value);
		else if (strcasecmp(line, "Content-Range") == 0)
			snprintf(request->content_range, sizeof(request->content_range), "%s", value);
		else if (strcasecmp(line, "Transfer-Encoding") == 0)
			snprintf(request->transfer_encoding, sizeof(request->transfer_encoding),
				"%s", value);
		else if (strcasecmp(line, "Authorization") == 0)
			snprintf(request->authorization, sizeof(request->authorization), "%s", value);
		else if (strcasecmp(line, "X-OVIS-CSRF") == 0)
			snprintf(request->csrf, sizeof(request->csrf), "%s", value);
		else if (strcasecmp(line, "Origin") == 0)
			snprintf(request->origin, sizeof(request->origin), "%s", value);
	}
	request->body = buffer + header_length;
	request->content_length = content_length;
	if (headers_only) {
		request->body_len = length > header_length ? length - header_length : 0;
		if (request->body_len > content_length)
			request->body_len = content_length;
		return 0;
	}
	if (content_length > OVIS_MAX_BODY_SIZE || header_length + content_length > length)
		return -1;
	request->body_len = content_length;
	request->body[content_length] = '\0';
	return 0;
}

static int path_action_id(const char *path, const char *prefix,
	const char *action, char id[17])
{
	size_t prefix_length = strlen(prefix);
	size_t action_length = strlen(action);
	const char *start;

	if (strncmp(path, prefix, prefix_length) != 0)
		return 0;
	start = path + prefix_length;
	if (strlen(start) != 16 + action_length || strcmp(start + 16, action) != 0)
		return 0;
	memcpy(id, start, 16);
	id[16] = '\0';
	return 1;
}

static int require_write_access(int fd, const struct http_request *request)
{
	if (!auth_check(request->authorization)) {
		send_json_error(fd, 401, "需要管理员身份验证", request);
		return -1;
	}
	if (strcmp(request->csrf, "1") != 0) {
		send_json_error(fd, 403, "缺少 CSRF 请求头", request);
		return -1;
	}
	return 0;
}

static int require_model_write_access(int fd, const struct http_request *request)
{
	if (strcmp(request->csrf, "1") != 0) {
		send_json_error(fd, 403, "缺少 CSRF 请求头", request);
		return -1;
	}
	return 0;
}

static int parse_revision_body(const char *body, char *revision, size_t size)
{
	cJSON *root = cJSON_Parse(body);
	cJSON *item;
	int result = -1;

	if (!cJSON_IsObject(root))
		goto done;
	item = cJSON_GetObjectItemCaseSensitive(root, "revision");
	if (!cJSON_IsString(item) || item->valuestring == NULL ||
	    item->valuestring[0] == '\0' || strlen(item->valuestring) >= size)
		goto done;
	snprintf(revision, size, "%s", item->valuestring);
	result = 0;
done:
	cJSON_Delete(root);
	return result;
}

static void send_model_error(int fd, int result, const char *error,
	const struct http_request *request)
{
	int status = result == -2 ? 404 : result == -3 ? 409 : 400;
	send_json_error(fd, status, error[0] == '\0' ? "模型操作失败" : error, request);
}

static void route_request(int fd, const struct http_request *request)
{
	char json[128 * 1024];
	char error[256] = "";
	char revision[33];
	char model_id[17];
	unsigned long id;
	enum service_action action;
	int rc;

	if (request->origin[0] != '\0' && !origin_is_allowed(request->origin)) {
		send_json_error(fd, 403, "不允许的请求来源", request);
		return;
	}
	if (strcmp(request->method, "OPTIONS") == 0) {
		send_response(fd, 204, "text/plain", "", request);
		return;
	}
	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/device/info") == 0) {
		if (device_info_json(json, sizeof(json)) != 0)
			send_json_error(fd, 500, "无法生成设备信息", request);
		else
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		return;
	}
	if (strcmp(request->method, "POST") == 0 &&
	    strcmp(request->path, "/api/v1/device/network/reset") == 0) {
		rc = schedule_network_reset();
		if (rc == 1)
			send_json_error(fd, 409, "设备网络重置已在执行", request);
		else if (rc != 0)
			send_json_error(fd, 500, "无法创建设备网络重置任务", request);
		else {
			audit_log("device.network.reset", "accepted");
			send_response(fd, 202, "text/plain; charset=utf-8", "", request);
		}
		return;
	}
	if (strcmp(request->method, "GET") == 0 &&
	    strcmp(request->path, "/api/v1/config/capabilities") == 0) {
		if (config_capabilities_json(json, sizeof(json)) != 0)
			send_json_error(fd, 500, "无法生成配置能力信息", request);
		else
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		return;
	}
	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/config") == 0) {
		if (config_read_json(json, sizeof(json)) != 0)
			send_json_error(fd, 409, "运行配置不可用", request);
		else
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		return;
	}
	if (strcmp(request->method, "POST") == 0 &&
	    strcmp(request->path, "/api/v1/config/validate") == 0) {
		rc = config_validate_json(request->body, json, sizeof(json), error, sizeof(error));
		if (rc == 0 || rc == 1)
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		else
			send_json_error(fd, rc == -2 ? 409 : rc == -3 ? 500 : 400, error, request);
		return;
	}
	if (strcmp(request->method, "PUT") == 0 && strcmp(request->path, "/api/v1/config") == 0) {
		rc = config_stage_json(request->body, json, sizeof(json), error, sizeof(error));
		if (rc == 0)
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		else
			send_json_error(fd, rc == -2 ? 409 : rc == -3 ? 500 : 400, error, request);
		return;
	}
	if (strcmp(request->method, "POST") == 0 &&
	    strcmp(request->path, "/api/v1/config/apply") == 0) {
		if (parse_revision_body(request->body, revision, sizeof(revision)) != 0) {
			send_json_error(fd, 400, "配置版本无效", request);
			return;
		}
		id = config_task_submit_apply(revision);
		if (id == 0)
			send_json_error(fd, 409, "已有配置任务正在执行", request);
		else {
			snprintf(json, sizeof(json), "{\"task_id\":%lu}", id);
			send_response(fd, 202, "application/json; charset=utf-8", json, request);
		}
		return;
	}
	if (strcmp(request->method, "POST") == 0 &&
	    strcmp(request->path, "/api/v1/config/reset") == 0) {
		id = config_task_submit_reset();
		if (id == 0)
			send_json_error(fd, 409, "已有配置任务正在执行", request);
		else {
			snprintf(json, sizeof(json), "{\"task_id\":%lu}", id);
			send_response(fd, 202, "application/json; charset=utf-8", json, request);
		}
		return;
	}
	if (strcmp(request->method, "GET") == 0 &&
	    strcmp(request->path, "/api/v1/models/importers") == 0) {
		if (model_importers_json(json, sizeof(json)) != 0)
			send_json_error(fd, 500, "无法生成模型导入器列表", request);
		else
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		return;
	}
	if (strcmp(request->method, "GET") == 0 &&
	    strcmp(request->path, "/api/v1/models") == 0) {
		if (model_list_json(json, sizeof(json)) != 0)
			send_json_error(fd, 500, "无法读取模型列表", request);
		else
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		return;
	}
	if (strcmp(request->method, "POST") == 0 &&
	    strcmp(request->path, "/api/v1/models/imports") == 0) {
		if (require_model_write_access(fd, request) != 0)
			return;
		rc = model_import_create(request->body, json, sizeof(json), error, sizeof(error));
		if (rc == 0)
			send_response(fd, 201, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/imports/", "", model_id) &&
	    strcmp(request->method, "GET") == 0) {
		rc = model_import_status(model_id, json, sizeof(json), error, sizeof(error));
		if (rc == 0)
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/imports/", "", model_id) &&
	    strcmp(request->method, "DELETE") == 0) {
		if (require_model_write_access(fd, request) != 0)
			return;
		rc = model_import_cancel(model_id, error, sizeof(error));
		if (rc == 0)
			send_response(fd, 204, "text/plain", "", request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/imports/", "/commit", model_id) &&
	    strcmp(request->method, "POST") == 0) {
		if (require_model_write_access(fd, request) != 0)
			return;
		rc = model_import_commit(model_id, json, sizeof(json), error, sizeof(error));
		if (rc == 0)
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/", "/deployment", model_id) &&
	    strcmp(request->method, "GET") == 0) {
		rc = model_deployment_get_json(model_id, json, sizeof(json), error,
			sizeof(error));
		if (rc == 0)
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/", "/deployment", model_id) &&
	    strcmp(request->method, "PUT") == 0) {
		if (require_model_write_access(fd, request) != 0)
			return;
		rc = model_deployment_update(model_id, request->body, json, sizeof(json),
			error, sizeof(error));
		if (rc == 0)
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/", "/activate", model_id) &&
	    strcmp(request->method, "POST") == 0) {
		if (require_model_write_access(fd, request) != 0)
			return;
		rc = model_activate(model_id, request->body, json, sizeof(json), error, sizeof(error));
		if (rc == 0)
			send_response(fd, 202, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/", "/deactivate", model_id) &&
	    strcmp(request->method, "POST") == 0) {
		if (require_model_write_access(fd, request) != 0)
			return;
		rc = model_deactivate(model_id, json, sizeof(json), error, sizeof(error));
		if (rc == 0)
			send_response(fd, 202, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/", "", model_id) &&
	    strcmp(request->method, "GET") == 0) {
		rc = model_get_json(model_id, json, sizeof(json), error, sizeof(error));
		if (rc == 0)
			send_response(fd, 200, "application/json; charset=utf-8", json, request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}
	if (path_action_id(request->path, "/api/v1/models/", "", model_id) &&
	    strcmp(request->method, "DELETE") == 0) {
		if (require_model_write_access(fd, request) != 0)
			return;
		rc = model_delete(model_id, error, sizeof(error));
		if (rc == 0)
			send_response(fd, 204, "text/plain", "", request);
		else
			send_model_error(fd, rc, error, request);
		return;
	}

	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/ipcamera/status") == 0) {
		service_get_status(json, sizeof(json)); send_response(fd, 200, "application/json", json, request); return;
	}
	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/system/version") == 0) {
		snprintf(json, sizeof(json), "{\"manager\":\"%s\",\"firmware\":\"%s\"}",
			OVIS_VERSION, OVIS_FIRMWARE_VERSION);
		send_response(fd, 200, "application/json", json, request); return;
	}
	if (strcmp(request->method, "GET") == 0 && strncmp(request->path, "/api/v1/tasks/", 14) == 0) {
		id = strtoul(request->path + 14, NULL, 10);
		if (task_get(id, json, sizeof(json)) != 0) send_json_error(fd, 404, "任务不存在", request);
		else send_response(fd, 200, "application/json", json, request);
		return;
	}
	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/ipcamera/config") == 0) {
		if (!auth_check(request->authorization)) { send_json_error(fd, 401, "需要管理员身份验证", request); return; }
		if (config_read_json(json, sizeof(json)) != 0) send_json_error(fd, 409, "运行配置不可用", request);
		else send_response(fd, 200, "application/json", json, request);
		return;
	}
	if (strcmp(request->method, "POST") == 0 &&
	    (strcmp(request->path, "/api/v1/ipcamera/start") == 0 ||
	     strcmp(request->path, "/api/v1/ipcamera/stop") == 0 ||
	     strcmp(request->path, "/api/v1/ipcamera/restart") == 0)) {
		if (require_write_access(fd, request) != 0) return;
		action = strstr(request->path, "/start") ? SERVICE_START : strstr(request->path, "/stop") ? SERVICE_STOP : SERVICE_RESTART;
		id = task_submit(action);
		if (!id) send_json_error(fd, 500, "无法创建服务任务", request);
		else { snprintf(json, sizeof(json), "{\"task_id\":%lu}", id); send_response(fd, 202, "application/json", json, request); }
		return;
	}
	if (strcmp(request->method, "PUT") == 0 && strcmp(request->path, "/api/v1/ipcamera/config") == 0) {
		send_json_error(fd, 404, "旧配置写接口已停用", request);
		return;
	}
	if (strcmp(request->method, "POST") == 0 && strcmp(request->path, "/api/v1/ipcamera/config/reset") == 0) {
		send_json_error(fd, 404, "旧配置重置接口已停用", request);
		return;
	}
	send_json_error(fd, 404, "接口不存在", request);
}

static void handle_client(int fd)
{
	char buffer[OVIS_MAX_REQUEST_SIZE + 1];
	char json[4096];
	char error[256] = "";
	char model_id[17];
	size_t used = 0;
	ssize_t count;
	char *headers;
	size_t header_length;
	size_t expected;
	struct http_request request;
	int rc;

	headers = NULL;
	while (used < OVIS_MAX_REQUEST_SIZE && headers == NULL) {
		count = recv(fd, buffer + used, OVIS_MAX_REQUEST_SIZE - used, 0);
		if (count < 0 && errno == EINTR)
			continue;
		if (count <= 0)
			break;
		used += (size_t)count;
		buffer[used] = '\0';
		headers = strstr(buffer, "\r\n\r\n");
	}
	if (headers == NULL || parse_request(buffer, used, &request, 1) != 0) {
		send_json_error(fd, 400, "HTTP 请求无效或过大", NULL);
		return;
	}
	header_length = (size_t)(headers - buffer) + 4;
	if (strcmp(request.method, "PUT") == 0 &&
	    path_action_id(request.path, "/api/v1/models/imports/", "/content", model_id)) {
		if (require_model_write_access(fd, &request) != 0)
			return;
		if (request.content_range[0] != '\0' || request.transfer_encoding[0] != '\0' ||
		    strcasecmp(request.content_type, "application/octet-stream") != 0) {
			send_json_error(fd, 400,
				"模型上传仅支持带 Content-Length 的 application/octet-stream 全量请求",
				&request);
			return;
		}
		rc = model_import_write_content(model_id, fd, request.body,
			request.body_len, request.content_length, json, sizeof(json),
			error, sizeof(error));
		if (rc == 0)
			send_response(fd, 200, "application/json; charset=utf-8", json, &request);
		else
			send_model_error(fd, rc, error, &request);
		return;
	}
	if (request.content_length > OVIS_MAX_BODY_SIZE ||
	    header_length + request.content_length > OVIS_MAX_REQUEST_SIZE) {
		send_json_error(fd, 400, "HTTP 请求无效或过大", &request);
		return;
	}
	expected = header_length + request.content_length;
	while (used < expected) {
		count = recv(fd, buffer + used, expected - used, 0);
		if (count < 0 && errno == EINTR)
			continue;
		if (count <= 0)
			break;
		used += (size_t)count;
	}
	buffer[used] = '\0';
	if (used < expected || parse_request(buffer, used, &request, 0) != 0)
		send_json_error(fd, 400, "HTTP 请求无效或过大", &request);
	else
		route_request(fd, &request);
}

static void get_bind_address(char *value, size_t size)
{
	struct in_addr parsed;
	FILE *file;

	snprintf(value, size, "%s", OVIS_BIND_ADDRESS);
	file = fopen(OVIS_BIND_ADDRESS_FILE, "r");
	if (file == NULL)
		return;
	if (fgets(value, (int)size, file) == NULL) {
		snprintf(value, size, "%s", OVIS_BIND_ADDRESS);
	} else {
		value[strcspn(value, "\r\n")] = '\0';
		if (inet_pton(AF_INET, value, &parsed) != 1)
			snprintf(value, size, "%s", OVIS_BIND_ADDRESS);
	}
	fclose(file);
}

int http_server_run(unsigned short port)
{
	struct sockaddr_in address = {0};
	char bind_address[INET_ADDRSTRLEN];
	int server;
	int option = 1;

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server < 0) return 1;
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	address.sin_family = AF_INET;
	get_bind_address(bind_address, sizeof(bind_address));
	if (inet_pton(AF_INET, bind_address, &address.sin_addr) != 1) {
		fprintf(stderr, "invalid bind address: %s\n", bind_address);
		close(server);
		return 1;
	}
	address.sin_port = htons(port);
	if (bind(server, (struct sockaddr *)&address, sizeof(address)) != 0 || listen(server, 8) != 0) {
		perror("ovis-manager listen"); close(server); return 1;
	}
	printf("ovis-managerd listening on %s:%u\n", bind_address, port);
	for (;;) {
		int client = accept(server, NULL, NULL);
		if (client < 0) { if (errno == EINTR) continue; break; }
		handle_client(client);
		close(client);
	}
	close(server);
	return 1;
}
