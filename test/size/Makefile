$(info ************ make dev ************)

SPDK_ROOT_DIR := $(abspath $(CURDIR)/../../spdk)

APP = test_size

NVME_DIR := $(SPDK_ROOT_DIR)/lib/nvme

include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

CFLAGS += -I$(CURDIR)/../../include
$(info ************ $(CFLAGS) ************)

C_SRCS := $(APP:%=%.c)
$(info ************ $(C_SRCS) ************)

SPDK_LIB_LIST = $(SOCK_MODULES_LIST) nvme vmd

include $(SPDK_ROOT_DIR)/mk/spdk.app_vars.mk


APP_NAME := $(notdir $(APP))
APP := $(APP_NAME:%=$(CURDIR)/../../build/test/%)$(EXEEXT)

LIBS += $(SPDK_LIB_LINKER_ARGS)

CLEAN_FILES = $(APP)

all : $(APP)
	@:

install: empty_rule

uninstall: empty_rule

# To avoid overwriting warning
empty_rule:
	@:

$(APP) : $(OBJS) $(SPDK_LIB_FILES) $(ENV_LIBS)
	$(LINK_C)

clean :
	$(CLEAN_C) $(CLEAN_FILES)

include $(SPDK_ROOT_DIR)/mk/spdk.deps.mk
