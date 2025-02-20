#!/bin/bash
if [ -n "$(command -v yum)" ]; then
    install="yum install -y --nogpgcheck"
    list_packages_command="yum list"
    package_extension="rpm"
elif [ -n "$(command -v apt)" ]; then
    install="apt install -y"
    list_packages_command="apt list"
    package_extension="deb"
    apt update && apt install lsb-release -y
else
    common_logger -e "Couldn't find type of system"
    exit 1
fi

package_name=$(ls /packages | grep "wazuh.*$package_extension$")
echo "Installing Wazuh agent $package_name"
$install "/packages/$package_name" && $list_packages_command | grep "wazuh-agent" | tee /packages/status.log
