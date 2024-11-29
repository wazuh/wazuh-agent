#!/bin/bash
# Program to build OSX wazuh-agent
# Wazuh package generator
# Copyright (C) 2015, Wazuh Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.
set -exf
DESTINATION_PATH=$1
WAZUH_PATH=$2
BUILD_JOBS=$3
MAKE_COMPILATION=$4
VCPKG_KEY=$5
SOURCES_DIR=${WAZUH_PATH}/src

set_vcpkg_remote_binary_cache(){
  local vcpkg_token="$1"

  if [[ $(mono --version 2>/dev/null) =~ [0-9] ]]; then
    echo "mono already installed, proceeding"
    export VCPKG_BINARY_SOURCES="clear;nuget,GitHub,readwrite"
    $WAZUH_PATH/src/vcpkg/bootstrap-vcpkg.sh
    mono `$WAZUH_PATH/src/vcpkg/vcpkg fetch nuget | tail -n 1` \
        sources add \
        -source "https://nuget.pkg.github.com/wazuh/index.json" \
        -name "GitHub" \
        -username "wazuh" \
        -password "$vcpkg_token"
    mono `$WAZUH_PATH/src/vcpkg/vcpkg fetch nuget | tail -n 1` \
        setapikey "$vcpkg_token" \
        -source "https://nuget.pkg.github.com/wazuh/index.json"  
  else
    echo "mono in not installed, remote binary caching not being enabled"
  fi
}

function build() {

    if [ "${MAKE_COMPILATION}" == "yes" ]; then
        if [ ! -z "${VCPKG_KEY}" ]; then
            set_vcpkg_remote_binary_cache $VCPKG_KEY
        fi
      	git submodule update --init --recursive
        cmake -S $SOURCES_DIR -B $SOURCES_DIR/build -DINSTALL_ROOT=$DESTINATION_PATH
        make -C $SOURCES_DIR/build -j $BUILD_JOBS
    fi

    EXECUTABLE_FILES=$(find "${SOURCES_DIR}" -maxdepth 1 -type f ! -name "*.py" -exec file {} + | grep 'executable' | cut -d: -f1)
    EXECUTABLE_FILES+=" $(find "${SOURCES_DIR}" -type f ! -name "*.py" ! -path "${SOURCES_DIR}/external/*" ! -path "${SOURCES_DIR}/symbols/*" -name "*.dylib" -print 2>/dev/null)"

    for var in $EXECUTABLE_FILES; do
        filename=$(basename "$var")
        dsymutil -o "${SOURCES_DIR}/symbols/${filename}.dSYM" "$var" 2>/dev/null && strip -S "$var"
    done

    echo "Installing sources"
    make -C $SOURCES_DIR/build install -j $BUILD_JOBS
}

build
