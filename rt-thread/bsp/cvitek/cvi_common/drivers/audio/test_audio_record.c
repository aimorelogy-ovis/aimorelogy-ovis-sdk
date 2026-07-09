#include <dfs_posix.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <stdlib.h>

#define AUDIO_RECORD_DEV_NAME "i2s0"

static void audio_record(int argc, char **argv)
{
	if (argc < 5) {
		rt_kprintf("Usage: %s <sample_rate> <chn> <file> <duration>\n", argv[0]);
		rt_kprintf("Example: %s 8000 2 record.pcm 10\n", argv[0]);
		return;
	}

	rt_size_t read_byte = 0;
	rt_uint32_t sample_rate = atoi(argv[1]);
	rt_uint16_t channels = atoi(argv[2]);
	const char *file_name = argv[3];
	rt_uint32_t duration = atoi(argv[4]);
	rt_uint32_t period_byte = 1024; // I2S_PERIOD_SIZE
	rt_device_t audio_dev = rt_device_find(AUDIO_RECORD_DEV_NAME);

	if (!audio_dev) {
		rt_kprintf("Audio device not found!\n");
		return;
	}

	rt_device_open(audio_dev, RT_DEVICE_OFLAG_RDONLY);

	struct rt_audio_caps caps = {
		.main_type = AUDIO_TYPE_INPUT,
		.sub_type = AUDIO_DSP_PARAM,
		.udata.config.samplerate = sample_rate,
		.udata.config.channels = channels,
		.udata.config.samplebits = 16,
	};
	rt_device_control(audio_dev, AUDIO_CTL_CONFIGURE, &caps);
	FILE *fd = fopen(file_name, "wb+");

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
	rt_int32_t total_byte = sample_rate * duration * channels * 2;

	rt_kprintf("Start recording...total_byte:%d\n", total_byte);
	while (total_byte > 0) {
		read_byte = rt_device_read(audio_dev, 0, buffer, period_byte);
		if (read_byte > 0) {
			fwrite(buffer, 1, read_byte, fd);
			total_byte -= read_byte;
		}
	}

	rt_kprintf("Recording finished!\n");

	rt_free(buffer);
	fclose(fd);
	rt_device_close(audio_dev);
}

MSH_CMD_EXPORT(audio_record, "Record audio to file");
