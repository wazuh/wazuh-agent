#! /usr/bin/env bash
# Copyright (C) 2024, Wazuh Inc.

mono `vcpkg fetch nuget | tail -n 1` \
    sources add \
    -source "https://nuget.pkg.github.com/wazuh/index.json" \
    -name "GitHub" \
    -username "wazuh" \
    -password "$GH_TOKEN"
mono `vcpkg fetch nuget | tail -n 1` \
    setapikey "$GH_TOKEN" \
    -source "https://nuget.pkg.github.com/wazuh/index.json"
