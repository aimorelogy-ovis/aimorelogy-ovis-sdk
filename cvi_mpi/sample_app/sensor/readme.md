## sample sensor instructions
Based on different sensors, configure the appropriate ini files to the following directory:/mnt/data/sensor_cfg.ini

LINUX:

| sample                 | arg     |example            | purpose                                |
| ---------------------- | --------|-------------------| ---------------------------------------|
| sample_sensor           | 1    |#./sample_sensor<br />#1<br />#0<br />#1   | dump vi raw frame(dump vi dev 0 one raw frame) |
| sample_sensor | 2 |#./sample_sensor<br />#1<br />#0<br />#1    | dump vi yuv frame(dump vi dev 0 one yuv frame) |
| sample_sensor | 3 | #./sample_sensor<br />#0<br />#0<br />#1              | set chn flip/mirror(set chn 0 flip disable,mirror enable) |
| sample_sensor | 4 |#./sample_sensor<br />#4<br />#0    | linear wdr switch(set linear mode)                           |
| sample_sensor | 5 |#./sample_sensor<br />#5    | start AE debug tools                                         |
| sample_sensor | 6 |.#./sample_sensor<br />#6<br />#0xa0002a0<br />#0x100    | dump  register(dump the registers continuously starting from 0xa0002a0 with a size of 0x100.) |
| sample_sensor | 7 |#./sample_sensor<br />#7<br />#3 | print proc mipi-rx\vi |
| sample_sensor | 255 |#./sample_sensor<br />#255 | exit |

DUAL_OS

| sample                 | arg     |example            | purpose                                |
| ---------------------- | --------|-------------------| ---------------------------------------|
| sample_sensor           | 1    |#./sample_sensor<br />#1<br />#0<br />#1   | dump vi raw frame(dump vi dev 0 one raw frame) |
| sample_sensor | 2 |#./sample_sensor<br />#1<br />#0<br />#1    | dump vi yuv frame(dump vi dev 0 one yuv frame) |
| sample_sensor | 3 | #./sample_sensor<br />#0<br />#0<br />#1              | set chn flip/mirror(set chn 0 flip disable,mirror enable) |
| sample_sensor | 4 |#./sample_sensor<br />#4<br />#0    | linear wdr switch(set linear mode)                           |
| sample_sensor | 5 |#./sample_sensor<br />#5    | start AE debug tools                                         |
| sample_sensor | 6 |.#./sample_sensor<br />#6<br />#0xa0002a0<br />#0x100    | dump  register(dump the registers continuously starting from 0xa0002a0 with a size of 0x100.) |
| sample_sensor | 255 |#./sample_sensor<br />#255 | exit |

If you want to obtain proc information in dual_os, type the following in Alios:
1.proc_vi_dbg
2.proc_vi
3.proc_mipi_rx



