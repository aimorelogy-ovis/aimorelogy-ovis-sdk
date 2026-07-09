#!/bin/sh
${CVI_SHOPTS}

if [ ! -f "/tmp/evb_init" ];then
   echo 1 > /tmp/evb_init
else
   exit 1
fi

