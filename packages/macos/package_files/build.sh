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
SOURCES_PATH=$2
BUILD_JOBS=$3
DEBUG=$4
MAKE_COMPILATION=$5
INSTALLATION_SCRIPTS_DIR=${DESTINATION_PATH}/packages_files/agent_installation_scripts
SEARCH_DIR=${SOURCES_PATH}/src

set_vcpkg_remote_binary_cache(){
  local vcpkg_token="$1"

  if [[ $(mono --version 2>/dev/null) =~ [0-9] ]]; then
    echo "mono already installed, proceeding"
    export VCPKG_BINARY_SOURCES="clear;nuget,GitHub,readwrite"
    $SOURCES_PATH/src/vcpkg/bootstrap-vcpkg.sh
    mono `$SOURCES_PATH/src/vcpkg/vcpkg fetch nuget | tail -n 1` \
        sources add \
        -source "https://nuget.pkg.github.com/wazuh/index.json" \
        -name "GitHub" \
        -username "wazuh" \
        -password "$vcpkg_token"
    mono `$SOURCES_PATH/src/vcpkg/vcpkg fetch nuget | tail -n 1` \
        setapikey "$vcpkg_token" \
        -source "https://nuget.pkg.github.com/wazuh/index.json"  
  else
    echo "mono in not installed, remote binary caching not being enabled"
  fi
}

function build() {

    set_vcpkg_remote_binary_cache

    if [ "${MAKE_COMPILATION}" == "yes" ]; then
        make -C ${SOURCES_PATH}/src deps TARGET=agent

        echo "Generating Wazuh executables"
        make -j $BUILD_JOBS -C ${SOURCES_PATH}/src DYLD_FORCE_FLAT_NAMESPACE=1 DEBUG=$DEBUG TARGET=agent build
    fi

    EXECUTABLE_FILES=$(find "${SEARCH_DIR}" -maxdepth 1 -type f ! -name "*.py" -exec file {} + | grep 'executable' | cut -d: -f1)
    EXECUTABLE_FILES+=" $(find "${SEARCH_DIR}" -type f ! -name "*.py" ! -path "${SEARCH_DIR}/external/*" ! -path "${SEARCH_DIR}/symbols/*" -name "*.dylib" -print 2>/dev/null)"

    for var in $EXECUTABLE_FILES; do
        filename=$(basename "$var")
        dsymutil -o "${SEARCH_DIR}/symbols/${filename}.dSYM" "$var" 2>/dev/null && strip -S "$var"
    done

    echo "Running install script"
    ${SOURCES_PATH}/install.sh

    find ${DESTINATION_PATH}/ruleset/sca/ -type f -exec rm -f {} \;

    # Add the auxiliar script used while installing the package
    mkdir -p ${INSTALLATION_SCRIPTS_DIR}/
    cp ${SOURCES_PATH}/gen_ossec.sh ${INSTALLATION_SCRIPTS_DIR}/
    cp ${SOURCES_PATH}/add_localfiles.sh ${INSTALLATION_SCRIPTS_DIR}/

    mkdir -p ${INSTALLATION_SCRIPTS_DIR}/src/init
    mkdir -p ${INSTALLATION_SCRIPTS_DIR}/etc/templates/config/{generic,darwin}

    cp -r ${SOURCES_PATH}/etc/templates/config/generic ${INSTALLATION_SCRIPTS_DIR}/etc/templates/config
    cp -r ${SOURCES_PATH}/etc/templates/config/darwin ${INSTALLATION_SCRIPTS_DIR}/etc/templates/config

    find ${SOURCES_PATH}/src/init/ -name *.sh -type f -exec install -m 0640 {} ${INSTALLATION_SCRIPTS_DIR}/src/init \;

    mkdir -p ${INSTALLATION_SCRIPTS_DIR}/sca/generic
    mkdir -p ${INSTALLATION_SCRIPTS_DIR}/sca/darwin/{15,16,17,18,20,21,22,23,24}

    cp -r ${SOURCES_PATH}/ruleset/sca/darwin ${INSTALLATION_SCRIPTS_DIR}/sca
    cp -r ${SOURCES_PATH}/ruleset/sca/generic ${INSTALLATION_SCRIPTS_DIR}/sca
    cp ${SOURCES_PATH}/etc/templates/config/generic/sca.files ${INSTALLATION_SCRIPTS_DIR}/sca/generic/

    for n in $(seq 15 24); do
        cp ${SOURCES_PATH}/etc/templates/config/darwin/$n/sca.files ${INSTALLATION_SCRIPTS_DIR}/sca/darwin/$n/
    done

    cp ${SOURCES_PATH}/src/VERSION ${INSTALLATION_SCRIPTS_DIR}/src/
    cp ${SOURCES_PATH}/src/REVISION ${INSTALLATION_SCRIPTS_DIR}/src/
}

build
