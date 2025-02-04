#!/bin/bash

package_name=$(ls /packages | grep "wazuh")

echo "Installing Wazuh agent $package_name"

if [ -n "$(command -v yum)" ]; then
    install="yum install -y --nogpgcheck"
    list_packages_command="yum list"
elif [ -n "$(command -v apt)" ]; then
    install="apt install -y"
    list_packages_command="apt list"
    apt update && apt install lsb-release -y
else
    common_logger -e "Couldn't find type of system"
    exit 1
fi

$install "/packages/$package_name" && $list_packages_command | grep "wazuh-agent" | tee /packages/status.log
