# Open-Source Release Notes

- This release package has removed proprietary directories:
  - cvitek/task/audio/src/SSP_Algorithm_20211202
  - cvitek/task/audio/src/unit_test
- Build time links audio algorithm through prebuilt static library libssp.a:
  - Default path: components/cvi_comps/cvi_mpi/cvi_mw_audio/lib/libssp.a
  - Or configure -DSSP_LIB=/abs/path/to/libssp.a
- Example build:
  - cd bsp/cvitek/c906_little/
  - scons -c && scons

