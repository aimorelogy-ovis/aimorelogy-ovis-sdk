#!/bin/sh

make clean
cd isp_test;make -j || exit "$?";cd ..
make -j || exit "$?"
make package || exit "$?"
