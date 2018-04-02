%.sys: %
	@echo "OBJCOPY $@"
	$(OBJCOPY) -O binary $< $@

%: %.o
	@echo "LD      $@"
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.a:
	@echo "AR      $@"
	$(AR) $(ARFLAGS) $@ $^

%.o: %.S
	@echo "AS      $@"
	$(AS) $(ASFLAGS) -c $< -o $@

%.o: %.c
	@echo "CC      $@"
	$(CC) $(CFLAGS) -c $< -o $@

%.sys: %.asm
	@echo "NASM    $@"
	$(NASM) $(NASMFLAGS) $< -o $@
