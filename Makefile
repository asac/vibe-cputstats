# Build with the Pantavisor SDK:
#   . /opt/pv-sdk/environment-setup-armv8a-poky-linux-musl && make export
APP    := cpustats
ROOTFS := rootfs

all: $(ROOTFS)/usr/bin/$(APP)

$(ROOTFS)/usr/bin/$(APP): main.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Wall -Wextra -o $@ $< $(LDFLAGS)
	$(STRIP) $@

export: all
	pv-mkpvrexport -b --no-var-volume --config-json config.json $(ROOTFS) $(APP)

clean:
	rm -rf $(ROOTFS) $(APP).pvrexport.tgz

.PHONY: all export clean
