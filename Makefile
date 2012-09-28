CROSS_COMPILE ?= arm-linux-
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip

LIBGCC=$(shell $(CC) --print-file-name libgcc.a)

UCLIBC_DIR=uClibc-0.9.33
TEST_SRC_ROOT=$(UCLIBC_DIR)/test
LIB_DIR := install
CFLAGS += -fno-builtin -nostdinc -nostdlib -fno-stack-protector -nostartfiles
GCCSYSTEM_DIR := $(dir LIB_DIR)
CFLAGS += -isystem $(GCCSYSTEM_DIR)/include
CFLAGS += -I$(GCCSYSTEM_DIR)/include-fixed
CFLAGS += -I$(LIB_DIR)/usr/include
CFLAGS += -march=armv5
CFLAGS += -Ilinux_header_goldfish/include 


all: install tests

install:
	make -C $(UCLIBC_DIR) -j2
	make -C $(UCLIBC_DIR) install

rootfs:
	mkdir rootfs

testobj:
	mkdir testobj

.PHONY: clean tests FORCE
FORCE:

tests: rootfs testobj
	echo $(LIBGCC)

#user applications
define make-user-app
$1: $(BUILD_DIR) $(addsuffix .o,$1) $(USER_LIB)
@echo LINK $$@
$(LD) $(FPGA_LD_FLAGS) -T $(USER_LIB_SRCDIR)/user.ld $(addsuffix .o,$1) $(USER_LIB) -o $$@
$(SED) 's/$$$$FILE/$(notdir $1)/g' tools/piggy.S.in > $(USER_OBJDIR)/piggy.S
$(AS) $(USER_OBJDIR)/piggy.S -o $$@.piggy.o
endef

clean:
	rm -rf install rootfs
	

