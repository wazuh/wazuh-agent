#! /bin/bash

set -e

/usr/share/wazuh-agent/bin/wazuh-agent --register-agent \
    --url https://server:55000 \
    --user wazuh \
    --password wazuh \
    --name dummy \
    --verification-mode none

/usr/share/wazuh-agent/bin/wazuh-agent
