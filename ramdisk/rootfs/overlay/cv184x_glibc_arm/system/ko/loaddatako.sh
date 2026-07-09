#!/bin/sh
${CVI_SHOPTS}
#
# Start to insert kernel modules
#
insmod /mnt/data/ko/cfg80211.ko
insmod /mnt/data/ko/libphy.ko
insmod /mnt/data/ko/fixed_phy.ko
insmod /mnt/data/ko/usb-common.ko
insmod /mnt/data/ko/usbcore.ko
insmod /mnt/data/ko/of_mdio.ko
insmod /mnt/data/ko/phylink.ko
insmod /mnt/data/ko/mdio_devres.ko
insmod /mnt/data/ko/pcs-xpcs.ko
insmod /mnt/data/ko/stmmac.ko
insmod /mnt/data/ko/stmmac-platform.ko
insmod /mnt/data/ko/dwmac-cvitek.ko
insmod /mnt/data/ko/dwmac-thead.ko
insmod /mnt/data/ko/cvitek.ko
insmod /mnt/data/ko/dwc2.ko
insmod /mnt/data/ko/libsha256.ko
insmod /mnt/data/ko/mii.ko
insmod /mnt/data/ko/mmc_core.ko
insmod /mnt/data/ko/mmc_block.ko
insmod /mnt/data/ko/pwrseq_emmc.ko
insmod /mnt/data/ko/pwrseq_simple.ko
insmod /mnt/data/ko/sdhci.ko
insmod /mnt/data/ko/sdhci-pltfm.ko
insmod /mnt/data/ko/sdhci-cvi.ko
insmod /mnt/data/ko/sha256_generic.ko

exit $?
