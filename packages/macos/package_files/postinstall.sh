#! /bin/bash
# By Spransy, Derek" <DSPRANS () emory ! edu> and Charlie Scott
# Modified by Santiago Bassett (http://www.wazuh.com) - Feb 2016
# alterations by bil hays 2013
# -Switched to bash
# -Added some sanity checks
# -Added routine to find the first 3 contiguous UIDs above 100,
#  starting at 600 puts this in user space
# -Added lines to append the ossec users to the group ossec
#  so the the list GroupMembership works properly
GROUP="wazuh"
USER="wazuh"
AGENT_DIR="/Library/Application Support/Wazuh agent.app"
CONF_DIR="$AGENT_DIR/etc"
DATA_DIR="$AGENT_DIR/var"
SERVICE_FILE="/Library/LaunchDaemons/com.wazuh.agent.plist"
UPGRADE_FILE_FLAG="${AGENT_DIR}/WAZUH_PKG_UPGRADE"


if [ -f "${AGENT_DIR}"/WAZUH_RESTART ]; then
    restart="true"
    rm -f "${AGENT_DIR}"/WAZUH_RESTART
fi

if [ -f "${UPGRADE_FILE_FLAG}" ]; then
    upgrade="true"
    rm -f "${UPGRADE_FILE_FLAG}"
    echo "Restoring configuration files from "${CONF_DIR}"/config_files/ to "${CONF_DIR}"/etc/"
    rm -f "${CONF_DIR}"/wazuh-agent.yml
    cp -rf "${CONF_DIR}"/config_files/* "${CONF_DIR}"
    rm -rf "${CONF_DIR}"/config_files/
fi

# Default for all directories
echo "Seting permissions and ownership for directories and files"
chmod -R 750 "${AGENT_DIR}"/
chown -R root:"${GROUP}" "${AGENT_DIR}"/
chown -R root:wheel "${AGENT_DIR}"/bin

chmod 770 "${DATA_DIR}"
chown -R "${USER}":"${GROUP}" "${DATA_DIR}"

chmod 770 "${CONF_DIR}"
chown "${USER}":"${GROUP}" "${CONF_DIR}"

sudo chown root:wheel "$SERVICE_FILE"
sudo chmod 644 "$SERVICE_FILE"

chown "${USER}":"${GROUP}" "${CONF_DIR}"/VERSION.json
chmod 440 "${CONF_DIR}"/VERSION.json

# Remove old ossec user and group if exists and change ownwership of files
if [[ $(dscl . -read /Groups/ossec) ]]; then
    echo "Changing group from Ossec to Wazuh"
    find "${AGENT_DIR}"/ -group ossec -user root -exec chown root:wazuh {} \ > /dev/null 2>&1 || true
    if [[ $(dscl . -read /Users/ossec) ]]; then
        echo "Changing user from Ossec to Wazuh"
        find "${AGENT_DIR}"/ -group ossec -user ossec -exec chown wazuh:wazuh {} \ > /dev/null 2>&1 || true
        echo "Removing Ossec user"
        sudo /usr/bin/dscl . -delete "/Users/ossec"
    fi
    echo "Removing Ossec group"
    sudo /usr/bin/dscl . -delete "/Groups/ossec"
fi

if [ -n "${upgrade}" ] && [ -n "${restart}" ]; then
    echo "Restarting Wazuh..."
    launchctl bootout system "${SERVICE_FILE}"
    launchctl bootstrap system "${SERVICE_FILE}"
fi
