ROOT_DIR = ..
redefault: pddpboot default

pddpboot: $(ROOT_DIR)/bin/pddpboot.tcl \
	$(ROOT_DIR)/bin/pddpclient.tcl $(ROOT_DIR)/bin/pddpserver.tcl

$(ROOT_DIR)/bin/%.tcl: %.tcl
	cp $< $@

include $(ROOT_DIR)/Makefile.common
