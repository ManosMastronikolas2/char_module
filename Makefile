# Name of the module (without the .ko extension)
obj-m := charmod.o

# Default target: build the module
all:
	@echo "Building kernel module..."
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

# Install (insert) the module
install: all
	@echo "Inserting module..."
	sudo insmod ./charmod.ko || true

# Uninstall (remove) the module
uninstall:
	@echo "Removing module..."
	sudo rmmod charmod || true

.PHONY: all clean install uninstall
