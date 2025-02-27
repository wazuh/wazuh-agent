#!/bin/bash

# Wazuh package builder
# Copyright (C) 2015, Wazuh Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.
set -e

build_directories() {
  local build_folder=$1
  local wazuh_dir="$2"
  local future="$3"

  mkdir -p "${build_folder}"
  wazuh_version="$(sed -n 's/.*"version"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' wazuh*/VERSION.json)"

  if [[ "$future" == "yes" ]]; then
    wazuh_version="$(future_version "$build_folder" "$wazuh_dir" $wazuh_version)"
    source_dir="${build_folder}/wazuh-${BUILD_TARGET}-${wazuh_version}"
  else
    package_name="wazuh-${BUILD_TARGET}-${wazuh_version}"
    source_dir="${build_folder}/${package_name}"
    cp -aR $wazuh_dir "$source_dir"
  fi
  echo "$source_dir"
}

# Function to handle future version
future_version() {
  local build_folder="$1"
  local wazuh_dir="$2"
  local base_version="$3"

  specs_path="$(find $wazuh_dir -name SPECS | grep $SYSTEM)"

  local major=$(echo "$base_version" | cut -dv -f2 | cut -d. -f1)
  local minor=$(echo "$base_version" | cut -d. -f2)
  local version="${major}.30.0"
  local old_name="wazuh-${BUILD_TARGET}-${base_version}"
  local new_name=wazuh-${BUILD_TARGET}-${version}

  local new_wazuh_dir="${build_folder}/${new_name}"
  cp -R ${wazuh_dir} "$new_wazuh_dir"
  find "$new_wazuh_dir" "${specs_path}" \( -name "*VERSION*" -o -name "*changelog*" \
        -o -name "*.spec" \) -exec sed -i "s/${base_version}/${version}/g" {} \;
  sed -i 's/\("version"[[:space:]]*:[[:space:]]*"\)[^"]*\("\)/\1'"${major}.30.0"'\2/' "$new_wazuh_dir/VERSION.json"
  echo "$version"
}

# Function to generate checksum and move files
post_process() {
  local file_path="$1"
  local checksum_flag="$2"
  local source_flag="$3"

  if [[ "$checksum_flag" == "yes" ]]; then
    sha512sum "$file_path" > /var/local/checksum/$(basename "$file_path").sha512
  fi

  if [[ "$source_flag" == "yes" ]]; then
    mv "$file_path" /var/local/wazuh
  fi
}

set_vcpkg_remote_binary_cache(){
  local vcpkg_token="$1"

  if [[ $(mono --version 2>/dev/null) =~ [0-9] ]]; then
    echo "mono already installed, proceeding"
    NUGET_PATH="/usr/local/bin/nuget"
    curl -o $NUGET_PATH https://dist.nuget.org/win-x86-commandline/v6.10.2/nuget.exe
    mono $NUGET_PATH \
        sources add \
        -source "https://nuget.pkg.github.com/wazuh/index.json" \
        -name "GitHub" \
        -username "wazuh" \
        -password "$vcpkg_token"
    mono $NUGET_PATH \
        setapikey "$vcpkg_token" \
        -source "https://nuget.pkg.github.com/wazuh/index.json"
  else
    echo "mono in not installed, remote binary caching not being enabled"
  fi
}

# Main script body

# Script parameters
export REVISION="$1"
export JOBS="$2"
debug="$3"
checksum="$4"
future="$5"
src="$6"

build_dir="/build_wazuh"

source /home/helper_function.sh

if [ -n "${WAZUH_VERBOSE}" ]; then
  set -x
fi

# Download source code if it is not shared from the local host
if [ ! -d "/wazuh-local-src" ] ; then
  git clone --branch ${WAZUH_BRANCH} --single-branch https://github.com/wazuh/wazuh-agent.git
  short_commit_hash="$(curl -s https://api.github.com/repos/wazuh/wazuh-agent/commits/${WAZUH_BRANCH} \
                          | grep '"sha"' | head -n 1| cut -d '"' -f 4 | cut -c 1-11)"
else
  short_commit_hash="$(cd /wazuh-local-src && git rev-parse --short HEAD)"
fi

# Build directories
source_dir=$(build_directories "$build_dir/${BUILD_TARGET}" "wazuh*" $future)

wazuh_version="$(sed -n 's/.*"version"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' wazuh*/VERSION.json)"
# TODO: Improve how we handle package_name
# Changing the "-" to "_" between target and version breaks the convention for RPM or DEB packages.
# For now, I added extra code that fixes it.
package_name="wazuh-${BUILD_TARGET}-${wazuh_version}"
specs_path="$(find $source_dir/packages -name SPECS | grep $SYSTEM)"

setup_build "$source_dir" "$specs_path" "$build_dir" "$package_name" "$debug"

set_debug $debug $sources_dir

# Installing build dependencies
cd $sources_dir

if [ "${ARCHITECTURE_TARGET}" != "amd64" ]; then
  export VCPKG_FORCE_SYSTEM_BINARIES=1
fi

if [ -n "${VCPKG_KEY}" ]; then
  set_vcpkg_remote_binary_cache "$VCPKG_KEY"
fi

build_deps
build_package $package_name $debug "$short_commit_hash" "$wazuh_version"

# Post-processing
get_package_and_checksum $wazuh_version $short_commit_hash $src
