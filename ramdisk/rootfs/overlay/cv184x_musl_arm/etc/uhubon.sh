GPIO_HUBPORT_EN=449
GPIO_ROLESEL=450
GPIO_HUBRST=451
SYS_GPIO=/sys/class/gpio

hub_on() {
  echo "turn on usb hub"
  if [ ! -d $SYS_GPIO/gpio$GPIO_HUBPORT_EN ]; then
      echo $GPIO_HUBPORT_EN >/sys/class/gpio/export
  fi

  if [ ! -d $SYS_GPIO/gpio$GPIO_ROLESEL ]; then
      echo $GPIO_ROLESEL >/sys/class/gpio/export
  fi

  if [ ! -d $SYS_GPIO/gpio$GPIO_HUBRST ]; then
      echo $GPIO_HUBRST >/sys/class/gpio/export
  fi

  echo "out" >/sys/class/gpio/gpio$GPIO_HUBPORT_EN/direction
  echo "out" >/sys/class/gpio/gpio$GPIO_ROLESEL/direction
  echo "out" >/sys/class/gpio/gpio$GPIO_HUBRST/direction

  echo 1 >/sys/class/gpio/gpio$GPIO_HUBPORT_EN/value
  echo 0 >/sys/class/gpio/gpio$GPIO_ROLESEL/value
  echo 0 >/sys/class/gpio/gpio$GPIO_HUBRST/value
}

hub_off() {
  echo "turn off usb hub"
  if [ ! -d $SYS_GPIO/gpio$GPIO_HUBPORT_EN ]; then
      echo $GPIO_HUBPORT_EN >/sys/class/gpio/export
  fi

  if [ ! -d $SYS_GPIO/gpio$GPIO_ROLESEL ]; then
      echo $GPIO_ROLESEL >/sys/class/gpio/export
  fi

  if [ ! -d $SYS_GPIO/gpio$GPIO_HUBRST ]; then
      echo $GPIO_HUBRST >/sys/class/gpio/export
  fi

  echo "out" >/sys/class/gpio/gpio$GPIO_HUBPORT_EN/direction
  echo "out" >/sys/class/gpio/gpio$GPIO_ROLESEL/direction
  echo "out" >/sys/class/gpio/gpio$GPIO_HUBRST/direction

  echo 0 >/sys/class/gpio/gpio$GPIO_HUBPORT_EN/value
  echo 1 >/sys/class/gpio/gpio$GPIO_ROLESEL/value
  echo 1 >/sys/class/gpio/gpio$GPIO_HUBRST/value
}

inst_mod() {
  modules="configfs libcomposite u_serial usb_f_acm cvi_usb_f_cvg
    usb_f_uvc usb_f_fs u_audio usb_f_uac1 usb_f_serial
    usb_f_mass_storage u_ether usb_f_ecm usb_f_eem usb_f_rndis"
  if [ -r /etc/ovis-boot.conf ]; then
    . /etc/ovis-boot.conf
    if [ "$OVIS_USB_DEVICE_ONLY" = 1 ]; then
      modules="configfs libcomposite u_ether usb_f_ncm usb_f_uvc usb_f_fs"
    fi
  fi
  for module in $modules
  do
    module_path=/mnt/system/ko/$module.ko
    if [ -f "$module_path" ]; then
      insmod "$module_path"
    fi
  done
}

case "$1" in
  host)
	insmod /mnt/system/ko/dwc2.ko
	hub_on
	;;
  device)
	hub_off
	inst_mod
	if [ "$(cat /proc/cviusb/otg_role 2>/dev/null)" != "device" ]; then
		echo device > /proc/cviusb/otg_role
	fi
	;;
  *)
	echo "Usage: $0 host"
	echo "Usage: $0 device"
	exit 1
esac
exit $?
