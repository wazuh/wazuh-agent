#! /usr/bin/env bash
# Copyright (C) 2024, Wazuh Inc.

# Check mono installation and run it if needed
if [ -z "$1" ]; then
    echo "Missing TOKEN, provide it."
    exit 1
fi

if [[ $(mono --version 2>/dev/null) =~ [0-9] ]]; then
    echo "mono already installed."
else
    if [[ "$(uname)" == "Linux" ]]; then
        sudo apt install -y ca-certificates gnupg
        sudo gpg --homedir /tmp --no-default-keyring \
            --keyring /usr/share/keyrings/mono-official-archive-keyring.gpg \
            --keyserver hkp://keyserver.ubuntu.com:80 \
            --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
        echo "deb [signed-by=/usr/share/keyrings/mono-official-archive-keyring.gpg] https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
        sudo apt update
        sudo apt install -y mono-devel
    elif [[ "$(uname)" == "Darwin" ]]; then
        curl -sO https://download.mono-project.com/archive/6.12.0/macos-10-universal/MonoFramework-MDK-6.12.0.206.macos10.xamarin.universal.pkg
        sudo installer -pkg MonoFramework-MDK-6.12.0.206.macos10.xamarin.universal.pkg -target / 
    fi
fi

# Set nuget as binary caching store
NUGET_PATH="/usr/local/bin/nuget"
curl -o $NUGET_PATH https://dist.nuget.org/win-x86-commandline/v6.10.2/nuget.exe
mono $NUGET_PATH \
    sources add \
    -source "https://nuget.pkg.github.com/wazuh/index.json" \
    -name "GitHub" \
    -username "wazuh" \
    -password "$1"
mono $NUGET_PATH \
    setapikey "$1" \
    -source "https://nuget.pkg.github.com/wazuh/index.json"
