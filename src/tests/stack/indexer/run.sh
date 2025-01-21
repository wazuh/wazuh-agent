#! /bin/bash

set -m

runuser -u wazuh-indexer -- /usr/share/wazuh-indexer/bin/opensearch &

while ! curl -k https://localhost:9200 2> /dev/null; do
    sleep 10
done

/usr/share/wazuh-indexer/bin/indexer-security-init.sh

fg %1
