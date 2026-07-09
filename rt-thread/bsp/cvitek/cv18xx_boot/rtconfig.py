import os
import shutil

# toolchains options
ARCH        ='risc-v'
# VENDOR      ='t-head'
CPU         ='rv64'
CROSS_TOOL  ='gcc'

IMAGE_OUTPUT_PATH = "./image"

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = r'../../..'

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = os.path.abspath(RTT_ROOT + r'/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1/bin')
else:
    print('Please make sure your toolchains is GNU GCC!')
    exit(0)

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

if os.path.exists(IMAGE_OUTPUT_PATH):
    shutil.rmtree(IMAGE_OUTPUT_PATH)
os.mkdir(IMAGE_OUTPUT_PATH)

# BUILD = 'debug'
BUILD = ''

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
    CFLAGS += '-D_SYS__PTHREADTYPES_H_'
    AFLAGS  = ' -c' + DEVICE + ' -x assembler-with-cpp -DENTRY_POINT=main' 
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
POST_ACTION = OBJCPY + ' -O binary $TARGET image/boot \n' + SIZE + ' $TARGET \n'

if BUILD == 'debug':
    POST_ACTION += DUMP_ACTION

