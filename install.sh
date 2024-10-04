#!/bin/bash

source ./src/init/install_helpers.sh

echo "Running wazuh-agent installation"

system=$(get_system)
set_dafault_installation_variables $system
build_binary
add_user
create_installation_directories
install_files

echo "Installation completed"
