#include "ovis_manager.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

static char *find_header(char *buffer, const char *name)
{
	size_t length = strlen(name);
	char *line = buffer;

	while (line != NULL && *line) {
		if (strncasecmp(line, name, length) == 0)
			return line;
		line = strstr(line, "\r\n");
		if (line != NULL) line += 2;
	}
	return NULL;
}

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

static void send_response(int fd, int status, const char *type, const char *body)
{
	char header[512];
	const char *reason = status == 200 ? "OK" : status == 202 ? "Accepted" :
		status == 400 ? "Bad Request" : status == 401 ? "Unauthorized" :
		status == 403 ? "Forbidden" : status == 404 ? "Not Found" :
		status == 409 ? "Conflict" : "Internal Server Error";
	size_t length = strlen(body);

	snprintf(header, sizeof(header),
		"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
		"Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
		"Content-Security-Policy: default-src 'self'; style-src 'self'; script-src 'self'\r\n"
		"Connection: close\r\n%s\r\n", status, reason, type, length,
		status == 401 ? "WWW-Authenticate: Basic realm=\"OVIS Manager\"\r\n" : "");
	send_all(fd, header, strlen(header));
	send_all(fd, body, length);
}

static void send_json_error(int fd, int status, const char *message)
{
	char escaped[512];
	char body[640];
	json_escape(message, escaped, sizeof(escaped));
	snprintf(body, sizeof(body), "{\"error\":\"%s\"}", escaped);
	send_response(fd, status, "application/json; charset=utf-8", body);
}

static int parse_request(char *buffer, size_t length, struct http_request *request)
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
		else if (strcasecmp(line, "Authorization") == 0)
			snprintf(request->authorization, sizeof(request->authorization), "%s", value);
		else if (strcasecmp(line, "X-OVIS-CSRF") == 0)
			snprintf(request->csrf, sizeof(request->csrf), "%s", value);
	}
	if (content_length > OVIS_MAX_BODY_SIZE || header_length + content_length > length)
		return -1;
	request->body = buffer + header_length;
	request->body_len = content_length;
	request->body[content_length] = '\0';
	return 0;
}

static int require_write_access(int fd, const struct http_request *request)
{
	if (!auth_check(request->authorization)) {
		send_json_error(fd, 401, "需要管理员身份验证");
		return -1;
	}
	if (strcmp(request->csrf, "1") != 0) {
		send_json_error(fd, 403, "缺少 CSRF 请求头");
		return -1;
	}
	return 0;
}

static void serve_static(int fd, const char *path)
{
	const char *name;
	const char *type;
	char full_path[512];
	char *data;
	struct stat statbuf;
	int file;

	if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
		name = "index.html"; type = "text/html; charset=utf-8";
	} else if (strcmp(path, "/app.js") == 0) {
		name = "app.js"; type = "application/javascript; charset=utf-8";
	} else if (strcmp(path, "/style.css") == 0) {
		name = "style.css"; type = "text/css; charset=utf-8";
	} else {
		send_json_error(fd, 404, "资源不存在"); return;
	}
	snprintf(full_path, sizeof(full_path), "%s/%s", OVIS_WWW_ROOT, name);
	file = open(full_path, O_RDONLY);
	if (file < 0 || fstat(file, &statbuf) != 0 || statbuf.st_size > 1024 * 1024) {
		if (file >= 0) close(file);
		send_json_error(fd, 404, "资源不存在"); return;
	}
	data = malloc((size_t)statbuf.st_size + 1);
	if (!data) { close(file); send_json_error(fd, 500, "内存不足"); return; }
	{
		size_t used = 0;
		while (used < (size_t)statbuf.st_size) {
			ssize_t count = read(file, data + used, (size_t)statbuf.st_size - used);
			if (count <= 0) { free(data); close(file); send_json_error(fd, 500, "读取资源失败"); return; }
			used += (size_t)count;
		}
	}
	close(file); data[statbuf.st_size] = '\0';
	send_response(fd, 200, type, data);
	free(data);
}

static void route_request(int fd, const struct http_request *request)
{
	char json[4096];
	char error[256];
	unsigned long id;
	enum service_action action;

	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/ipcamera/status") == 0) {
		service_get_status(json, sizeof(json)); send_response(fd, 200, "application/json", json); return;
	}
	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/system/version") == 0) {
		snprintf(json, sizeof(json), "{\"manager\":\"%s\",\"firmware\":\"%s\"}", OVIS_VERSION, "CV1842HP OVIS");
		send_response(fd, 200, "application/json", json); return;
	}
	if (strcmp(request->method, "GET") == 0 && strncmp(request->path, "/api/v1/tasks/", 14) == 0) {
		id = strtoul(request->path + 14, NULL, 10);
		if (task_get(id, json, sizeof(json)) != 0) send_json_error(fd, 404, "任务不存在");
		else send_response(fd, 200, "application/json", json);
		return;
	}
	if (strcmp(request->method, "GET") == 0 && strcmp(request->path, "/api/v1/ipcamera/config") == 0) {
		if (!auth_check(request->authorization)) { send_json_error(fd, 401, "需要管理员身份验证"); return; }
		if (config_read_json(json, sizeof(json)) != 0) send_json_error(fd, 409, "运行配置不可用");
		else send_response(fd, 200, "application/json", json);
		return;
	}
	if (strcmp(request->method, "POST") == 0 &&
	    (strcmp(request->path, "/api/v1/ipcamera/start") == 0 ||
	     strcmp(request->path, "/api/v1/ipcamera/stop") == 0 ||
	     strcmp(request->path, "/api/v1/ipcamera/restart") == 0)) {
		if (require_write_access(fd, request) != 0) return;
		action = strstr(request->path, "/start") ? SERVICE_START : strstr(request->path, "/stop") ? SERVICE_STOP : SERVICE_RESTART;
		id = task_submit(action);
		if (!id) send_json_error(fd, 500, "无法创建服务任务");
		else { snprintf(json, sizeof(json), "{\"task_id\":%lu}", id); send_response(fd, 202, "application/json", json); }
		return;
	}
	if (strcmp(request->method, "PUT") == 0 && strcmp(request->path, "/api/v1/ipcamera/config") == 0) {
		if (require_write_access(fd, request) != 0) return;
		if (config_update_json(request->body, error, sizeof(error)) != 0) send_json_error(fd, 400, error);
		else send_response(fd, 200, "application/json", "{\"saved\":true,\"restart_required\":true}");
		return;
	}
	if (strcmp(request->method, "POST") == 0 && strcmp(request->path, "/api/v1/ipcamera/config/reset") == 0) {
		if (require_write_access(fd, request) != 0) return;
		if (config_reset(error, sizeof(error)) != 0) send_json_error(fd, 409, error);
		else send_response(fd, 200, "application/json", "{\"reset\":true,\"restart_required\":true}");
		return;
	}
	if (strncmp(request->path, "/api/", 5) == 0) { send_json_error(fd, 404, "接口不存在"); return; }
	if (strcmp(request->method, "GET") != 0) { send_json_error(fd, 400, "不支持的请求方法"); return; }
	serve_static(fd, request->path);
}

static void handle_client(int fd)
{
	char buffer[OVIS_MAX_REQUEST_SIZE + 1];
	size_t used = 0;
	ssize_t count;
	char *headers;
	size_t expected = 0;
	struct http_request request;

	while (used < OVIS_MAX_REQUEST_SIZE) {
		count = recv(fd, buffer + used, OVIS_MAX_REQUEST_SIZE - used, 0);
		if (count <= 0) break;
		used += (size_t)count; buffer[used] = '\0';
		headers = strstr(buffer, "\r\n\r\n");
		if (headers) {
			char *length_header = find_header(buffer, "Content-Length:");
			expected = (size_t)(headers - buffer) + 4;
			if (length_header && length_header < headers)
				expected += strtoul(length_header + 15, NULL, 10);
			if (used >= expected) break;
		}
	}
	if (used == OVIS_MAX_REQUEST_SIZE || parse_request(buffer, used, &request) != 0)
		send_json_error(fd, 400, "HTTP 请求无效或过大");
	else
		route_request(fd, &request);
}

int http_server_run(unsigned short port)
{
	struct sockaddr_in address = {0};
	int server;
	int option = 1;

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server < 0) return 1;
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	address.sin_family = AF_INET;
	if (inet_pton(AF_INET, OVIS_BIND_ADDRESS, &address.sin_addr) != 1) {
		fprintf(stderr, "invalid bind address: %s\n", OVIS_BIND_ADDRESS);
		close(server);
		return 1;
	}
	address.sin_port = htons(port);
	if (bind(server, (struct sockaddr *)&address, sizeof(address)) != 0 || listen(server, 8) != 0) {
		perror("ovis-manager listen"); close(server); return 1;
	}
	printf("ovis-managerd listening on port %u\n", port);
	for (;;) {
		int client = accept(server, NULL, NULL);
		if (client < 0) { if (errno == EINTR) continue; break; }
		handle_client(client);
		close(client);
	}
	close(server);
	return 1;
}
