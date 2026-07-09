import argparse
import os
import sys
import json
import time
import re
from glob import glob

MAX_VI_PIPE_NUM = 4

class PRINT:
    """
    for print the information
    """
    w_e_log = ""
    log_total = ""
    is_error = is_warn = is_info = 1
    is_debug = is_verbose = 0
    def __init__(self):
        pass

    @classmethod
    def error(cls, error_message, is_exit=False):
        """
        print error message
        """
        e_log = f"[error] {error_message}"
        PRINT.log_add_we(e_log)
        PRINT.log_add_total(e_log)
        if cls.is_error:
            print(e_log)
        if is_exit:
            sys.exit(1)

    @classmethod
    def warn(cls, warning_message):
        """
        print warning message
        """
        w_log = f"[warn] {warning_message}"
        PRINT.log_add_we(w_log)
        PRINT.log_add_total(w_log)
        if cls.is_warn:
            print(w_log)

    @classmethod
    def info(cls, info_message):
        """
        print info message
        """
        info_log = f"[info] {info_message}"
        PRINT.log_add_total(info_log)
        if cls.is_info:
            print(info_log)

    @classmethod
    def debug(cls, debug_message):
        """
        print debug message
        """
        d_log = f"[debug] {debug_message}"
        PRINT.log_add_total(d_log)
        if cls.is_debug:
            print(d_log)

    @classmethod
    def verbose(cls, verbose_message):
        """
        print debug message
        """
        v_log = f"[verbose] {verbose_message}"
        PRINT.log_add_total(v_log)
        if cls.is_verbose:
            print(v_log)

    @classmethod
    def log_add_we(cls, log_content):
        PRINT.w_e_log += f"{log_content}\n"

    @classmethod
    def log_add_total(cls, log_content):
        PRINT.log_total += f"{log_content}\n"

    @classmethod
    def write_log(cls, marker):
        log_dir_path = "tmp"
        if not os.path.exists(log_dir_path):
            os.makedirs(log_dir_path)
        w_e_log_path = os.path.join(log_dir_path, f"{marker}_pqjson_warning_error.log")
        i_d_v_log_path = os.path.join(log_dir_path, f"{marker}_pqjson_total.log")

        with open(w_e_log_path, "w", encoding="utf-8") as f:
            f.write(PRINT.w_e_log)

        with open(i_d_v_log_path, "w", encoding="utf-8") as f:
            f.write(PRINT.log_total)

class PARAMS:
    """
    Get the pq parameters
    """
    def __init__(self, json_dir_path):
        self.json_dir_path = json_dir_path
        self.struct_method_outer = {}
        self.sensor_json_info = self._get_sensor_json_info()

    def _get_dict_from_json(self, pq_json_save_data_ls):
        param_val_dict = {}

        for module_dict in pq_json_save_data_ls:
            for _, path_val_ls in module_dict.items():
                PRINT.debug(f"deal module: {_}")
                for path_val in path_val_ls:
                    # here should check fist, if the path is the same in the previous
                    if path_val["PATH"] in param_val_dict:
                        PRINT.warn(f'{path_val["PATH"]} has already in the param_val_dict!')
                    param_val_dict.update({path_val["PATH"]: path_val["VALUE"]})
                    PRINT.verbose(f'{path_val["PATH"]}: {path_val["VALUE"]}')
        return param_val_dict

    def _get_struct_method_dict_from_json(self, json_data):
        # collect as much as possilbe method
        try:
            method_ls = json_data["JSONRPC"]["METHOD"]
        except:
            PRINT.error("can not get the jsonrpc method from the  json, exit!")

        self.struct_method_outer = {}

        for method_dict in method_ls:
            struct = method_dict["STRUCT"]
            set_method = method_dict["SET"]
            self.struct_method_outer.update({struct: set_method})

    def _get_sensor_json_info(self):
        if not os.path.exists(self.json_dir_path):
            return {}

        candicate_json_path_ls = glob(os.path.join(self.json_dir_path, "*.json"))

        if len(candicate_json_path_ls) == 0:
            return {}

        sensor_json_info = {}
        sensor_json_info_wdr = {}
        sensor_json_info_sdr = {}

        # name with sensorx
        for candicate_json_path in candicate_json_path_ls:
            for sensor_id in range(MAX_VI_PIPE_NUM):
                sensor_id_str = f"cam{sensor_id}"
                candicate_json_file_name = os.path.basename(candicate_json_path)
                if sensor_id_str in candicate_json_file_name:
                    with open(candicate_json_path, "r", encoding="utf-8") as f:
                        json_data = json.load(f)

                        if "SAVE DATA" not in json_data.keys():
                            continue

                        if "sdr" in candicate_json_path:
                            sensor_info_sdr = {sensor_id_str: {"json_path": candicate_json_file_name,
                                                           "params": self._get_dict_from_json(json_data["SAVE DATA"])}
                                           }
                            sensor_json_info_sdr.update(sensor_info_sdr)
                        elif "wdr" in candicate_json_path:
                            sensor_info_wdr = {sensor_id_str: {"json_path": candicate_json_file_name,
                                                           "params": self._get_dict_from_json(json_data["SAVE DATA"])}
                                           }
                            sensor_json_info_wdr.update(sensor_info_wdr)

                        # update the method
                        self._get_struct_method_dict_from_json(json_data)
        # name without sensorx, from the isp_tuning
        #for candicate_json_path in candicate_json_path_ls:
        #    sensor_id_str = f"cam0"
        #    candicate_json_file_name = os.path.basename(candicate_json_path)
        #    with open(candicate_json_path, "r", encoding="utf-8") as f:
        #        json_data = json.load(f)

        #        if "SAVE DATA" not in json_data.keys():
        #            continue

        #        if "sdr" in candicate_json_path:
        #            sensor_info_sdr = {sensor_id_str: {"json_path": candicate_json_file_name,
        #                                           "params": self._get_dict_from_json(json_data["SAVE DATA"])}
        #                           }
        #            sensor_json_info_sdr.update(sensor_info_sdr)
        #        elif "wdr" in candicate_json_path:
        #            sensor_info_wdr = {sensor_id_str: {"json_path": candicate_json_file_name,
        #                                           "params": self._get_dict_from_json(json_data["SAVE DATA"])}
        #                           }
        #            sensor_json_info_wdr.update(sensor_info_wdr)

        #        # update the method
        #        self._get_struct_method_dict_from_json(json_data)

        sensor_json_info = {"wdr": sensor_json_info_wdr, "sdr": sensor_json_info_sdr}
        return sensor_json_info

    def get_params_from_json(self):
        """
        public function to get the json information
        """
        return [
                self.sensor_json_info,
                self.struct_method_outer
                ]

class CODE:
    def __init__(self, param_info, profile_path, header_path, code_dir, code_path, sensor):
        # input
        #self.param_val_dict = param_info[0]["cam"]["params"]
        self.param_info = param_info[0]
        self.sensor_id_ls, self.sensor_id_v_ls = self._prob_sensor_id()
        self.sensor = sensor
        self.param_struct_method_dict = param_info[1]
        self.profile_path = profile_path
        self.header_path = header_path

        self.code_path = os.path.join(code_dir, code_path)

        if os.path.exists(self.code_path):
            os.remove(self.code_path)

        self.code_body_text = ""
        self.code_header = ""
        self.code_static_func = ""
        self.code_api = ""
        self.total_code = ""

    def _prob_sensor_id(self):
        sensor_name_ls = list(self.param_info.keys())
        sensor_id_ls = [int(sensor_name.strip("cam")) for sensor_name in sensor_name_ls]
        sensor_id_v_ls = []
        for i in range(MAX_VI_PIPE_NUM):
            if i not in sensor_id_ls:
                sensor_id_v_ls.append(i)
        sensor_id_ls.sort()
        sensor_id_v_ls.sort()
        # check sdr or wrd
        if len(sensor_name_ls) > 0 and "wdr" in self.param_info[sensor_name_ls[0]]["json_path"]:
            self.sdr_or_wdr = "wdr"
        else:
            self.sdr_or_wdr = "sdr"

        return sensor_id_ls, sensor_id_v_ls

    def _code_func_start(self, module_type, module_name, get_method):
        indent = self.indent(1)
        module_name = module_name.title()
        func_define_str = f"static void set{module_name}Params(VI_PIPE ViPipe)\n{{\n"
        func_define_str += f"{indent}UNUSED(ViPipe);\n"
        func_define_str += f"{indent}{module_type} attr = {{0}};\n\n"
        func_define_str += f"{indent}{get_method}(ViPipe, &attr);\n\n"
        self.code_body_text += func_define_str
        self.code_static_func += f"static void set{module_name}Params(VI_PIPE ViPipe);\n"

    def _construct_leaf_path(self, path_info_ls, pre_dims):
        leaf_path_ls = []
        for i, node_info in enumerate(path_info_ls):
            if i == 0:
                # Note: pq json, path start with the module structure type
                leaf_path_ls.append(node_info["type"][0])
            else:
                leaf_path_ls.append(node_info["name"])

        if pre_dims > 0:
            # string format for latter use
            leaf_path_ls[-2] += "[{}]"

        leaf_path = ".".join(leaf_path_ls)

        return leaf_path

    @classmethod
    def indent(cls, n):
        return "\t" * n

    @classmethod
    def newline(cls, n):
        return "\n" * n

    def _format_1d_array(self, value, indent=""):
        indent3 = indent + self.indent(1)
        indent4 = indent + self.indent(2)
        array = f"{{\n{indent4}"
        for i, val in enumerate(value, 1):
            if i == len(value):
                array += str(val) + f"\n{indent3}"
            elif i % 16 == 0:
                array += str(val) + f",\n{indent4}"
            else:
                array += str(val) + ", "

        array += "};\n"
        return array

    def _format_2d_array(self, rows, cols, value, indent=""):
        # note: value the 1d list
        indent1 = indent + self.indent(1)
        indent2 = indent + self.indent(2)
        main_part = ""
        for row in range(rows):
            row_str = ""
            for col in range(cols):
                ele = value[row * cols + col]
                if col == cols - 1:
                    row_str += str(ele)
                else:
                    row_str += str(ele) + ", "
            if row == rows - 1:
                row_str = f"{indent2}{{" + row_str + "}\n"
            else:
                row_str = f"{indent2}{{" + row_str + "},\n"

            main_part += row_str

        array = f"{{\n{main_part}{indent1}}};\n"

        return array

    def _code_line(self, data_type, data_var_name, data_dims, data_val, indent="\t"):
        line = ""
        if len(data_dims) == 0:
            line = f"{indent}{data_var_name} = {data_val};\n"

        elif len(data_dims) <= 2:
            var_name_init = data_var_name.replace(".", "_") + "_temp"
            var_name_init = var_name_init.replace("[", "_")
            var_name_init = var_name_init.replace("]", "_")
            if len(data_dims) == 1:
                line_init = f"{indent}{data_type} {var_name_init}[{data_dims[0].strip()}] = "
                line_init += self._format_1d_array(data_val, indent)
            else:
                rows = int(data_dims[0])
                cols = int(data_dims[1])
                line_init = f"{indent}{data_type} {var_name_init}[{rows}][{cols}] = "
                line_init += self._format_2d_array(rows, cols, data_val, indent)

            cpy_size = " * ".join(data_dims)
            line_mem_cpy = f"\n{indent}memcpy({data_var_name}, {var_name_init}, sizeof({data_type}) * {cpy_size});\n\n"
            line = line_init + line_mem_cpy
        else:
            PRINT.error("dims >= 3, no such dealing code, please this situation!", is_exit=True)

        return line

    def _code_case_line(self, data_type, data_var_name, data_dims, data_val_ls):
        indent = self.indent(1)
        case_block = f"\n{indent}switch(ViPipe) {{\n"

        for i, sensor_id in enumerate(self.sensor_id_ls):
            if i == 0:
                first_case_block_ls = self.sensor_id_v_ls + [i]
                first_case_block_ls.sort()
                for first_case_block_sensor_id in first_case_block_ls:
                    case_block += f"{indent}case {first_case_block_sensor_id}:\n"
                case_block += f"{indent}{{\n"
                case_block += self._code_line(data_type, data_var_name, data_dims, data_val_ls[i], self.indent(2))
                case_block += f"{indent}}}\n"
                case_block += f"{indent * 2}break;\n"
            else:
                case_block += f"{indent}case {sensor_id}:\n"
                case_block += f"{indent}{{\n"
                case_block += self._code_line(data_type, data_var_name, data_dims, data_val_ls[i], self.indent(2))
                case_block += f"{indent}}}\n"
                case_block += f"{indent * 2}break;\n"
        case_block += f"{indent}default:\n"
        case_block += f"{indent * 2}break;\n"

        case_block += f"{indent}}}\n"

        self.code_body_text += case_block

    def _get_leaf_path_val(self, leave_path):
        # val type: 0, all none, 1, all same, 2, different
        leaf_path_val_ls = []

        is_all_none = True
        is_all_same = True
        for i, sensor_id in enumerate(self.sensor_id_ls):
            sensor_name = "cam" + str(sensor_id)
            val = self.param_info[sensor_name]["params"].get(leave_path, None)
            leaf_path_val_ls.append(val)
            if val is not None:
                is_all_none = False

            if i > 0 and val != pre_val:
                is_all_same = False

            pre_val = val

        if is_all_none:
            val_type = 0
            PRINT.warn(f"path: {leave_path} can not find the value in pq json!")
        elif is_all_same:
            val_type = 1
        else:
            val_type = 2

        return leaf_path_val_ls, val_type

    def _code_func_body(self, path_info_ls):
        cur_node_info = path_info_ls[-1]
        cur_node_type = " ".join(cur_node_info["type"])
        cur_ndoe_dims = cur_node_info["dims"]

        # get the previous dims, int number
        if len(path_info_ls) >= 2 and path_info_ls[-2]["dims"]:
            pre_dims = int(path_info_ls[-2]["dims"][0])
        else:
            pre_dims = 0

        leaf_path = self._construct_leaf_path(path_info_ls, pre_dims)

        if pre_dims > 0:
            for i in range(pre_dims):
                leaf_path_i = leaf_path.format(i)

                leaf_path_val_ls, val_type = self._get_leaf_path_val(leaf_path_i)
                # replace strcut type with the attr
                cur_node_path = leaf_path_i.replace(path_info_ls[0]["type"][0], "attr")

                if val_type == 0:
                    continue

                elif val_type == 1:

                    self.code_body_text += self._code_line(cur_node_type, cur_node_path, cur_ndoe_dims, leaf_path_val_ls[0])
                else:
                    PRINT.debug(f"{cur_node_path}: value different between sensor")
                    self._code_case_line(cur_node_type, cur_node_path, cur_ndoe_dims, leaf_path_val_ls)
        else:

            leaf_path_val_ls, val_type = self._get_leaf_path_val(leaf_path)
            cur_node_path = leaf_path.replace(path_info_ls[0]["type"][0], "attr")

            if val_type == 0:
                return
            elif val_type == 1:
                self.code_body_text += self._code_line(cur_node_type, cur_node_path, cur_ndoe_dims, leaf_path_val_ls[0])
            else:
                self._code_case_line(cur_node_type, cur_node_path, cur_ndoe_dims, leaf_path_val_ls)

    def _code_func_end(self, set_method):
        indent = self.indent(1)
        set_func_str = f"\n{indent}{set_method}(ViPipe, &attr);\n"
        self.code_body_text += set_func_str
        self.code_body_text += "}\n\n"

    def _travel_profile_module_struct(self, node, path_info_ls):

        current_node_info = {"type": node["type"], "name": node["name"], "dims": node["dims"]}

        cur_path_info_ls = path_info_ls + [current_node_info]

        if "children" in node and len(node["children"]) > 0:
            for child in node["children"]:
                self._travel_profile_module_struct(child, cur_path_info_ls)
        else:
            self._code_func_body(cur_path_info_ls)

    def _travel_profile(self):
        """
        for profile
        """
        if not os.path.exists(self.profile_path):
            PRINT.error(f"{self.profile_path} not exits! Exit...", is_exit=True)

        profile_data = None

        with open(self.profile_path, "r", encoding="utf-8") as f:
            profile_data = json.load(f)

        for module_dict in profile_data:
            module_type = module_dict['type'][0]
            set_method = self.param_struct_method_dict.get(module_type, "None")

            if set_method == "None":
                PRINT.warn(f"struct: {module_type} not find in the json's method, "\
                             "remember to update the pqtool json!")
                continue
            # TODO: opt
            get_method = set_method.replace("Set", "Get")

            module_name = module_dict['name']
            self._code_func_start(module_type, module_name, get_method)
            self._travel_profile_module_struct(module_dict, [])
            self._code_func_end(set_method)

    def _code_file_header(self):
        self.code_header = \
            '/*\n'\
            '* NOTICE:\n'\
            '* This file is auto generated by cvi_pqjson2ccode_generator.py.\n'\
            '* Please do not modify this file manually!\n'\
            '*/\n'\
            '#include <stdio.h>\n'\
            '#include <stdlib.h>\n'\
            '#include <string.h>\n'\
            '#include <stdbool.h>\n'\
            '#include "cvi_isp.h"\n'\
            '#include "cvi_ae.h"\n'\
            '#include "cvi_af.h"\n'\
            '#include "cvi_awb.h"\n\n'\
            '#define UNUSED(x) (void)(x)\n\n'

    def _code_api_func(self, sdr_or_wdr):
        self.code_api = f"\nint isp_set_pqjson_{sdr_or_wdr}(int ViPipe)\n{{\n"
        indent = self.indent(1)
        static_func_ls = self.code_static_func.split("\n")
        for static_func in static_func_ls:
            if len(static_func.strip()) == 0:
                continue
            idx_s = static_func.find("set")
            idx_e = static_func.find("(")
            fun_name = static_func[idx_s : idx_e]
            self.code_api += f"{indent}{fun_name}(ViPipe);\n"
        # output sensor information
        #self.code_api += f'{indent}printf("--- pq json information ---\\n");\n'
        #for sensor_name in self.param_info:
        #    self.code_api += f'{indent}printf("%s: %s\\n", "{sensor_name}", "{self.param_info[sensor_name]["json_path"]}");\n'
        self.code_api += f"{indent}return 0;\n}}\n\n"

    def code_run(self, code_from="profile"):
        """
        call this function to run code, from the profile or the paramter header
        """
        self._code_file_header()

        if code_from == "param_header":
            pass
        else:
            self._travel_profile()

        self._code_api_func(self.sdr_or_wdr)

        self.total_code = self.code_header + self.code_static_func +\
                            self.code_api + self.code_body_text

    def code_write(self):
        """
        call this api to write the code
        """
        with open(self.code_path, "w", encoding="utf-8") as f:
            f.write(self.total_code)

    @classmethod
    def code_write_default(cls, file_path, code_str):
        with open(file_path, "w") as f:
            f.write(code_str)

def post_code(c_file_dir):
    """
    deal the special case in post of the code generation
    """
    c_file_ls = os.listdir(c_file_dir)
    c_file_ls = [os.path.join(c_file_dir, i) for i in c_file_ls if i.endswith(".c")]

    for c_file in c_file_ls:
        with open(c_file, "r") as f:
            lines = f.readlines()
            f.seek(0)
            content = f.read()

            function_content = ""
            is_start = False
            target_line = ""
            for line in lines:
                if "setPub_AttrParams" in line and ";" not in line:
                    is_start = True

                if is_start:
                    function_content += line
                    if "f32FrameRate" in line:
                        target_line = line

                if is_start and line.strip() == "}":
                    break

            if function_content:
                new_function_content = \
                        f"static void setPub_AttrParams(VI_PIPE ViPipe) {{\n"\
                        f"\tUNUSED(ViPipe);\n"\
                        f"\tISP_PUB_ATTR_S attr = {{0}};\n\n"\
                        f"\tCVI_ISP_GetPubAttr(ViPipe, &attr);\n"\
                        f"{target_line}"\
                        f"\tCVI_ISP_SetPubAttr(ViPipe, &attr);\n"\
                        f"}}\n"
                new_content = content.replace(function_content, new_function_content)
                with open(c_file, "w") as f:
                    f.write(new_content)

def gen_c_code(args):
    """
    generate the c code
    """
    PRINT.info(f"------ deal with sensor pqjson dir: {args.isp_tuning_dir} ------")
    sensor = os.path.basename(args.isp_tuning_dir)
    params = PARAMS(args.isp_tuning_dir)
    params_info = params.get_params_from_json()
    c_file_sdr_name = f"{sensor}_sdr.c"
    c_file_wdr_name = f"{sensor}_wdr.c"

    if "sdr" in params_info[0] and params_info[0]['sdr']:
        params_info_sdr = [params_info[0]["sdr"], params_info[1]]
        code_sdr = CODE(params_info_sdr, args.profile_path, args.param_header_path,
                        args.gen_c_file_dir, c_file_sdr_name, sensor)
        code_sdr.code_run("profile")
        code_sdr.code_write()
    else:
        code_str = "int isp_set_pqjson_sdr(int ViPipe) {\n"\
                   "\t(void)(ViPipe);\n"\
                   "\treturn 0;\n"\
                   "}"
        CODE.code_write_default(os.path.join(args.gen_c_file_dir, c_file_sdr_name), code_str)

    if "wdr" in params_info[0] and params_info[0]['wdr']:
        params_info_wdr = [params_info[0]["wdr"], params_info[1]]
        code_wdr = CODE(params_info_wdr, args.profile_path, args.param_header_path,
                        args.gen_c_file_dir, c_file_wdr_name, sensor)
        code_wdr.code_run("profile")
        code_wdr.code_write()
    else:
        code_str = "int isp_set_pqjson_wdr(int ViPipe) {\n"\
                   "\t(void)(ViPipe);\n"\
                   "\treturn 0;\n"\
                   "}"
        CODE.code_write_default(os.path.join(args.gen_c_file_dir, c_file_wdr_name), code_str)

    # write the api header
    code_str = "int isp_set_pqjson_wdr(int ViPipe);\n"\
               "int isp_set_pqjson_sdr(int ViPipe);\n"

    CODE.code_write_default(os.path.join(args.gen_c_file_dir, args.header_name), code_str)
    post_code(args.gen_c_file_dir)

if __name__ == "__main__":
    PRINT.is_warn = 0
    PRINT.info("-------------  Start: pqjson -> c code generator --------")
    parser = argparse.ArgumentParser()

    top_dir_env = os.getenv("TOP_DIR")

    if top_dir_env == "":
        PIRNT.error("run the pqjson2ccode generator fail! Should source the env and defconfig first!")
        sys.exit(1)

    # input data structure
    parser.add_argument('--param_header_path', type=str,
                        default='')

    parser.add_argument('--profile_path', type=str,
                        default='python/cvi_bin_profile.json')

    parser.add_argument('--isp_tuning_dir', type=str,
                        default=f'{top_dir_env}/isp_tuning/cv184x/src/cvsens_cv2003')

    # output
    parser.add_argument('--gen_c_file_dir', type=str,
                        default='pqjson2ccode_src')


    parser.add_argument('--header_name', type=str,
                        default='isp_set_pqjson.h')

    args = parser.parse_args()

    if not os.path.exists(args.gen_c_file_dir):
        os.system(f"mkdir -p {args.gen_c_file_dir}")

    t1 = time.time()
    gen_c_code(args)
    t2 = time.time()

    PRINT.info(f"generator run time: {(t2 - t1) * 1000:.2f} ms")
    PRINT.info("-------------  End: pqjson -> c code generator --------")
    marker = os.path.basename(args.isp_tuning_dir.strip("/"))
    PRINT.write_log(marker)
