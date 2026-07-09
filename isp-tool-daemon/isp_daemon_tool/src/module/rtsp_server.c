#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/prctl.h>

#include "rtsp_server.h"

#define CLOG_OUTPUT_LVL CLOG_LVL_INFO
#define CLOG_TAG "rtsp_server"
#include "clog.h"

#ifndef CLOG_TAG
#ifndef clog_e
#define clog_e(fmt, ...)                                                       \
	fprintf(stderr, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#endif
#ifndef clog_i
#define clog_i(fmt, ...)                                                       \
	fprintf(stdout, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#endif
#ifndef clog_d
#define clog_d(fmt, ...)                                                       \
	fprintf(stdout, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define RTSP_MSG_BUF_SIZE (2048)
#define RTP_OVER_TCP_SEND_BUF_SIZE (4 * 1024 * 1024)

#define RTP_CHANNEL 0 /* 0: RTP, 1: RTCP */
#define MTU 1500
#define RTP_CLOCK 90000

typedef enum {
	RTSP_STATE_IDLE,
	RTSP_STATE_PLAYING,
	RTSP_STATE_BUTT
} rtsp_session_state_e;

typedef struct {
	uint8_t magic;
	uint8_t chn;
	uint16_t len;
} __attribute__((packed)) rtp_over_tcp_hdr_t;

typedef struct {
	uint8_t vpxcc;
	uint8_t mpt;
	uint16_t seq;
	uint32_t ts;
	uint32_t ssrc;
} __attribute__((packed)) rtp_hdr_t;

typedef struct {
	uint16_t seq_num;
	uint32_t time_stamp;
	uint32_t ssrc;
	uint64_t last_time_stamp;
} rtp_session_t;

typedef struct {
	int client_fd;
	char clientip_str[32];
	int session_id;
	int cseq;
	char localip_str[32];
	rtsp_session_state_e state;
	rtp_session_t *rtp_session;
} rtsp_session_t;

typedef struct {
	int is_init;
	pthread_mutex_t mutex;
	cvi_rtsp_video_info_t video_info;
	cvi_rtsp_event_callback_t event_cb;
	rtsp_session_t *session;
} rtsp_hdl_t;

static int rtsp_server_thread_run;
static pthread_t rtsp_server_thread;
static rtsp_session_t *rtsp_sessions[RTSP_MAX_CLIENTS];
static rtsp_hdl_t rtsp_hdl[RTSP_MAX_CLIENTS];

static int rev_rtsp_msg(int client_fd, char *buf, int buf_size)
{
	if (buf == NULL || buf_size <= 0) {
		clog_e("Invalid buffer parameters\n");
		return -1;
	}

	int n = read(client_fd, buf, buf_size - 1);

	if (n < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		clog_e("read error: %s\n", strerror(errno));
		return -1;
	} else if (n == 0) {
		clog_i("client closed connection\n");
		return -1;
	} else if (n >= buf_size - 1) {
		clog_e("buffer overflow, message too long\n");
		return -1;
	}
	buf[n] = '\0'; // Null-terminate the string
	clog_d("rev msg: %s\ncount: %d\n", buf, n);
	return n;
}

static int send_rtsp_msg(int client_fd, const char *msg)
{
	int n = write(client_fd, msg, strlen(msg));

	if (n < 0) {
		if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN) {
			clog_i("Client disconnected during RTSP message send: %s\n", strerror(errno));
		} else {
			clog_e("write error: %s\n", strerror(errno));
		}
		return -1;
	} else if (n < (int)strlen(msg)) {
		clog_e("incomplete write: sent %d of %d bytes\n", n, (int)strlen(msg));
		return -1;
	}
	clog_d("send msg: %s\n", msg);
	return n;
}

static int release_session(rtsp_session_t *session)
{
	if (session == NULL) {
		return -1;
	}

	for (int i = 0; i < RTSP_MAX_CLIENTS; ++i) {
		if (rtsp_sessions[i] == session) {
			if (session->session_id >= 0) {
				rtsp_hdl[session->session_id].session = NULL;
			}
			if (session->rtp_session != NULL) {
				free(session->rtp_session);
				session->rtp_session = NULL;
			}
			session->client_fd = -1;
			free(rtsp_sessions[i]);
			rtsp_sessions[i] = NULL;
			clog_d("free session index:%d\n", i);
			return 0;
		}
	}

	return 0;
}

static int get_session_id(const char *msg)
{
	char *session_str = NULL;
	int session_id = -1;

	if (strstr(msg, "rtsp") == NULL) {
		clog_e("Invalid RTSP message: %s\n", msg);
		return -1;
	}

	session_str = strstr(msg, RTSP_SESSION_PREFIX);
	if (session_str == NULL) {
		clog_e("No session found in message: %s\n", msg);
		return -1;
	}

	session_str += strlen(RTSP_SESSION_PREFIX);
	session_id = atoi(session_str);
	if (session_id < 0 || session_id >= RTSP_MAX_CLIENTS) {
		clog_e("Invalid session ID: %d\n", session_id);
		return -1;
	}

	clog_d("session_id: %d\n", session_id);
	return session_id;
}

static int get_localip_str(rtsp_session_t *session, const char *msg)
{
	// url = rtsp://ip:port/stream....
	const char *url = NULL;
	const char *port = NULL;
	int ip_len = 0;

	url = strstr(msg, "rtsp://");
	if (url == NULL) {
		goto parse_err;
	}

	port = strchr(&url[7], ':');
	if (port == NULL) {
		goto parse_err;
	}

	ip_len = port - url - 7;
	if (ip_len >= (int)sizeof(session->localip_str)) {
		clog_e("IP address too long: %d\n", ip_len);
		goto parse_err;
	}

	strncpy(session->localip_str, &url[7], ip_len);
	session->localip_str[ip_len] = '\0';
	clog_d("get localip: %s\n", session->localip_str);
	return 0;

parse_err:
	clog_e("Invalid RTSP message: %s\n", msg);
	return -1;
}

static int init_rtsp_session_info(rtsp_session_t *session, const char *msg)
{
	int ret = 0;
	int session_id = -1;

	session_id = get_session_id(msg);
	if (session_id < 0) {
		clog_e("Failed to get session ID from message: %s\n", msg);
		return -1;
	}

	for (int i = 0; i < RTSP_MAX_CLIENTS; ++i) {
		if (rtsp_sessions[i] == NULL) {
			continue;
		}
		if (rtsp_sessions[i] != session &&
		    rtsp_sessions[i]->session_id == session_id) {
			clog_e("Only one client per session allowed\n");
			return -1;
		}
	}
	session->session_id = session_id;

	if (rtsp_hdl[session_id].is_init) {
		rtsp_hdl[session_id].session = session;
	} else {
		clog_e("Session %d not initialized\n", session_id);
		return -1;
	}

	ret = get_localip_str(session, msg);
	return ret;
}

static int check_rtsp_msg(rtsp_session_t *session, const char *msg)
{
	char res[16] = { '\0' };

	snprintf(res, sizeof(res), "%s%d", RTSP_SESSION_PREFIX,
		 session->session_id);

	if (strstr(msg, res) != NULL) {
		return 0;
	} else {
		return -1;
	}
}

static int get_rtsp_msg_cseq(rtsp_session_t *session, const char *msg)
{
	char *cseq_str = NULL;
	int cseq = -1;

	cseq_str = strstr(msg, "CSeq:");
	if (cseq_str == NULL) {
		clog_e("No CSeq found in message: %s\n", msg);
		return -1;
	}

	cseq_str += 5; // Skip "CSeq:"
	cseq = atoi(cseq_str);
	if (cseq < 0) {
		clog_e("Invalid CSeq value: %d\n", cseq);
		return -1;
	}

	clog_d("CSeq: %d\n", cseq);
	session->cseq = cseq;
	return 0;
}

static int do_rtsp_cmd_options(rtsp_session_t *session)
{
	char response[256] = { '\0' };

	snprintf(response, sizeof(response),
		 "RTSP/1.0 200 OK\r\n"
		 "CSeq: %d\r\n"
		 "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n"
		 "\r\n",
		 session->cseq);
	return send_rtsp_msg(session->client_fd, response);
}

static int generate_sdp_str(rtsp_session_t *session, char *sdp_buf,
			    size_t buf_size)
{
	char sdp_time[32];
	time_t t = time(NULL);

	snprintf(sdp_time, sizeof(sdp_time), "%lu", (unsigned long)t);

	static const char h265_sdp[] = "m=video 0 RTP/AVP 98\r\n"
				"a=rtpmap:98 H265/90000\r\n";

	static const char h264_sdp[] = "m=video 0 RTP/AVP 96\r\n"
				"a=rtpmap:96 H264/90000\r\n";

	const char *sdp = NULL;
	rtsp_hdl_t *hdl = &rtsp_hdl[session->session_id];

	if (hdl->video_info.codec_type == RTSP_CODEC_H265) {
		sdp = h265_sdp;
	} else if (hdl->video_info.codec_type == RTSP_CODEC_H264) {
		sdp = h264_sdp;
	} else {
		clog_e("Unsupported codec type: %d\n",
		       hdl->video_info.codec_type);
		return -1;
	}

	snprintf(sdp_buf, buf_size,
		 "v=0\r\n"
		 "o=- %s 0 IN IP4 %s\r\n"
		 "s=session\r\n"
		 "c=IN IP4 %s\r\n"
		 "t=0 0\r\n"
		 "%s"
		 "a=control:video0\r\n",
		 sdp_time, session->localip_str, session->clientip_str, sdp);

	return 0;
}

static int do_rtsp_cmd_describe(rtsp_session_t *session)
{
	char response[256 + 128];
	char sdp_buf[256];

	generate_sdp_str(session, sdp_buf, sizeof(sdp_buf));
	snprintf(response, sizeof(response),
		 "RTSP/1.0 200 OK\r\n"
		 "CSeq: %d\r\n"
		 "Content-Type: application/sdp\r\n"
		 "Content-Length: %d\r\n"
		 "\r\n"
		 "%s",
		 session->cseq, (int)strlen(sdp_buf), sdp_buf);

	return send_rtsp_msg(session->client_fd, response);
}

static int do_rtsp_cmd_setup(rtsp_session_t *session)
{
	char response[256];

	snprintf(response, sizeof(response),
		 "RTSP/1.0 200 OK\r\n"
		 "CSeq: %d\r\n"
		 "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n"
		 "Session: %s%d\r\n"
		 "\r\n",
		 session->cseq, RTSP_SESSION_PREFIX, session->session_id);
	return send_rtsp_msg(session->client_fd, response);
}

static int do_rtsp_cmd_play(rtsp_session_t *session)
{
	int ret = -1;
	char response[256];

	snprintf(response, sizeof(response),
		 "RTSP/1.0 200 OK\r\n"
		 "CSeq: %d\r\n"
		 "Session: %s%d\r\n"
		 "Range: npt=0.000-\r\n"
		 "\r\n",
		 session->cseq, RTSP_SESSION_PREFIX, session->session_id);
	ret = send_rtsp_msg(session->client_fd, response);

	if (ret < 0) {
		return -1;
	}

	if (session->rtp_session == NULL) {
		session->rtp_session =
			(rtp_session_t *)calloc(1, sizeof(rtp_session_t));
		session->rtp_session->ssrc = (uint32_t)rand();
		if (rtsp_hdl[session->session_id].event_cb.on_play) {
			clog_d("Calling on_play callback for session_id=%d\n",
			       session->session_id);
			rtsp_hdl[session->session_id].event_cb.on_play(
				session->session_id,
				rtsp_hdl[session->session_id].event_cb.play_arg);
		}
	} else {
		clog_d("RTP session %d already start\n", session->session_id);
	}

	return 0;
}

static int do_rtsp_cmd_teardown(rtsp_session_t *session)
{
	//char response[256];

	if (session->rtp_session != NULL) {
		free(session->rtp_session);
		session->rtp_session = NULL;
		if (rtsp_hdl[session->session_id].event_cb.on_teardown) {
			clog_d("Calling on_teardown callback for session_id=%d\n",
			       session->session_id);
			rtsp_hdl[session->session_id].event_cb.on_teardown(
				session->session_id,
				rtsp_hdl[session->session_id]
					.event_cb.teardown_arg);
		}
	} else {
		clog_d("No RTP session to stop for session_id=%d\n",
		       session->session_id);
	}

	//snprintf(response, sizeof(response),
	//         "RTSP/1.0 200 OK\r\n"
	//         "CSeq: %d\r\n"
	//         "Session: %s%d\r\n"
	//         "\r\n", session->cseq,
	//         RTSP_SESSION_PREFIX, session->session_id);

	//return send_rtsp_msg(session->client_fd, response);
	return 0;
}

static int do_rtsp_msg_cmd(rtsp_session_t *session, char *msg_buf)
{
	int ret = 0;

	ret = rev_rtsp_msg(session->client_fd, msg_buf, RTSP_MSG_BUF_SIZE);
	if (ret < 0) {
		return -1;
	}

	if (session->state == RTSP_STATE_IDLE) {
		if (init_rtsp_session_info(session, msg_buf) < 0) {
			return -1;
		}
	}

	ret = check_rtsp_msg(session, msg_buf);
	if (ret < 0) {
		if (session->state != RTSP_STATE_PLAYING) {
			//clog_e("check rtsp msg err: %s\n", msg_buf);
			clog_e("check rtsp msg err\n");
			return -1;
		} else {
			//clog_d("skip unknown msg in play state, %s\n", msg_buf);
			clog_d("skip unknown msg in play state.\n");
			return 0;
		}
	}

	ret = get_rtsp_msg_cseq(session, msg_buf);
	if (ret < 0) {
		return -1;
	}

	if (strstr(msg_buf, "OPTIONS")) {
		ret = do_rtsp_cmd_options(session);
	} else if (strstr(msg_buf, "DESCRIBE")) {
		ret = do_rtsp_cmd_describe(session);
	} else if (strstr(msg_buf, "SETUP")) {
		ret = do_rtsp_cmd_setup(session);
	} else if (strstr(msg_buf, "PLAY")) {
		ret = do_rtsp_cmd_play(session);
		session->state = RTSP_STATE_PLAYING;
	} else if (strstr(msg_buf, "TEARDOWN")) {
		ret = do_rtsp_cmd_teardown(session);
		session->state = RTSP_STATE_IDLE;
		return -1;
	} else {
		clog_e("Not support RTSP command: %s\n", msg_buf);
		return -1;
	}

	return ret;
}

static int set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);

	if (flags == -1)
		return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void *rtsp_server_loop(void *arg)
{
	int listen_fd, max_fd, i;
	fd_set all_set, r_set;

	UNUSED(arg);

	prctl(PR_SET_NAME, "rtsp_server", 0, 0, 0);

	// Ignore SIGPIPE to prevent program termination when client disconnects
	signal(SIGPIPE, SIG_IGN);

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		clog_e("socket error: %s\n", strerror(errno));
		return NULL;
	}

	int reuseaddr = 1;

	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
		   sizeof(reuseaddr));

	uint16_t port = RTSP_PORT;
	struct sockaddr_in addr = { .sin_family = AF_INET,
				    .sin_addr.s_addr = htonl(INADDR_ANY),
				    .sin_port = htons(port) };

	if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		clog_e("bind error: %s\n", strerror(errno));
		close(listen_fd);
		return NULL;
	}

	if (listen(listen_fd, RTSP_MAX_CLIENTS * 2) < 0) {
		clog_e("listen error: %s\n", strerror(errno));
		close(listen_fd);
		return NULL;
	}

	if (set_nonblock(listen_fd) == -1) {
		clog_e("set_nonblock error: %s\n", strerror(errno));
		close(listen_fd);
		return NULL;
	}

	FD_ZERO(&all_set);
	FD_SET(listen_fd, &all_set);
	max_fd = listen_fd;

	char *msg_buf = (char *)calloc(RTSP_MSG_BUF_SIZE, sizeof(char));

	clog_i("rtsp server listening on port %d\n", port);

	while (rtsp_server_thread_run) {
		r_set = all_set;

		struct timeval timeout = { 0, 500 * 1000 }; // 500ms

		if (select(max_fd + 1, &r_set, NULL, NULL, &timeout) == -1) {
			if (errno == EINTR)
				continue;
			clog_e("select error: %s\n", strerror(errno));
			break;
		}

		if (FD_ISSET(listen_fd, &r_set)) {
			while (1) {
				struct sockaddr_in cli;
				socklen_t len = sizeof(cli);
				int conn_fd =
					accept(listen_fd,
					       (struct sockaddr *)&cli, &len);

				if (conn_fd < 0) {
					if (errno == EAGAIN ||
					    errno == EWOULDBLOCK)
						break;
					clog_e("accept error: %s\n",
					       strerror(errno));
					continue;
				}

				clog_i("new rtsp connection %s:%d, fd=%d\n",
				       inet_ntoa(cli.sin_addr),
				       ntohs(cli.sin_port), conn_fd);

				if (set_nonblock(conn_fd) == -1) {
					clog_e("fcntl conn_fd error: %s\n",
					       strerror(errno));
					close(conn_fd);
					continue;
				}

				int send_buf_size = RTP_OVER_TCP_SEND_BUF_SIZE;

				if (setsockopt(conn_fd, SOL_SOCKET, SO_SNDBUF,
					&send_buf_size, sizeof(send_buf_size)) < 0) {
					clog_e("setsockopt SO_SNDBUF error: %s\n", strerror(errno));
					close(conn_fd);
					continue;
				}

				for (i = 0; i < RTSP_MAX_CLIENTS; ++i) {
					if (rtsp_sessions[i] != NULL) {
						continue;
					}

					rtsp_sessions[i] =
						(rtsp_session_t *)calloc(
							1,
							sizeof(rtsp_session_t));
					rtsp_sessions[i]->state =
						RTSP_STATE_IDLE;
					rtsp_sessions[i]->client_fd = conn_fd;
					rtsp_sessions[i]->session_id = -1;
					snprintf(rtsp_sessions[i]->clientip_str,
						 sizeof(rtsp_sessions[i]
								->clientip_str),
						 "%s", inet_ntoa(cli.sin_addr));
					break;
				}

				if (i == RTSP_MAX_CLIENTS) {
					clog_e("too many clients, closing fd=%d\n",
					       conn_fd);
					close(conn_fd);
					continue;
				}

				FD_SET(conn_fd, &all_set);
				if (conn_fd > max_fd)
					max_fd = conn_fd;
			}
		}

		for (i = 0; i < RTSP_MAX_CLIENTS; ++i) {
			if (rtsp_sessions[i] == NULL) {
				continue;
			}

			int ret = 0;
			int fd = rtsp_sessions[i]->client_fd;

			if (fd <= 0)
				continue;
			if (!FD_ISSET(fd, &r_set))
				continue;

			int session_id = rtsp_sessions[i]->session_id;

			if (session_id >= 0) {
				pthread_mutex_lock(
					&rtsp_hdl[session_id].mutex); // can be optimize
			}
			ret = do_rtsp_msg_cmd(rtsp_sessions[i], msg_buf);
			if (ret < 0) {
				FD_CLR(fd, &all_set);
				close(fd);
				release_session(rtsp_sessions[i]);
			}
			if (session_id >= 0) {
				pthread_mutex_unlock(
					&rtsp_hdl[session_id].mutex);
			}
		}
	}

	clog_i("rtsp server stopped\n");

	for (i = 0; i < RTSP_MAX_CLIENTS; ++i) {
		if (rtsp_sessions[i] != NULL) {
			close(rtsp_sessions[i]->client_fd);
			release_session(rtsp_sessions[i]);
		}
	}

	if (msg_buf)
		free(msg_buf);
	close(listen_fd);

	return NULL;
}

static int writev_rtp_packet(rtsp_session_t *session, struct iovec *iov, int iovcnt)
{
	int ret = 0;
	fd_set write_fds;
	struct timeval timeout;

	if (session->client_fd <= 0) {
		clog_e("Invalid client_fd: %d\n", session->client_fd);
		return -1;
	}

	while (1) {
		ret = writev(session->client_fd, iov, iovcnt);
		if (ret >= 0) {
			break;
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			FD_ZERO(&write_fds);
			FD_SET(session->client_fd, &write_fds);
			timeout.tv_sec = 0;
			timeout.tv_usec = 500 * 1000; // 500ms

			int select_ret = select(session->client_fd + 1, NULL, &write_fds, NULL, &timeout);

			if (select_ret < 0) {
				if (errno == EINTR) {
					continue;
				}
				clog_e("select error: %s\n", strerror(errno));
				return -1;
			} else if (select_ret == 0) {
				clog_e("writev timeout waiting for fd to be writable\n");
				return -1;
			}
		} else if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN) {
			clog_i("Client disconnected during send: %s\n", strerror(errno));
			return -1;
		} else {
			clog_e("writev error: %s\n", strerror(errno));
			return -1;
		}
	}

	return ret;
}

static int rtp_send_h26x_frame(rtsp_session_t *session,
			       rtsp_codec_type_e codec_type, uint8_t *frame,
			       int frame_size, uint64_t ts)
{
	int ret = 0;
	uint8_t payload_type = 0;
	//uint32_t vpts = (1000 / 25) * (RTP_CLOCK / 1000); // fps = 25
	uint64_t diff_ms = (ts - session->rtp_session->last_time_stamp) /
			   1000; // convert us to ms

	session->rtp_session->last_time_stamp = ts;
	if (diff_ms > 2000) {
		diff_ms = 40;
	}

	// Calculate RTP timestamp based on the difference in milliseconds
	// and the RTP clock rate (90kHz)
	uint32_t vpts = (diff_ms * RTP_CLOCK + 500) / 1000;

	clog_d("vpts: %d\n", vpts);

	if (frame[0] == 0 && frame[1] == 0 && frame[2] == 1) {
		frame += 3;
		frame_size -= 3;
	} else if (frame[0] == 0 && frame[1] == 0 && frame[2] == 0 &&
		   frame[3] == 1) {
		frame += 4;
		frame_size -= 4;
	} else {
		clog_e("invalid frame: %d,%d,%d,%d\n", frame[0], frame[1],
		       frame[2], frame[3]);
		return -1;
	}

	rtp_over_tcp_hdr_t over_tcp_hdr;
	rtp_hdr_t rtp_hdr;

	over_tcp_hdr.magic = 0x24; // '$'
	over_tcp_hdr.chn = RTP_CHANNEL;

	rtp_hdr.vpxcc = 0x2 << 6; // V=2
	rtp_hdr.vpxcc |= 0 << 5; // P=0
	rtp_hdr.vpxcc |= 0 << 4; // X=0
	rtp_hdr.vpxcc |= 0; // CC=0
	rtp_hdr.ssrc = htonl(session->rtp_session->ssrc);
	rtp_hdr.mpt = 0x0;

	// Set payload type based on codec
	if (codec_type == RTSP_CODEC_H265) {
		rtp_hdr.mpt |= (98 & 0x7F); // payload type 98 for H265
		payload_type =
			(frame[0] & 0x7E) >> 1; // H265 NALU type is in bits 1-6
	} else {
		rtp_hdr.mpt |= (96 & 0x7F); // payload type 96 for H264
		payload_type = frame[0] & 0x1F; // H264 NALU type is in bits 0-4
	}

	session->rtp_session->time_stamp += vpts;
	rtp_hdr.ts = htonl(session->rtp_session->time_stamp);

	int total_len =
		sizeof(rtp_over_tcp_hdr_t) + sizeof(rtp_hdr_t) + frame_size;

	if (total_len <= MTU) {
		rtp_hdr.mpt |= (0x1 << 7); // marker bit set
		rtp_hdr.seq = htons(session->rtp_session->seq_num++);
		over_tcp_hdr.len = htons(
			(uint16_t)(total_len - sizeof(rtp_over_tcp_hdr_t)));

		struct iovec iov[3];
		iov[0].iov_base = &over_tcp_hdr;
		iov[0].iov_len = sizeof(rtp_over_tcp_hdr_t);
		iov[1].iov_base = &rtp_hdr;
		iov[1].iov_len = sizeof(rtp_hdr_t);
		iov[2].iov_base = (void *)frame;
		iov[2].iov_len = frame_size;

		clog_d("Sending RTP packet: len=%d, seq=%d, ts=%d, codec=%s\n",
		       total_len, ntohs(rtp_hdr.seq), ntohl(rtp_hdr.ts),
		       codec_type == RTSP_CODEC_H265 ? "H265" : "H264");

		ret = writev_rtp_packet(session, iov, 3);
		if (ret < 0 || ret != total_len) {
			clog_e("Failed to send RTP data: %d, total len: %d\n",
			       ret, total_len);
			return -1;
		}
	} else {
		// Fragmentation for large frames
		uint8_t fu_indicator;
		const uint8_t *payload;
		int payload_size;
		int header_size;
		int fu_headers_size;

		if (codec_type == RTSP_CODEC_H265) {
			// H265 FU fragmentation
			fu_indicator =
				(frame[0] & 0x81) | (49 << 1); // Type 49 for FU
			payload = &frame[2]; // skip NALU header (2 bytes)
			payload_size = frame_size - 2;
			fu_headers_size = 3; // 3 bytes for H265 FU headers
		} else {
			// H264 FU-A fragmentation
			fu_indicator =
				(frame[0] & 0xE0) | 28; // Type 28 for FU-A
			payload =
				&frame[1]; // skip NALU header (1 byte for H264)
			payload_size = frame_size - 1;
			fu_headers_size = 2; // 2 bytes for H264 FU headers
		}

		header_size = sizeof(rtp_over_tcp_hdr_t) + sizeof(rtp_hdr_t) +
			      fu_headers_size;
		int max_payload_size = MTU - header_size;
		int sent_bytes = 0;
		int is_first_fragment = 1;

		while (sent_bytes < payload_size) {
			int current_payload_size =
				(payload_size - sent_bytes > max_payload_size) ?
					max_payload_size :
					(payload_size - sent_bytes);

			int is_last_fragment =
				(sent_bytes + current_payload_size >=
				 payload_size);

			rtp_hdr.seq = htons(session->rtp_session->seq_num++);
			if (is_last_fragment) {
				rtp_hdr.mpt |=
					(0x1
					 << 7); // set marker bit for last fragment
			} else {
				rtp_hdr.mpt &= ~(0x1 << 7); // clear marker bit
			}

			uint8_t fu_header = 0;
			if (is_first_fragment) {
				fu_header |= (0x1 << 7); // S bit (start)
			}
			if (is_last_fragment) {
				fu_header |= (0x1 << 6); // E bit (end)
			}

			if (codec_type == RTSP_CODEC_H265) {
				fu_header |= (payload_type &
					      0x3F); // FuType for H265
			} else {
				fu_header |= (payload_type &
					      0x1F); // NALU Type for H264
			}

			uint8_t fu_headers[3];
			fu_headers[0] = fu_indicator;
			if (codec_type == RTSP_CODEC_H265) {
				fu_headers[1] = frame[1]; // H265 second byte
				fu_headers[2] = fu_header;
			} else {
				fu_headers[1] =
					fu_header; // H264 only needs 2 bytes
			}

			int fragment_total_len =
				header_size + current_payload_size;
			over_tcp_hdr.len =
				htons((uint16_t)(fragment_total_len -
						 sizeof(rtp_over_tcp_hdr_t)));

			struct iovec iov[4];
			iov[0].iov_base = &over_tcp_hdr;
			iov[0].iov_len = sizeof(rtp_over_tcp_hdr_t);
			iov[1].iov_base = &rtp_hdr;
			iov[1].iov_len = sizeof(rtp_hdr_t);
			iov[2].iov_base = fu_headers;
			iov[2].iov_len = fu_headers_size;
			iov[3].iov_base = (void *)(payload + sent_bytes);
			iov[3].iov_len = current_payload_size;

			clog_d("Sending FU fragment: seq=%d, S=%d, E=%d, size=%d, total_sent=%d/%d, codec=%s\n",
			       ntohs(rtp_hdr.seq), is_first_fragment,
			       is_last_fragment, current_payload_size,
			       sent_bytes + current_payload_size, payload_size,
			       codec_type == RTSP_CODEC_H265 ? "H265" : "H264");

			ret = writev_rtp_packet(session, iov, 4);
			if (ret < 0 || ret != fragment_total_len) {
				clog_e("Failed to send RTP FU fragment: %d, total len: %d\n",
					ret, fragment_total_len);
				return -1;
			}

			sent_bytes += current_payload_size;
			is_first_fragment = 0;
		}
	}

	return 0;
}

int cvi_rtsp_send_frame(int id, cvi_rtsp_frame_t *frame)
{
	if (id < 0 || id >= RTSP_MAX_CLIENTS) {
		clog_e("Invalid session ID: %d\n", id);
		return -1;
	}

	if (frame == NULL || frame->data == NULL || frame->size <= 0) {
		clog_e("Invalid frame parameters\n");
		return -1;
	}

	pthread_mutex_lock(&rtsp_hdl[id].mutex);

	if (!rtsp_hdl[id].is_init) {
		pthread_mutex_unlock(&rtsp_hdl[id].mutex);
		clog_e("Session %d not init\n", id);
		return -1;
	}

	rtsp_session_t *session = rtsp_hdl[id].session;
	if (session == NULL) {
		pthread_mutex_unlock(&rtsp_hdl[id].mutex);
		clog_d("Session %d is NULL\n", id);
		return -1;
	}

	if (session->state != RTSP_STATE_PLAYING) {
		pthread_mutex_unlock(&rtsp_hdl[id].mutex);
		clog_d("Session %d not in PLAYING state\n", id);
		return -1;
	}

	if (session->client_fd <= 0) {
		pthread_mutex_unlock(&rtsp_hdl[id].mutex);
		clog_d("Session %d has invalid client_fd: %d\n", id, session->client_fd);
		return -1;
	}

	int ret = rtp_send_h26x_frame(session, rtsp_hdl[id].video_info.codec_type,
				    frame->data, frame->size, frame->ts);

	if (ret < 0) {
		clog_d("Failed to send frame to session %d, client may have disconnected\n", id);
	}

	pthread_mutex_unlock(&rtsp_hdl[id].mutex);
	return ret;
}

int cvi_rtsp_init(int id, cvi_rtsp_cfg_t *cfg)
{
	if (id < 0 || id >= RTSP_MAX_CLIENTS) {
		clog_e("Invalid session ID: %d\n", id);
		return -1;
	}

	if (cfg == NULL) {
		clog_e("Invalid config pointer\n");
		return -1;
	}

	if (rtsp_hdl[id].is_init) {
		clog_e("Session %d already init\n", id);
		return -1;
	}

	// Ignore SIGPIPE globally to prevent application termination
	signal(SIGPIPE, SIG_IGN);

	if (rtsp_server_thread_run == 0) {
		rtsp_server_thread_run = 1;
		if (pthread_create(&rtsp_server_thread, NULL, rtsp_server_loop,
				   NULL) != 0) {
			clog_e("Failed to create RTSP server thread: %s\n",
			       strerror(errno));
			rtsp_server_thread_run = 0;
			return -1;
		}
	} else {
		rtsp_server_thread_run++;
	}

	rtsp_hdl[id].is_init = 1;
	rtsp_hdl[id].video_info = cfg->video_info;
	rtsp_hdl[id].event_cb = cfg->event_cb;
	pthread_mutex_init(&rtsp_hdl[id].mutex, NULL);

	clog_i("\n\nRTSP URL: rtsp://%s:%d/%s%d\n\n", "127.0.0.1", RTSP_PORT,
	       RTSP_SESSION_PREFIX, id);

	return 0;
}

int cvi_rtsp_deinit(int id)
{
	if (id < 0 || id >= RTSP_MAX_CLIENTS) {
		clog_e("Invalid session ID: %d\n", id);
		return -1;
	}

	if (!rtsp_hdl[id].is_init) {
		clog_e("Session %d not init\n", id);
		return -1;
	}

	pthread_mutex_lock(&rtsp_hdl[id].mutex);
	rtsp_hdl[id].is_init = 0;
	rtsp_hdl[id].video_info = (cvi_rtsp_video_info_t){ 0 };
	rtsp_hdl[id].event_cb = (cvi_rtsp_event_callback_t){ 0 };
	pthread_mutex_unlock(&rtsp_hdl[id].mutex);
	pthread_mutex_destroy(&rtsp_hdl[id].mutex);

	rtsp_server_thread_run--;
	if (rtsp_server_thread_run <= 0) {
		pthread_join(rtsp_server_thread, NULL);
	}

	return 0;
}
