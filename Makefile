CC ?= gcc
CPPFLAGS ?= -D_FORTIFY_SOURCE=2
CFLAGS ?= -std=gnu11 -Wall -Wextra -O2

-include Env.mak

override LDFLAGS += -ljson-c -luuid

ifdef DEBUG
CFLAGS += -g
LDFLAGS += -Wl,--strip-unneeded
else
CPPFLAGS += -DNDEBUG
LDFLAGS += -Wl,--strip-all
endif

__d = $(if $(value $1),-D$1='"$($1)"',$(info [WARN] $1 is undefined))

all: rpc run

rpc: CPPFLAGS += $(call __d,CLIENT_ID)
rpc: CPPFLAGS += $(call __d,DETAILS)
rpc: CPPFLAGS += $(call __d,STATE)
rpc: CPPFLAGS += $(call __d,LARGE_IMAGE)
rpc: CPPFLAGS += $(call __d,LARGE_TEXT)
rpc: CPPFLAGS += $(call __d,SMALL_IMAGE)
rpc: CPPFLAGS += $(call __d,SMALL_TEXT)
rpc: rpc.c; @$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) -o $@

.PHONY: run
run: rpc; @./$^
