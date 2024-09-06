# # Copyright (C) 2015, Wazuh Inc.
# # TODO: mysql and postgresql?
# #
# # Copyright (C) 2015, Wazuh Inc.
# #
# # This program is free software; you can redistribute it and/or modify
# # it under the terms of the GNU General Public License as published by
# # the Free Software Foundation; either version 2 of the License, or
# # (at your option) any later version.
# #

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_P := $(shell sh -c 'uname -p 2>/dev/null || echo not')
uname_R := $(shell sh -c 'uname -r 2>/dev/null || echo not')
uname_V := $(shell sh -c 'uname -v 2>/dev/null || echo not')
uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')

ifeq (${uname_S},Linux)
	PRECOMPILED_OS:=linux
else
ifeq (${uname_S},AIX)
	PRECOMPILED_OS:=aix
else
ifeq (${uname_S},SunOS)
	PRECOMPILED_OS:=solaris
else
ifeq (${uname_S},Darwin)
	PRECOMPILED_OS:=darwin
else
ifeq (${uname_S},FreeBSD)
	PRECOMPILED_OS:=freebsd
else
ifeq (${uname_S},NetBSD)
	PRECOMPILED_OS:=netbsd
else
ifeq (${uname_S},OpenBSD)
	PRECOMPILED_OS:=openbsd
else
ifeq (${uname_S},HP-UX)
	PRECOMPILED_OS:=hpux
else
	    # Unknow platform
endif # HPUX
endif # OpenBSD
endif # NetBSD
endif # FreeBSD
endif # Darwin
endif # SunOS
endif # AIX
endif # Linux

# ################################
# #### External dependencies  ####
# ################################

TAR := tar -xf
GUNZIP := gunzip
GZIP := gzip
CURL := curl -so
DEPS_VERSION = 30
RESOURCES_URL_BASE := https://packages.wazuh.com/deps/
RESOURCES_URL := $(RESOURCES_URL_BASE)$(DEPS_VERSION)

ifneq (,$(filter ${uname_S},Linux Darwin HP-UX))
	cpu_arch := ${uname_M}
ifneq (,$(filter ${cpu_arch},x86_64 amd64))
	PRECOMPILED_ARCH := /amd64
else
ifneq (,$(filter ${cpu_arch},i386 i686))
	PRECOMPILED_ARCH := /i386
else
ifneq (,$(filter ${cpu_arch},aarch64 arm64))
	PRECOMPILED_ARCH := /aarch64
else
ifneq (,$(filter ${cpu_arch},armv8l armv7l arm32 armhf))
	PRECOMPILED_ARCH := /arm32
else
ifeq (${cpu_arch},ppc64le)
	PRECOMPILED_ARCH := /ppc64le
else
ifneq (,$(filter ${cpu_arch},ia64))
	PRECOMPILED_ARCH := /ia64
else
	PRECOMPILED_ARCH := /${uname_P}
endif
endif
endif
endif
endif
endif
else
ifneq (,$(filter ${uname_S},SunOS AIX))
	cpu_arch := ${uname_P}
ifeq (${cpu_arch},powerpc)
	PRECOMPILED_ARCH := /powerpc
else
ifneq (,$(filter ${cpu_arch},sparc sun4u))
	PRECOMPILED_ARCH := /sparc
else
ifneq (,$(filter ${cpu_arch},i386 i86pc))
	PRECOMPILED_ARCH := /i386
else
	PRECOMPILED_ARCH := /${uname_M}
endif
endif
endif
else
    cpu_arch := ${uname_M}
ifneq (,$(filter ${cpu_arch},unknown Unknown not))
	cpu_arch := ${uname_P}
endif
	PRECOMPILED_ARCH := /${cpu_arch}
endif
endif

PRECOMPILED_RES := $(PRECOMPILED_OS)$(PRECOMPILED_ARCH)

# Agent dependencies
EXTERNAL_RES := libdb procps pacman rpm

EXTERNAL_DIR := $(EXTERNAL_RES:%=external/%)
EXTERNAL_TAR := $(EXTERNAL_RES:%=external/%.tar.gz)

.PHONY: all deps

all: deps

external_dir:
	@mkdir -p external

deps: external_dir $(EXTERNAL_TAR)

# Remaining dependencies
ifneq (,$(PRECOMPILED_RES))
external/%.tar.gz: external-precompiled/%.tar.gz
	test -d $(patsubst %.tar.gz,%,$@) ||\
	($(CURL) $@ $(RESOURCES_URL)/libraries/sources/$(patsubst external/%,%,$@) &&\
	cd external && $(GUNZIP) $(patsubst external/%,%,$@) && $(TAR) $(patsubst external/%.gz,%,$@) && rm $(patsubst external/%.gz,%,$@))
else
external/%.tar.gz:
	$(CURL) $@ $(RESOURCES_URL)/libraries/sources/$(patsubst external/%,%,$@)
	cd external && $(GUNZIP) $(patsubst external/%,%,$@)
	cd external && $(TAR) $(patsubst external/%.gz,%,$@)
	rm $(patsubst %.gz,%,$@)
endif

ifneq (,$(PRECOMPILED_RES))
external-precompiled/%.tar.gz:
	-$(CURL) external/$(patsubst external-precompiled/%,%,$@) $(RESOURCES_URL)/libraries/$(PRECOMPILED_RES)/$(patsubst external-precompiled/%,%,$@) || true
	-cd external && test -e $(patsubst external-precompiled/%,%,$@) && $(GUNZIP) $(patsubst external-precompiled/%,%,$@) || true
	-cd external && test -e $(patsubst external-precompiled/%.gz,%,$@) && $(TAR) $(patsubst external-precompiled/%.gz,%,$@) || true
	-test -e external/$(patsubst external-precompiled/%.gz,%,$@) && rm external/$(patsubst external-precompiled/%.gz,%,$@) || true
endif

clean:
	rm -rf external/
