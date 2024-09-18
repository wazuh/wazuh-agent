#! /usr/bin/env bash
# Copyright (C) 2024, Wazuh Inc.

# Clone vcpkg repository and bootstrap it
git clone --branch master --single-branch https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh
export VCPKG_ROOT=$(pwd)
export PATH=$VCPKG_ROOT:$PATH

# Check mono installation and run it if needed
if [[ $(mono --version 2>/dev/null) =~ [0-9] ]]; then
    echo "mono already installed."
else
    if [[ "$(uname)" == "Linux" ]]; then
        sudo apt install ca-certificates gnupg
        sudo gpg --homedir /tmp --no-default-keyring \
            --keyring /usr/share/keyrings/mono-official-archive-keyring.gpg \
            --keyserver hkp://keyserver.ubuntu.com:80 \
            --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
        echo "deb [signed-by=/usr/share/keyrings/mono-official-archive-keyring.gpg] https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
        sudo apt update
        sudo apt install mono-devel
    elif [[ "$(uname)" == "Darwin" ]]; then
        curl -sO https://download.mono-project.com/archive/6.12.0/macos-10-universal/MonoFramework-MDK-6.12.0.206.macos10.xamarin.universal.pkg
        sudo installer -pkg MonoFramework-MDK-6.12.0.206.macos10.xamarin.universal.pkg -target / 
    fi
fi

# Set nuget as binary caching store
mono `vcpkg fetch nuget | tail -n 1` \
    sources add \
    -source "https://nuget.pkg.github.com/wazuh/index.json" \
    -name "GitHub" \
    -username "wazuh" \
    -password "$GH_TOKEN"
mono `vcpkg fetch nuget | tail -n 1` \
    setapikey "$GH_TOKEN" \
    -source "https://nuget.pkg.github.com/wazuh/index.json"

