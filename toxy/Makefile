ROOT_DIR = ..
redefault: setup.wiq default
setup.wiq: $(ROOT_DIR)/test/toxy/setup.wid
	$(ROOT_DIR)/quoteinitializer $< \
	    '"puts stderr [concat loading built-in widget definitions]\n"' > $@
include $(ROOT_DIR)/Makefile.common
