ROOT_DIR = ..
redefault: default.wiq default
default.wiq: $(ROOT_DIR)/test/toxy/default.wid
	$(ROOT_DIR)/quoteinitializer $< \
		'"puts [concat loading built-in widget definitions]\n"' > $@
include $(ROOT_DIR)/Makefile.common
