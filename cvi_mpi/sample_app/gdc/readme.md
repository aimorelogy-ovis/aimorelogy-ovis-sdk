# sample_gdc instructions


Sample instructions:

| Sample                                   | Usage           | Purpose                                                                                        |Res                                                     |
|------------------------------------------|-----------------|------------------------------------------------------------------------------------------------|--------------------------------------------------------|
|  gdc basic test rot                      | ./sample_gdc 0  | Verify the rotation functionality of the GDC module.                                           | 1920x1080.yuv                                          |
|  gdc basic test ldc                      | ./sample_gdc 1  | Verify the LDC functionality of the GDC module                                                 | 1920x1080_barrel_0.3.yuv、1920x1080_pincushion_0.3.yuv |
|  gdc basic test ldc load mesh            | ./sample_gdc 2  | Verify the lens distortion correction function of the GDC module based on mesh data.           | 1920x1080_barrel_0.3_r0_ofst_0_0_d-200.mesh            |
|  gdc basic test cmdq                     | ./sample_gdc 3  | Verify the GDC module's command queue (CMDQ) functionality                                     | 1920x1080.yuv                                          |
|  gdc basic test cmdq_1to2                | ./sample_gdc 4  | Verify the capability of the GDC module's CMDQ to handle a "one input to two outputs" scenario.| 1920x1080.yuv                                          |
|  gdc basic test async                    | ./sample_gdc 5  | Verify the asynchronous processing capability of the GDC module                                | 1920x1080.yuv                                          |
|  gdc basic test grid_info                | ./sample_gdc 6  | Verify the LDC grid information loading and processing capability of the GDC module.           | 1280x768.yuv、grid_info_79_44_3476_80_45_1280x720.dat  |
|  gdc basic test running suspend          | ./sample_gdc 7  | Verify the behavior of the GDC module when asynchronous processing is interrupted              | 1920x1080.yuv                                          |
|                                          |                 |                                                                                                |                                                        |


