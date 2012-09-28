CROSS_COMPILE ?= arm-linux-
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip

LIBGCC=$(shell $(CC) --print-file-name libgcc.a)

UCLIBC_DIR=uClibc-0.9.33
TEST_SRC_ROOT=$(UCLIBC_DIR)/test
LIB_DIR := install
CFLAGS += -fno-builtin -nostdinc -nostdlib -fno-stack-protector -nostartfiles
GCCSYSTEM_DIR := $(dir $(LIBGCC))
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


.PHONY: clean tests FORCE
FORCE:

USER_APPS := malloc/malloc string/tst-strlen
USER_BINS := $(addprefix rootfs/, $(notdir $(USER_APPS)))

tests: rootfs $(USER_BINS)
	@echo DONE: $(USER_BINS)


#user applications
define make-user-app1
rootfs/$(notdir $1):
@echo LINK $$@
$(LD) -static -T $(LIB_DIR)/user1.ld -o $$@ -L$(LIB_DIR)/usr/lib -L$(LIB_DIR)/lib $(LIB_DIR)/usr/lib/crt1.o $(LIB_DIR)/usr/lib/crti.o  $(addprefix $(TEST_SRC_ROOT)/, $(addsuffix .o,$1)) -lpthread -m -lrt -lc $(LIBGCC) $(LIB_DIR)/usr/lib/crtn.o
endef

define make-user-app
rootfs/$(notdir $1): rootfs $(addprefix $(TEST_SRC_ROOT)/, $(addsuffix .o,$1))
	@echo LINK $$@
	$(LD) -static -T user1.ld -o $$@ -L$(LIB_DIR)/usr/lib -L$(LIB_DIR)/lib $(LIB_DIR)/usr/lib/crt1.o $(LIB_DIR)/usr/lib/crti.o  $(addprefix $(TEST_SRC_ROOT)/, $(addsuffix .o,$1)) -lpthread -lm -lrt -lc $(LIBGCC) $(LIB_DIR)/usr/lib/crtn.o
	$(STRIP) $$@

endef

$(foreach bdir, $(USER_APPS),$(eval $(call make-user-app,$(bdir))))

clean:
	rm -rf install rootfs
	find $(TEST_SRC_ROOT) -name *.o -exec rm -f {} \;
	

