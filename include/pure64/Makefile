DESTDIR ?=

PREFIX ?= /usr/local

install_files += $(DESTDIR)$(PREFIX)/include/pure64/error.h
install_files += $(DESTDIR)$(PREFIX)/include/pure64/dir.h
install_files += $(DESTDIR)$(PREFIX)/include/pure64/fs.h
install_files += $(DESTDIR)$(PREFIX)/include/pure64/file.h
install_files += $(DESTDIR)$(PREFIX)/include/pure64/path.h
install_files += $(DESTDIR)$(PREFIX)/include/pure64/stream.h

.PHONY: all
all:

.PHONY: clean
clean:

.PHONY: install
install: $(DESTDIR)$(PREFIX)/include/pure64 $(install_files)

$(DESTDIR)$(PREFIX)/include/pure64/%.h: %.h
	cp $< $@

$(DESTDIR)$(PREFIX)/include/pure64:
	mkdir -p $@

.PHONY: test
test:
