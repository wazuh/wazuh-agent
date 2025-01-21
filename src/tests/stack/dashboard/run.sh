#! /bin/bash

mkdir /etc/wazuh-dashboard/certs
cp /etc/wazuh-indexer/certs/{root-ca,admin{,-key}}.pem /etc/wazuh-dashboard/certs

chown -R wazuh-dashboard /etc/wazuh-dashboard/certs
chmod 400 /etc/wazuh-dashboard/certs/*.pem

runuser -u wazuh-dashboard -- /usr/share/wazuh-dashboard/bin/opensearch-dashboards
