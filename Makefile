.PHONY: all clean test install
all clean test install:
	$(MAKE) -C src $@
	$(MAKE) -C src/bootsectors $@
	$(MAKE) -C src/lib $@
	$(MAKE) -C src/util $@
	$(MAKE) -C include/pure64 $@

$(V).SILENT:
