#include <dfs_posix.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <stdlib.h>

#define AUDIO_PLAY_DEV_NAME "i2s3"

static void audio_play(int argc, char **argv)
{
	if (argc < 4) {
		rt_kprintf("Usage: %s <sample_rate> <channels> <file_name>\n", argv[0]);
		rt_kprintf("Example: %s 8000 2 play.raw\n", argv[0]);
		return;
	}

	rt_thread_t debug_tid;
	rt_size_t read_size = 0;
	rt_uint32_t sample_rate = atoi(argv[1]);
	rt_uint16_t channels = atoi(argv[2]);
	const char *file_name = argv[3];
	rt_uint32_t period_byte = 1024;
	// period_size * channels * 2; rt_audio_buf_info info->block_size =
	// I2S_PERIOD_SIZE (1024);
	rt_device_t audio_dev = rt_device_find(AUDIO_PLAY_DEV_NAME);

	if (!audio_dev) {
		rt_kprintf("Audio device not found!\n");
		return;
	}

	rt_device_open(audio_dev, RT_DEVICE_OFLAG_WRONLY);

	struct rt_audio_caps caps = {
		.main_type = AUDIO_TYPE_OUTPUT,
		.sub_type = AUDIO_DSP_PARAM,
		.udata.config.samplerate = sample_rate,
		.udata.config.channels = channels,
		.udata.config.samplebits = 16,
	};
	rt_device_control(audio_dev, AUDIO_CTL_CONFIGURE, &caps);
	FILE *fd = fopen(file_name, "rb+");

	if (fd < 0) {
		rt_kprintf("Failed to open file: %s\n", file_name);
		return;
	}
	rt_uint8_t *buffer = rt_malloc(period_byte);

	if (!buffer) {
		rt_kprintf("Failed to allocate buffer!\n");
		fclose(fd);
		return;
	}

	rt_kprintf("Start playing...\n");

	while (1) {
		read_size = fread(buffer, 1, period_byte, fd);
		if (read_size > 0) {
			rt_device_write(audio_dev, 0, buffer, read_size);
		} else {
			break;
		}
	}

	rt_kprintf("Playback finished!\n");

	rt_free(buffer);
	fclose(fd);
	rt_device_close(audio_dev);
}

MSH_CMD_EXPORT(audio_play, "Play audio from file");
