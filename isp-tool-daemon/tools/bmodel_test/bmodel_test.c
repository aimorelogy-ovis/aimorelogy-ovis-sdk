#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "bmruntime_interface.h"

void print_usage(const char *prog_name)
{
	printf("Usage: %s <model_path> [loop_count]\n", prog_name);
	printf("  model_path  - Path to the bmodel file\n");
	printf("  loop_count  - Number of inference loops (default: 1)\n");
	printf("\nExample:\n");
	printf("  %s ./model.bmodel 10\n", prog_name);
}

const char *dtype_to_string(bm_data_type_t dtype)
{
	switch (dtype) {
	case BM_FLOAT32:
		return "FLOAT32";
	case BM_FLOAT16:
		return "FLOAT16";
	case BM_INT8:
		return "INT8";
	case BM_UINT8:
		return "UINT8";
	case BM_INT16:
		return "INT16";
	case BM_UINT16:
		return "UINT16";
	case BM_INT32:
		return "INT32";
	case BM_UINT32:
		return "UINT32";
	case BM_BFLOAT16:
		return "BF16";
	case BM_INT4:
		return "INT4";
	case BM_UINT4:
		return "UINT4";
	default:
		return "UNKNOWN";
	}
}

void print_tensor_info(const char *prefix, const bm_tensor_t *tensor, int index, const char *name)
{
	char shape_str[128];
	int pos = 0;

	pos += snprintf(shape_str + pos, sizeof(shape_str) - pos, "[");
	for (int i = 0; i < tensor->shape.num_dims; i++) {
		pos += snprintf(shape_str + pos, sizeof(shape_str) - pos, "%d", tensor->shape.dims[i]);
		if (i < tensor->shape.num_dims - 1)
			pos += snprintf(shape_str + pos, sizeof(shape_str) - pos, ",");
	}
	snprintf(shape_str + pos, sizeof(shape_str) - pos, "]");

	printf("%s[%-2d] %-24s dtype=%-8s shape=%-20s addr=0x%08llx, size=%u bytes\n",
		prefix, index,
		name ? name : "",
		dtype_to_string(tensor->dtype),
		shape_str,
		(unsigned long long)tensor->device_mem.u.device.device_addr,
		tensor->device_mem.size);
}

void print_model_info(const char *net_name, const bm_net_info_t *net_info)
{
	printf("\n========== Model Information ==========\n");
	printf("Network name: %s\n", net_name);
	printf("Input tensors: %d\n", net_info->input_num);
	printf("Output tensors: %d\n", net_info->output_num);
	printf("Number of stages: %d\n", net_info->stage_num);
	printf("=====================================\n\n");
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		print_usage(argv[0]);
		return -1;
	}

	const char *model_path = argv[1];
	int loop_count = 1;

	if (argc >= 3) {
		loop_count = atoi(argv[2]);
		if (loop_count <= 0) {
			printf("Error: loop_count must be a positive integer\n");
			return -1;
		}
	}

	printf("Model path: %s\n", model_path);
	printf("Loop count: %d\n\n", loop_count);

	// Check if model file exists
	if (access(model_path, F_OK) != 0) {
		printf("Error: Model file not found: %s\n", model_path);
		return -1;
	}

	// Request bm_handle
	bm_handle_t bm_handle;
	bm_status_t status = bm_dev_request(&bm_handle, 0);

	if (status != BM_SUCCESS) {
		printf("Error: Failed to request BM device\n");
		return -1;
	}
	printf("BM device initialized successfully\n");

	// Create bmruntime
	void *p_bmrt = bmrt_create(bm_handle);

	if (p_bmrt == NULL) {
		printf("Error: Failed to create bmruntime\n");
		bm_dev_free(bm_handle);
		return -1;
	}
	printf("BMRuntime created successfully\n");

	// Load bmodel
	struct timeval tv_load_start, tv_load_end;

	gettimeofday(&tv_load_start, NULL);

	bool ret = bmrt_load_bmodel(p_bmrt, model_path);

	if (!ret) {
		printf("Error: Failed to load bmodel: %s\n", model_path);
		bmrt_destroy(p_bmrt);
		bm_dev_free(bm_handle);
		return -1;
	}

	gettimeofday(&tv_load_end, NULL);
	int load_time_us = (tv_load_end.tv_sec - tv_load_start.tv_sec) * 1000000 +
					   (tv_load_end.tv_usec - tv_load_start.tv_usec);
	printf("Model loaded successfully (time: %d us, %.2f ms)\n\n",
		   load_time_us, load_time_us / 1000.0);

	// Get network names
	const char **net_names = NULL;

	bmrt_get_network_names(p_bmrt, &net_names);
	if (net_names == NULL || net_names[0] == NULL) {
		printf("Error: Failed to get network names\n");
		bmrt_destroy(p_bmrt);
		bm_dev_free(bm_handle);
		return -1;
	}

	// Get network info
	const bm_net_info_t *net_info = bmrt_get_network_info(p_bmrt, net_names[0]);

	if (net_info == NULL) {
		printf("Error: Failed to get network info\n");
		free(net_names);
		bmrt_destroy(p_bmrt);
		bm_dev_free(bm_handle);
		return -1;
	}

	// Print model information
	print_model_info(net_names[0], net_info);

	// Allocate input tensors
	bm_tensor_t *input_tensors = (bm_tensor_t *)malloc(net_info->input_num * sizeof(bm_tensor_t));

	if (input_tensors == NULL) {
		printf("Error: Failed to allocate input tensors\n");
		free(net_names);
		bmrt_destroy(p_bmrt);
		bm_dev_free(bm_handle);
		return -1;
	}

	printf("Allocating input tensors...\n");
	for (int i = 0; i < net_info->input_num; i++) {
		input_tensors[i].dtype = net_info->input_dtypes[i];
		input_tensors[i].shape = net_info->stages[0].input_shapes[i];
		input_tensors[i].st_mode = BM_STORE_1N;

		status = bm_malloc_device_byte(bm_handle, &input_tensors[i].device_mem,
									   net_info->max_input_bytes[i]);
		if (status != BM_SUCCESS) {
			printf("Error: Failed to allocate device memory for input[%d]\n", i);
			// Free already allocated memory
			for (int j = 0; j < i; j++) {
				bm_free_device(bm_handle, input_tensors[j].device_mem);
			}
			free(input_tensors);
			free(net_names);
			bmrt_destroy(p_bmrt);
			bm_dev_free(bm_handle);
			return -1;
		}
		print_tensor_info("Input", &input_tensors[i], i, net_info->input_names[i]);
	}

	// Allocate output tensors
	bm_tensor_t *output_tensors = (bm_tensor_t *)malloc(net_info->output_num * sizeof(bm_tensor_t));

	if (output_tensors == NULL) {
		printf("Error: Failed to allocate output tensors\n");
		for (int i = 0; i < net_info->input_num; i++) {
			bm_free_device(bm_handle, input_tensors[i].device_mem);
		}
		free(input_tensors);
		free(net_names);
		bmrt_destroy(p_bmrt);
		bm_dev_free(bm_handle);
		return -1;
	}

	printf("\nAllocating output tensors...\n");
	for (int i = 0; i < net_info->output_num; i++) {
		output_tensors[i].dtype = net_info->output_dtypes[i];
		output_tensors[i].shape = net_info->stages[0].output_shapes[i];
		output_tensors[i].st_mode = BM_STORE_1N;

		status = bm_malloc_device_byte(bm_handle, &output_tensors[i].device_mem,
									   net_info->max_output_bytes[i]);
		if (status != BM_SUCCESS) {
			printf("Error: Failed to allocate device memory for output[%d]\n", i);
			// Free already allocated memory
			for (int j = 0; j < i; j++) {
				bm_free_device(bm_handle, output_tensors[j].device_mem);
			}
			free(output_tensors);
			for (int j = 0; j < net_info->input_num; j++) {
				bm_free_device(bm_handle, input_tensors[j].device_mem);
			}
			free(input_tensors);
			free(net_names);
			bmrt_destroy(p_bmrt);
			bm_dev_free(bm_handle);
			return -1;
		}
		print_tensor_info("Output", &output_tensors[i], i, net_info->output_names[i]);
	}

	// Run inference loop
	printf("\n========== Starting Inference ==========\n");
	printf("Running %d inference loop(s)...\n\n", loop_count);

	long long total_time_us = 0;
	long long min_time_us = 0x7FFFFFFFFFFFFFFFLL;
	long long max_time_us = 0;

	for (int loop = 0; loop < loop_count; loop++) {
		struct timeval tv_start, tv_end;

		gettimeofday(&tv_start, NULL);

		ret = bmrt_launch_tensor_ex(p_bmrt, net_names[0],
									input_tensors, net_info->input_num,
									output_tensors, net_info->output_num,
									true, false);
		if (!ret) {
			printf("Error: Inference failed at loop %d\n", loop);
			break;
		}

		status = bm_thread_sync(bm_handle);
		if (status != BM_SUCCESS) {
			printf("Error: Thread sync failed at loop %d\n", loop);
			break;
		}

		gettimeofday(&tv_end, NULL);

		long long time_us = (tv_end.tv_sec - tv_start.tv_sec) * 1000000LL +
							(tv_end.tv_usec - tv_start.tv_usec);

		total_time_us += time_us;
		if (time_us < min_time_us)
			min_time_us = time_us;
		if (time_us > max_time_us)
			max_time_us = time_us;

		printf("Loop[%d]: %lld us (%.3f ms)\n", loop, time_us, time_us / 1000.0);
	}

	// Print statistics
	printf("\n========== Inference Statistics ==========\n");
	printf("Total loops: %d\n", loop_count);
	printf("Total time: %lld us (%.3f ms)\n", total_time_us, total_time_us / 1000.0);
	if (loop_count > 0) {
		printf("Average time: %.3f us (%.3f ms)\n",
			   (double)total_time_us / loop_count,
			   (double)total_time_us / loop_count / 1000.0);
		printf("Min time: %lld us (%.3f ms)\n", min_time_us, min_time_us / 1000.0);
		printf("Max time: %lld us (%.3f ms)\n", max_time_us, max_time_us / 1000.0);
		printf("FPS: %.2f\n", 1000000.0 * loop_count / total_time_us);
	}
	printf("=========================================\n");

	// Cleanup
	for (int i = 0; i < net_info->output_num; i++) {
		bm_free_device(bm_handle, output_tensors[i].device_mem);
	}
	free(output_tensors);

	for (int i = 0; i < net_info->input_num; i++) {
		bm_free_device(bm_handle, input_tensors[i].device_mem);
	}
	free(input_tensors);

	free(net_names);
	bmrt_destroy(p_bmrt);
	bm_dev_free(bm_handle);

	printf("\nTest completed successfully!\n");
	return 0;
}
