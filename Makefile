CC ?= gcc
CFLAGS ?= -std=gnu11 -Wall -Wextra
LDFLAGS ?= -Wl,--build-id=uuid

-include Env.mak

LDFLAGS += -ljson-c -luuid

ifdef DEBUG
CFLAGS += -g -Og
else
CFLAGS += -flto -O2
CPPFLAGS += -DNDEBUG -D_FORTIFY_SOURCE=2
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
rpc: rpc.o

.PHONY: run
run: rpc; @./$<

.PHONY: clean
clean: ; $(RM) rpc rpc.o

.PHONY: lint
lint: $(wildcard *.[ch]); clang-tidy $^

.PHONY: format
format: $(wildcard *.[ch]); clang-format -i $^
