import os
import shutil

# toolchains options
ARCH        ='risc-v'
# VENDOR      ='t-head'
CPU         ='rv64'
CROSS_TOOL  ='gcc'

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = r'../../..'

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = os.path.abspath(RTT_ROOT + r'/host-tools/gcc/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1/bin')
else:
    print('Please make sure your toolchains is GNU GCC!')
    exit(0)

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

GENERATED_DIR = 'generated/'
IMAGE_OUTPUT_PATH = GENERATED_DIR + "image/"

if os.path.exists(IMAGE_OUTPUT_PATH):
    shutil.rmtree(IMAGE_OUTPUT_PATH)
os.makedirs(IMAGE_OUTPUT_PATH)

# BUILD = 'debug'
BUILD = ''

CHIP_TYPE = 'cv180xb_qfn'
with open('rtconfig.h', 'r', encoding='utf-8') as file:
    content = file.read()
    if '#define SOC_TYPE_CV180XB_QFN' in content:
        CHIP_TYPE = 'cv180xb_qfn'
    elif '#define SOC_TYPE_CV181XC_QFN' in content:
        CHIP_TYPE = 'cv181xc_qfn'

FIP_PATH = '../cvi_boards/tools/fip/' + CHIP_TYPE +'/'
BOOT_PATH = '../cvi_boards/' + CHIP_TYPE + '/bootimgs/'
PARTITION_TABLE_PATH = '../cvi_boards/' + CHIP_TYPE + '/partitions/'
IMAGE_TOOL_PATH = '../../../tools/image_tools/'

if PLATFORM == 'gcc':
    # toolchains
    # PREFIX  = 'riscv64-unknown-elf-'
    PREFIX  = os.getenv('RTT_CC_PREFIX') or 'riscv64-unknown-elf-'
    CC      = PREFIX + 'gcc'
    CXX     = PREFIX + 'g++'
    AS      = PREFIX + 'gcc'
    AR      = PREFIX + 'ar'
    LINK    = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE    = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY  = PREFIX + 'objcopy'

    DEVICE  = ' -mcmodel=medany -march=rv64imafdc_xtheadc -mabi=lp64d'
    CFLAGS  = DEVICE + ' -Wall -Werror -Wno-error=int-to-pointer-cast -Wno-error=unused-function -Wno-cpp -fno-var-tracking-assignments -ffreestanding -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -fno-asynchronous-unwind-tables -fno-builtin-fprintf -D_POSIX_SOURCE '
    CFLAGS += '-D_SYS__PTHREADTYPES_H_ -Wno-error=unused-but-set-variable'
    AFLAGS  = ' -c' + DEVICE + ' -x assembler-with-cpp -DENTRY_POINT=entry' 
    LFLAGS  = DEVICE + ' -fno-asynchronous-unwind-tables -nostartfiles -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,_start -T link.lds' + ' -lsupc++ -lgcc -static'
    CPATH   = ''
    LPATH   = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -ggdb'
        AFLAGS += ' -ggdb'
    else:
        CFLAGS += ' -O3 -Os'

    CXXFLAGS = CFLAGS

DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > rtthread.asm\n'
POST_ACTION = OBJCPY + ' -O binary $TARGET ' + IMAGE_OUTPUT_PATH + 'rtt.bin' +' \n' + SIZE + ' $TARGET \n'
POST_ACTION += '@ python ' + IMAGE_TOOL_PATH + 'generate_partition.py ' + IMAGE_OUTPUT_PATH +' ' + IMAGE_OUTPUT_PATH + ' ' + PARTITION_TABLE_PATH + 'partition_spinor.xml\n'
POST_ACTION += '@ mv imtb ' + IMAGE_OUTPUT_PATH + '\n'
POST_ACTION += '@ cp ' + PARTITION_TABLE_PATH + 'partition_spinor.xml ' + IMAGE_OUTPUT_PATH + ' \n'
POST_ACTION += '@ cp ' + BOOT_PATH + 'boot0 ' + IMAGE_OUTPUT_PATH + ' \n'
POST_ACTION += '@ cp ' + BOOT_PATH + 'boot ' + IMAGE_OUTPUT_PATH + ' \n'
POST_ACTION += '@ cp ' + FIP_PATH +'fip.bin ' + IMAGE_OUTPUT_PATH + ' \n'
POST_ACTION += '@ zip -qrj image.zip ' + IMAGE_OUTPUT_PATH + ' \n'
POST_ACTION += '@ mv image.zip ' + GENERATED_DIR + '\n'


if BUILD == 'debug':
    POST_ACTION += DUMP_ACTION

