#!/bin/bash

source ./src/init/install_helpers.sh

sys=$(get_system)
set_dafault_installation_variables $sys
build_binary
add_user
create_installation_directories
install_files
