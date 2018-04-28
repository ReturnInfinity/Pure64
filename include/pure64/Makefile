.PHONY: all clean test
all clean test:

.PHONY: install
install: core-install lang-install

core-install:
	install core.h $(DESTDIR)$(PREFIX)/include/pure64/core.h
	$(MAKE) -C core install

lang-install:
	install lang.h $(DESTDIR)$(PREFIX)/include/pure64/lang.h
	$(MAKE) -C lang install
