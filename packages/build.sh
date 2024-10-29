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
  wazuh_version="$(cat wazuh*/src/VERSION| cut -d 'v' -f 2)"

  if [[ "$future" == "yes" ]]; then
    wazuh_version="$(future_version "$build_folder" "$wazuh_dir" $wazuh_version)"
    source_dir="${build_folder}/wazuh-${BUILD_TARGET}-${wazuh_version}"
  else
    package_name="wazuh-${BUILD_TARGET}-${wazuh_version}"
    source_dir="${build_folder}/${package_name}"
    cp -R $wazuh_dir "$source_dir"
  fi
  echo "$source_dir"
}

# Function to handle future version
future_version() {
  local build_folder="$1"
  local wazuh_dir="$2"
  local base_version="$3"

  specs_path="$(find $wazuh_dir -name SPECS|grep $SYSTEM)"

  local major=$(echo "$base_version" | cut -dv -f2 | cut -d. -f1)
  local minor=$(echo "$base_version" | cut -d. -f2)
  local version="${major}.30.0"
  local old_name="wazuh-${BUILD_TARGET}-${base_version}"
  local new_name=wazuh-${BUILD_TARGET}-${version}

  local new_wazuh_dir="${build_folder}/${new_name}"
  cp -R ${wazuh_dir} "$new_wazuh_dir"
  find "$new_wazuh_dir" "${specs_path}" \( -name "*VERSION*" -o -name "*changelog*" \
        -o -name "*.spec" \) -exec sed -i "s/${base_version}/${version}/g" {} \;
  sed -i "s/\$(VERSION)/${major}.${minor}/g" "$new_wazuh_dir/src/Makefile"
  sed -i "s/${base_version}/${version}/g" $new_wazuh_dir/src/init/wazuh-{server,client,local}.sh
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
      echo "mono already installed."
  else
    if [ -n "$(command -v yum)" ]; then
      rpmkeys --import "http://keyserver.ubuntu.com/pks/lookup?op=get&search=0x3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF"
      su -c 'curl https://download.mono-project.com/repo/centos7-stable.repo | tee /etc/yum.repos.d/mono-centos7-stable.repo'
      yum install mono-devel -y
    elif [ -n "$(command -v dpkg)" ]; then
      apt install ca-certificates gnupg
      gpg --homedir /tmp --no-default-keyring \
        --keyring /usr/share/keyrings/mono-official-archive-keyring.gpg \
        --keyserver hkp://keyserver.ubuntu.com:80 \
        --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
      echo "deb [signed-by=/usr/share/keyrings/mono-official-archive-keyring.gpg] https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
      apt update
      apt install mono-devel -y
    else
      echo "Couldn't find type of system"
      exit 1
    fi
  fi

  export VCPKG_INSTALL_OPTIONS='--debug'
  $sources_dir/src/vcpkg/bootstrap-vcpkg.sh
  
  mono `$sources_dir/src/vcpkg/vcpkg fetch nuget | tail -n 1` \
      sources add \
      -source "https://nuget.pkg.github.com/wazuh/index.json" \
      -name "GitHub" \
      -username "wazuh" \
      -password "$vcpkg_token"
  mono `$sources_dir/src/vcpkg/vcpkg fetch nuget | tail -n 1` \
      setapikey "$vcpkg_token" \
      -source "https://nuget.pkg.github.com/wazuh/index.json"
}

# Main script body

# Script parameters
export REVISION="$1"
export JOBS="$2"
debug="$3"
checksum="$4"
future="$5"
legacy="$6"
src="$7"

build_dir="/build_wazuh"

source /home/helper_function.sh

if [ -n "${WAZUH_VERBOSE}" ]; then
  set -x
fi

# Download source code if it is not shared from the local host
if [ ! -d "/wazuh-local-src" ] ; then
    git clone --branch ${WAZUH_BRANCH} --single-branch --recurse-submodules https://github.com/wazuh/wazuh-agent.git
    short_commit_hash="$(curl -s https://api.github.com/repos/wazuh/wazuh-agent/commits/${WAZUH_BRANCH} \
                          | grep '"sha"' | head -n 1| cut -d '"' -f 4 | cut -c 1-11)"
else
    if [ "${legacy}" = "no" ]; then
      short_commit_hash="$(cd /wazuh-local-src && git rev-parse --short HEAD)"
    else
      # Git package is not available in the CentOS 5 repositories.
      hash_commit=$(cat /wazuh-local-src/.git/$(cat /wazuh-local-src/.git/HEAD|cut -d" " -f2))
      short_commit_hash="$(cut -c 1-11 <<< $hash_commit)"
    fi

fi

# Build directories
source_dir=$(build_directories "$build_dir/${BUILD_TARGET}" "wazuh*" $future)

wazuh_version="$(cat wazuh*/src/VERSION| cut -d 'v' -f 2)"
# TODO: Improve how we handle package_name
# Changing the "-" to "_" between target and version breaks the convention for RPM or DEB packages.
# For now, I added extra code that fixes it.
package_name="wazuh-${BUILD_TARGET}-${wazuh_version}"
specs_path="$(find $source_dir -name SPECS|grep $SYSTEM)"

setup_build "$source_dir" "$specs_path" "$build_dir" "$package_name" "$debug"

set_debug $debug $sources_dir

# Installing build dependencies
cd $sources_dir

echo $sources_dir
echo "antes de vcpkg"

if [ -n "${VCPKG_KEY}" ]; then
  set_vcpkg_remote_binary_cache "$VCPKG_KEY"
else
  export VCPKG_MAX_CONCURRENCY=1
fi

build_deps $legacy
build_package $package_name $debug "$short_commit_hash" "$wazuh_version"

# Post-processing
get_package_and_checksum $wazuh_version $short_commit_hash $src
