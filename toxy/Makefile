ROOT_DIR = ..
WIQFILE = setup.wiq
redefault: checkwiq $(WIQFILE) default
checkwiq:
	@if [ -f $(WIQFILE) ] && ! [ -s $(WIQFILE) ] ; then \
		echo ERROR: empty file $(WIQFILE)... removed ; \
		rm -f $(WIQFILE) ; fi
$(WIQFILE): $(ROOT_DIR)/test/toxy/$(WIQFILE:.wiq=.wid)
	$(ROOT_DIR)/quoteinitializer $< \
	    '"puts stderr [concat loading built-in widget definitions]\n"' > $@
	@if [ -f $(WIQFILE) ] && ! [ -s $(WIQFILE) ] ; then \
		echo ERROR: quoteinitializer failed... $(WIQFILE) removed ; \
		rm -f $(WIQFILE) ; fi
include $(ROOT_DIR)/Makefile.common
