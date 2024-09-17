#! /usr/bin/env bash
# Copyright (C) 2024, Wazuh Inc.
          
git clone --branch master --single-branch https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh 
export VCPKG_ROOT=$(pwd)
export PATH=$VCPKG_ROOT:$PATH
