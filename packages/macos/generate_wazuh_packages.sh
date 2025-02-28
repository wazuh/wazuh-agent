#!/bin/bash
set -e
# Program to build and package OSX wazuh-agent
# Wazuh package generator
# Copyright (C) 2015, Wazuh Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

export PATH=/usr/local/bin:/Applications/CMake.app/Contents/bin:/opt/homebrew/bin:/opt/homebrew/sbin:$PATH
CURRENT_PATH="$( cd $(dirname ${0}) ; pwd -P )"
PACKAGED_DIRECTORY=$CURRENT_PATH/wazuh-agent/payload
ARCH="intel64"
WAZUH_SOURCE_REPOSITORY="https://github.com/wazuh/wazuh-agent"
SERVICE_PATH="/Library/LaunchDaemons/com.wazuh.agent.plist"
INSTALLATION_PATH="/Library/Application Support/Wazuh agent.app"    # Installation path.
VERSION=""                            # Default VERSION (branch/tag).
REVISION="1"                          # Package revision.
BRANCH_TAG=""                         # Branch that will be downloaded to build package.
DESTINATION="${CURRENT_PATH}/output"  # Where package will be stored.
JOBS="2"                              # Compilation jobs.
VERBOSE="no"                          # Enables the full log by using `set -exf`.
DEBUG="no"                            # Enables debug symbols while compiling.
CHECKSUM="no"                         # Enables the checksum generation.
IS_STAGE="no"                         # Enables release package naming.
CERT_APPLICATION_ID=""                # Apple Developer ID certificate to sign Apps and binaries.
CERT_INSTALLER_ID=""                  # Apple Developer ID certificate to sign pkg.
KEYCHAIN=""                           # Keychain where the Apple Developer ID certificate is.
KC_PASS=""                            # Password of the keychain.
NOTARIZE="no"                         # Notarize the package for macOS Catalina.
DEVELOPER_ID=""                       # Apple Developer ID.
ALTOOL_PASS=""                        # Temporary Application password for altool.
TEAM_ID=""                            # Team ID of the Apple Developer ID.
pkg_name=""
notarization_path=""
VCPKG_KEY=""

trap ctrl_c INT

function clean_and_exit() {
    exit_code=$1
    rm -rf "${SOURCES_DIRECTORY}"
    rm -rf $CURRENT_PATH/wazuh-agent
    exit ${exit_code}
}

function ctrl_c() {
    clean_and_exit 1
}


function notarize_pkg() {

    # Notarize the macOS package
    sleep_time="120"
    build_timestamp="$(date +"%m%d%Y%H%M%S")"
    if [ "${NOTARIZE}" = "yes" ]; then

        if sudo xcrun notarytool submit ${1} --apple-id "${DEVELOPER_ID}" --team-id "${TEAM_ID}" --password "${ALTOOL_PASS}" --wait ; then
            echo "Package is notarized and ready to go."
            echo "Adding the ticket to the package."
            if xcrun stapler staple -v "${1}" ; then
                echo "Ticket added. Ready to release the package."
                mkdir -p "${DESTINATION}/" && cp "${1}" "${DESTINATION}/"
                return 0
            else
                echo "Something went wrong while adding the package."
                clean_and_exit 1
            fi
        else
            echo "Error notarizing the package."
            clean_and_exit 1
        fi
    fi

    return 0
}

function sign_binaries() {
    if [ ! -z "${KEYCHAIN}" ] && [ ! -z "${CERT_APPLICATION_ID}" ] ; then
        security -v unlock-keychain -p "${KC_PASS}" "${KEYCHAIN}" > /dev/null
        # Sign every single binary in Wazuh's installation. This also includes library files.
        find "${PACKAGED_DIRECTORY}" -type f -print0 | xargs -0 file | grep -E 'executable|bit|XML' | cut -d: -f1 | while IFS= read -r bin; do
            codesign -f --sign "${CERT_APPLICATION_ID}" --entitlements $(find . -name "entitlements.plist") --timestamp  --options=runtime --verbose=4 "${bin}"
        done
        security -v lock-keychain "${KEYCHAIN}" > /dev/null
    fi
}

function sign_pkg() {
    if [ ! -z "${KEYCHAIN}" ] && [ ! -z "${CERT_INSTALLER_ID}" ] ; then
        # Unlock the keychain to use the certificate
        security -v unlock-keychain -p "${KC_PASS}" "${KEYCHAIN}"  > /dev/null

        # Sign the package
        productsign --sign "${CERT_INSTALLER_ID}" --timestamp ${DESTINATION}/${pkg_name} ${DESTINATION}/${pkg_name}.signed
        mv ${DESTINATION}/${pkg_name}.signed ${DESTINATION}/${pkg_name}

        security -v lock-keychain "${KEYCHAIN}" > /dev/null
    fi
}

function prepare_building_folder() {

    version="$1"
    pkg_final_name="$2"
    build_info_file="${WAZUH_PACKAGES_PATH}/specs/build-info.json"
    preinstall_script="${WAZUH_PACKAGES_PATH}/package_files/preinstall.sh"
    postinstall_script="${WAZUH_PACKAGES_PATH}/package_files/postinstall.sh"

    if [ -d "$CURRENT_PATH/wazuh-agent" ]; then

        echo "The wazuh agent building directory is present on this machine."
        echo "Removing it from the system."

        rm -rf $CURRENT_PATH/wazuh-agent
    fi

    munkipkg --create --json $CURRENT_PATH/wazuh-agent

    cp -f $build_info_file $CURRENT_PATH/wazuh-agent/

    sed -i '' "s|VERSION|$version|g" $CURRENT_PATH/wazuh-agent/$(basename $build_info_file)
    sed -i '' "s|PACKAGE_NAME|$pkg_final_name|g" $CURRENT_PATH/wazuh-agent/$(basename $build_info_file)

    cp $preinstall_script $CURRENT_PATH/wazuh-agent/scripts/preinstall
    cp $postinstall_script $CURRENT_PATH/wazuh-agent/scripts/postinstall

    sed -i '' "s|PACKAGE_ARCH|$ARCH|g" $CURRENT_PATH/wazuh-agent/scripts/preinstall

    mkdir -p ${PACKAGED_DIRECTORY}
    mkdir -p $DESTINATION
}

function build_package() {

    # Download source code
    if [ -n "$BRANCH_TAG" ]; then
        SOURCES_DIRECTORY="${CURRENT_PATH}/repository"
        WAZUH_PATH="${SOURCES_DIRECTORY}/wazuh"
        git clone -b ${BRANCH_TAG} --single-branch ${WAZUH_SOURCE_REPOSITORY} "${WAZUH_PATH}"
    else
        WAZUH_PATH="${CURRENT_PATH}/../.."
        cd $WAZUH_PATH
    fi

    short_commit_hash="$(cd "${WAZUH_PATH}" && git rev-parse --short HEAD)"
    WAZUH_PACKAGES_PATH="${WAZUH_PATH}/packages/macos"
    VERSION="$(sed -n 's/.*"version"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' ${WAZUH_PATH}/VERSION.json)"

    # Define output package name
    if [ $IS_STAGE == "no" ]; then
        pkg_name="wazuh-agent_${VERSION}-${REVISION}_${ARCH}_${short_commit_hash}"
    else
        pkg_name="wazuh-agent-${VERSION}-${REVISION}.${ARCH}"
    fi

    prepare_building_folder $VERSION $pkg_name

    # Build binaries
    ${WAZUH_PACKAGES_PATH}/package_files/build.sh "${VERBOSE}" "${PACKAGED_DIRECTORY}" "${WAZUH_PATH}" ${JOBS} ${VCPKG_KEY}

    # Sign the binaries and the libraries
    sign_binaries

    # Create package
    if munkipkg $CURRENT_PATH/wazuh-agent ; then
        echo "The wazuh agent package for macOS has been successfully built."
        mv $CURRENT_PATH/wazuh-agent/build/*.pkg $DESTINATION/
        # symbols_pkg_name="${pkg_name}_debug_symbols"
        # cp -R "${WAZUH_PATH}/src/symbols"  "${DESTINATION}"
        # zip -r "${DESTINATION}/${symbols_pkg_name}.zip" "${DESTINATION}/symbols"
        # rm -rf "${DESTINATION}/symbols"
        pkg_name=$(awk -F'"' '/"name":/ {print $4}' $CURRENT_PATH/wazuh-agent/build-info.json)
        sign_pkg
        if [[ "${CHECKSUM}" == "yes" ]]; then
            shasum -a512 "${DESTINATION}/$pkg_name" > "${DESTINATION}/$pkg_name.sha512"
            # shasum -a512 "${DESTINATION}/${symbols_pkg_name}.zip" > "${DESTINATION}/${symbols_pkg_name}.sha512"
        fi
        clean_and_exit 0
    else
        echo "ERROR: something went wrong while building the package."
        clean_and_exit 1
    fi
}

function help() {
    set +x
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "  Build options:"
    echo "    -a, --architecture <arch>     [Optional] Target architecture of the package [intel64/arm64]. By Default: intel64."
    echo "    -b, --branch <branch>         [Optional] Select Git branch [${BRANCH}]."
    echo "    -s, --store-path <path>       [Optional] Set the destination absolute path of package."
    echo "    -j, --jobs <number>           [Optional] Number of parallel jobs when compiling."
    echo "    -r, --revision <rev>          [Optional] Package revision that append to version e.g. x.x.x-rev"
    echo "    -d, --debug                   [Optional] Build the binaries with debug symbols. By default: no."
    echo "    -c, --checksum                [Optional] Generate checksum on the store path."
    echo "    --is_stage                    [Optional] Use release name in package"
    echo "    --vcpkg-binary-caching-key    [Optional] VCPK remote binary caching repository key."
    echo "    -h, --help                    [  Util  ] Show this help."
    echo "    -i, --install-deps            [  Util  ] Install build dependencies."
    echo "    -x, --install-xcode           [  Util  ] Install X-Code and brew. Can't be executed as root."
    echo "    -v, --verbose                 [  Util  ] Show additional information during the package generation."
    echo "  Signing options:"
    echo "    --keychain                    [Optional] Keychain where the Certificates are installed."
    echo "    --keychain-password           [Optional] Password of the keychain."
    echo "    --application-certificate     [Optional] Apple Developer ID certificate name to sign Apps and binaries."
    echo "    --installer-certificate       [Optional] Apple Developer ID certificate name to sign pkg."
    echo "    --notarize                    [Optional] Notarize the package for its distribution on macOS."
    echo "    --notarize-path <path>        [Optional] Path of the package to be notarized."
    echo "    --developer-id                [Optional] Your Apple Developer ID."
    echo "    --team-id                     [Optional] Your Apple Team ID."
    echo "    --altool-password             [Optional] Temporary password to use altool from Xcode."
    echo
    exit "$1"
}



function testdep() {

    if [[ ! $(brew --version 2>/dev/null) =~ [0-9] ]]; then
        echo "Error: brew not found. Download and install it."
        echo "Use $0 -x for install it."
        exit 1
    fi

    if [[ ! $(munkipkg --version 2>/dev/null) =~ [0-9] ]]; then
        echo "Error: munkipkg not found. Download and install dependencies."
        echo "Use $0 -i for install it."
        exit 1
    fi

    if [ -n "${VCPKG_KEY}" ]; then
        if [[ ! $(mono --version 2>/dev/null) =~ [0-9] ]]; then
            echo "Error: mono not found. Download and install dependencies."
            echo "Use $0 -i for install it."
            exit 1
        fi
    fi
}

function install_deps() {

    if [[ $(mono --version 2>/dev/null) =~ [0-9] ]]; then
        echo "mono already installed"
    else
        # Install mono tool
        echo "Installing mono"
        brew install mono
    fi

    if [[ $(munkipkg --version 2>/dev/null) =~ [0-9] ]]; then
        echo "munkipkg already installed"
    else
        # Install munkipkg tool
        echo "Installing munkipkg"
        git clone https://github.com/munki/munki-pkg.git ~/Developer/munki-pkg
        mkdir -p /usr/local/bin
        sudo ln -s "$HOME/Developer/munki-pkg/munkipkg" /usr/local/bin/munkipkg

        if [[ $(munkipkg --version 2>/dev/null) =~ [0-9] ]]; then
            echo "Munkipkg was correctly installed."
        else
            echo "Something went wrong installing Munkipkg."
        fi
    fi

    echo "Installing build dependencies for $(uname -m) architecture."
    if [ "$(uname -m)" = "arm64" ]; then
        brew install gcc binutils autoconf automake libtool cmake git pkg-config openssl
    else
        brew install cmake git pkg-config openssl
    fi
    exit 0
}

function install_xcode() {

    # Install brew tool. Brew will install X-Code if it is not already installed in the host.
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
    exit 0
}

function check_root() {

    if [[ $EUID -ne 0 ]]; then
        echo "This script must be run as root\n"
        exit 1
    fi
}

function main() {

    while [ -n "$1" ]
    do
        case "$1" in
        "-a"|"--architecture")
            if [ -n "$2" ]; then
                ARCH="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "-b"|"--branch")
            if [ -n "$2" ]; then
                BRANCH_TAG="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "-s"|"--store-path")
            if [ -n "$2" ]; then
                DESTINATION=$(echo "$2" | sed 's:/*$::')
                shift 2
            else
                help 1
            fi
            ;;
        "-j"|"--jobs")
            if [ -n "$2" ]; then
                JOBS="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "-r"|"--revision")
            if [ -n "$2" ]; then
                REVISION="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "-h"|"--help")
            help 0
            ;;
        "-i"|"--install-deps")
            install_deps
            ;;
        "-x"|"--install-xcode")
            install_xcode
            ;;
        "-v"|"--verbose")
            VERBOSE="yes"
            shift 1
            ;;
        "-d"|"--debug")
            DEBUG="yes"
            shift 1
            ;;
        "-c"|"--checksum")
            CHECKSUM="yes"
            shift 1
            ;;
        "--is_stage")
            IS_STAGE="yes"
            shift 1
            ;;
       "--vcpkg-binary-caching-key")
            if [ -n "$2" ]; then
                VCPKG_KEY="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--keychain")
            if [ -n "$2" ]; then
                KEYCHAIN="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--keychain-password")
            if [ -n "$2" ]; then
                KC_PASS="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--application-certificate")
            if [ -n "$2" ]; then
                CERT_APPLICATION_ID="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--installer-certificate")
            if [ -n "$2" ]; then
                CERT_INSTALLER_ID="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--notarize")
            NOTARIZE="yes"
            shift 1
            ;;
        "--notarize-path")
            if [ -n "$2" ]; then
                notarization_path="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--developer-id")
            if [ -n "$2" ]; then
                DEVELOPER_ID="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--team-id")
            if [ -n "$2" ]; then
                TEAM_ID="$2"
                shift 2
            else
                help 1
            fi
            ;;
        "--altool-password")
            if [ -n "$2" ]; then
                ALTOOL_PASS="$2"
                shift 2
            else
                help 1
            fi
            ;;
        *)
            help 1
        esac
    done

    if [ ${VERBOSE} = "yes" ]; then
        set -ex
    fi

    if [ "${ARCH}" != "intel64" ] && [ "${ARCH}" != "arm64" ]; then
        echo "Error: architecture not supported."
        echo "Supported architectures: intel64, arm64"
        exit 1
    fi

    testdep
    check_root
    build_package

    if [ "${NOTARIZE}" = "yes" ]; then
        
        notarization_path="${DESTINATION}/${pkg_name}.pkg"

        if [ -z "${notarization_path}" ]; then
            echo "The path of the package to be notarized has not been specified."
            help 1
        fi
        notarize_pkg "${notarization_path}"
    else
        echo "Notarization has not been selected or was not available for the selected building type."
    fi

    return 0
}

main "$@"
