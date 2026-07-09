import argparse
import os
import sys
import json
import time
import re
from glob import glob

main_func_template = \
"""
#include <stdio.h>
#include <stdlib.h>
#include "isp_set_pqjson.h"
#include "cvi_isp.h"

int main(int argc, char **argv)
{{
	int ret = 0;
	int dev_num = 0;

	if (argc > 1) {{
		dev_num = atoi(argv[1]);
	}} else {{
		dev_num = 1;
	}}

	if (dev_num < 1) {{
		dev_num = 1;
	}}

	for (int pipe = 0; pipe < dev_num; ++pipe) {{
        printf("---- pipe: %d ----\\n", pipe);
		ret = CVI_ISP_MemInit(pipe);

		if (ret != 0) {{
			printf("CVI_ISP_MemInit fail, pipe: %d!\\n", pipe);
			return -1;
		}}
{isp_set_pqjson_code}
		CVI_ISP_Exit(pipe);
	}}

	return ret;
}}

"""

sensor_pqjson_block_template = \
"""
		printf("# sensor: {sensor_name} #\\n");
		ret = isp_set_pqjson_sdr_{sensor_name}(pipe);
		ret = isp_set_pqjson_wdr_{sensor_name}(pipe);
"""


api_sdr = "isp_set_pqjson_sdr"
api_wdr = "isp_set_pqjson_wdr"
api_header_file_name = "isp_set_pqjson.h"
main_name = "main.c"
api_suffix = "(int ViPipe);"

def gen_test_code(args):
    isp_tuning_dir = args.isp_tuning_dir
    test_code_save_dir = args.test_code_save_dir

    if not os.path.exists(isp_tuning_dir):
        print(f"{isp_tuning_dir} not exist! Exit...")
        exit(1)

    if not os.path.exists(test_code_save_dir):
        os.makedirs(test_code_save_dir)

    isp_tuning_dir = os.path.abspath(isp_tuning_dir)
    test_code_save_dir = os.path.abspath(test_code_save_dir)

    sensor_ls = os.listdir(isp_tuning_dir)
    sensor_ls = [sensor for sensor in sensor_ls if os.path.isdir(os.path.join(isp_tuning_dir, sensor))]


    api_name_ls = []
    isp_set_pqjson_code = ""

    for sensor in sensor_ls:
        sensor_tuning_path = os.path.join(isp_tuning_dir, sensor)
        c_files = glob(f"{sensor_tuning_path}/*.c")

        # copy the c file
        for c_file in c_files:
            with open(c_file, "r") as f:
                content = f.read()
                new_content = ""
                if api_sdr in content:
                    new_content = content.replace(api_sdr, f"{api_sdr}_{sensor}")
                    api_name_ls.append(f"int {api_sdr}_{sensor}{api_suffix}")
                elif api_wdr in content:
                    new_content = content.replace(api_wdr, f"{api_wdr}_{sensor}")
                    api_name_ls.append(f"int {api_wdr}_{sensor}{api_suffix}")

                if new_content:
                    c_file_name = os.path.basename(c_file)
                    new_c_file_path = os.path.join(test_code_save_dir, c_file_name)
                    with open(new_c_file_path, "w") as f:
                        f.write(new_content)

        isp_set_pqjson_code += sensor_pqjson_block_template.format(
                sensor_name = sensor
        )

    # deal the header
    api_header_path = os.path.join(test_code_save_dir, api_header_file_name)
    with open(api_header_path, "w") as f:
        f.write("\n".join(api_name_ls))

    # generate the main.c
    main_path = os.path.join(test_code_save_dir, main_name)
    with open(main_path, "w") as f:
        f.write(main_func_template.format(
            isp_set_pqjson_code = isp_set_pqjson_code
        ))

if __name__ == "__main__":
    print(f"------ generate pqjson test code ------")

    parser = argparse.ArgumentParser()

    parser.add_argument('--isp_tuning_dir', type=str,
                        default=f'../cv184x/src')

    parser.add_argument('--test_code_save_dir', type=str,
                        default='test')

    args = parser.parse_args()

    gen_test_code(args)

