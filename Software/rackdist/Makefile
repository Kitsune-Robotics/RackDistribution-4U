ifneq ($(KERNELRELEASE),)
obj-m := rackdist.o
else

KDIR ?= /lib/modules/$(shell uname -r)/build
DKMS_NAME := rackdist
DKMS_VERS := 0.1.0
SRC_DEST := /usr/src/$(DKMS_NAME)-$(DKMS_VERS)

.PHONY: all clean dkms-install dkms-remove

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean

# DKMS only updates the on-disk .ko. The running HID driver stays
# until rmmod; hotplug reuses whatever is already loaded.
dkms-install:
	install -d $(SRC_DEST)
	install -m 644 Makefile dkms.conf rackdist.c rackdist_hid.h $(SRC_DEST)
	dkms add -m $(DKMS_NAME) -v $(DKMS_VERS)
	dkms install -m $(DKMS_NAME) -v $(DKMS_VERS)

dkms-remove:
	-dkms remove -m $(DKMS_NAME) -v $(DKMS_VERS) --all
	rm -rf $(SRC_DEST)

endif
