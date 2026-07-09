
SUBDIR = src

.PHONY: all clean package $(SUBDIR)

all: $(SUBDIR)

$(SUBDIR):
	$(MAKE) -C $@ $(filter-out package,$(MAKECMDGOALS))

clean: $(SUBDIR)
	rm -rf install
	rm -f cvi_rtsp.tar.gz

package: $(SUBDIR)
	@mkdir -p install/include
	@mkdir -p install/lib
	@cp -r include/cvi_rtsp install/include
	@cp -r src/libcvi_rtsp.so install/lib
	@cp -r src/libcvi_rtsp.a install/lib
	@tar zcf cvi_rtsp.tar.gz install
