#! /bin/bash
#
test_code_dir="$1"
if [ -z $test_code_dir ]; then
	test_code_dir=code_test
fi

# generate cvi_bin_param_header.i
top_dir=${TOP_DIR}
if [ -z $top_dir ]; then
	echo "You should source the env and build middleware first! Exit..."
	exit -1
fi

cur_dir=$(cd "$(dirname "$0")" && pwd)
chip_arch=$(echo $CHIP_ARCH | tr 'A-Z' 'a-z')

preprocess_file="${top_dir}/cvi_mpi/modules/cvi_bin/include/cvi_bin_param_header.h"
output_dep_file="$cur_dir/../tmp/cvi_bin_param_header.i"

if [ ! -f $preprocess_file ]; then
	echo "gcc preprocess file: ${preprocess_file} not exists! Exit..."
	exit -1
fi

if [ ! -d $cur_dir/../tmp ]; then
	mkdir -p $cur_dir/../tmp
fi

incs="-I${top_dir}/cvi_mpi/include -I${top_dir}/cvi_mpi/include/isp"
c_flags="-E -MMD ${incs}"

${CROSS_COMPILE}gcc ${c_flags} -c ${preprocess_file} -o $output_dep_file

if [ ! -f $output_dep_file ]; then
	echo "generate dep file: $output_dep_file fail! Exit..."
	exit -1
else
	echo "generate dep file: $output_dep_file successfully!"
fi

# generate cvi_bin_profile.json
profile_gen_script_path="$top_dir/cvi_mpi/modules/cvi_bin/python/gencode.py"
profile_save_path="$cur_dir/../tmp/cvi_bin_profile.json"

if [ ! -f $profile_gen_script_path ]; then
	echo "profile file gen script: $profile_gen_script_path no exits! Exit..."
	exit -1
fi

rm -f $profile_save_path

python3 "$profile_gen_script_path" "$output_dep_file" --profile "$profile_save_path" --gen_profile_only > /dev/null

if [ ! -f $profile_save_path ]; then
	echo "generate profile: $profile_save_path fail! Exit..."
	exit -1
else
	echo "generate profile: $profile_save_path successfully!"
fi

# convert pqjson to c code
genccode_script_path="$cur_dir/pqjson2ccode_generator.py"
pq_tuning_dir="$cur_dir/../$chip_arch/src"

for sensor_tuning_dir in "$pq_tuning_dir"/*; do
	if [ -d $sensor_tuning_dir ]; then
		python3 $genccode_script_path --isp_tuning_dir "$sensor_tuning_dir" --profile_path "$profile_save_path" \
			--gen_c_file_dir $sensor_tuning_dir
	fi
done

# generate the test code
test_script_path="$cur_dir/testcode_generator.py"
test_code_dir_path="$cur_dir/../$test_code_dir"
rm -rf $test_code_dir_path
mkdir -p $test_code_dir_path

test_code_dir_path=$(cd $test_code_dir_path && pwd)

python3 $test_script_path --isp_tuning_dir "$pq_tuning_dir" --test_code_save_dir "$test_code_dir_path"
