include Makefile.dirs

all:
	@for i in $(MIXED_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) ; fi ) ; done
	cd doc/src; $(MAKE)

clean cleanall:
	@for i in $(MIXED_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) $@ ; fi ) ; done
	rm -f *.gz

diff depend emptydeps:
	@for i in $(MIXED_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) $@ ; fi ) ; done

snap release:
	@for i in $(RELEASE_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) $@ ; fi ) ; done

backup:
	tar -X miXed-bak-exclude.files -zcf miXed-bak.tar.gz *
