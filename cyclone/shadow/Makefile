ROOT_DIR = ../..
redefault: default $(ROOT_DIR)/bin/cyclist
include $(ROOT_DIR)/Makefile.common

$(ROOT_DIR)/bin/cyclist: $(SHARED_DIR)/common/binport.c
	$(CC) -DBINPORT_STANDALONE -o $@ $<

