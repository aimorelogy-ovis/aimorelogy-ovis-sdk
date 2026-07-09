#!/bin/sh
${CVI_SHOPTS}
#
# Define kernel modules sequence
#
modules_single_seq="
/system/ko/cv184x_osal.ko
/system/ko/cv184x_base.ko
/system/ko/cv184x_sys.ko
/system/ko/cv184x_mipi_rx.ko
/system/ko/cv184x_snsr_i2c.ko
/system/ko/cv184x_vi.ko
/system/ko/cv184x_vpss.ko
/system/ko/cv184x_vc_drv.ko
/system/ko/cv184x_rgn.ko
/system/ko/cv184x_ldc.ko
/system/ko/cv184x_vo.ko
/system/ko/cv184x_mipi_tx.ko
/system/ko/cv184x_tde.ko
/system/ko/bmtpu.ko
"

modules_dualos_seq="
/system/ko/cv184x_osal.ko
/system/ko/cv184x_base.ko
/system/ko/cv184x_tde.ko
/system/ko/cvi_ipcm.ko
/system/ko/bmtpu.ko
"
#/system/ko/cv184x_gfbg.ko
#
# Start to insert kernel modules
#
modules_seq=
if [ -n "$modules_single_seq" ]; then
    modules_seq="$modules_single_seq"
elif [ -n "$modules_dualos_seq" ]; then
    modules_seq="$modules_dualos_seq"
fi
if [ -n "$modules_seq" ]; then
    for mod in $modules_seq; do
        insmod "$mod"
    done
fi

echo 3 > /proc/sys/vm/drop_caches
dmesg -n 4

#usb hub control
#/etc/uhubon.sh host

exit $?
