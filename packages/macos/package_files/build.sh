#!/bin/bash
# Program to build OSX wazuh-agent
# Wazuh package generator
# Copyright (C) 2015, Wazuh Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.
set -e
VERBOSE=$1
DESTINATION_PATH=$2
WAZUH_PATH=$3
BUILD_JOBS=$4
VCPKG_KEY=$5

set_vcpkg_remote_binary_cache(){
  local vcpkg_token="$1"

  if [[ $(mono --version 2>/dev/null) =~ [0-9] ]]; then
    echo "mono already installed, proceeding"

    git clone --branch master --single-branch https://github.com/microsoft/vcpkg.git
    pushd vcpkg

    export VCPKG_ROOT="$(pwd)"

    git checkout "2024.09.30"

    export VCPKG_BINARY_SOURCES="clear;nuget,GitHub,readwrite"
    ./bootstrap-vcpkg.sh
    mono `./vcpkg fetch nuget | tail -n 1` \
        sources add \
        -source "https://nuget.pkg.github.com/wazuh/index.json" \
        -name "GitHub" \
        -username "wazuh" \
        -password "$vcpkg_token"
    mono `./vcpkg fetch nuget | tail -n 1` \
        setapikey "$vcpkg_token" \
        -source "https://nuget.pkg.github.com/wazuh/index.json"

    popd
  else
    echo "mono is not installed, remote binary caching not being enabled"
  fi
}

function build() {

    if [ "${VERBOSE}" = "yes" ]; then
        set -ex
    fi

    if [ ! -d "${WAZUH_PATH}/install" ]; then

        if [ ! -d "$WAZUH_PATH/build" ]; then
            if [ ! -z "${VCPKG_KEY}" ]; then
                set_vcpkg_remote_binary_cache $VCPKG_KEY
            fi

            cmake -S $WAZUH_PATH/src -B $WAZUH_PATH/build -DVCPKG_INSTALL_OPTIONS="--debug"
            cmake --build $WAZUH_PATH/build --parallel $BUILD_JOBS
        fi

        echo "Installing sources"
        cmake --install $WAZUH_PATH/build --prefix $WAZUH_PATH/install
    fi

    cp -r "${WAZUH_PATH}/install/." "${DESTINATION_PATH}/"
}

build
