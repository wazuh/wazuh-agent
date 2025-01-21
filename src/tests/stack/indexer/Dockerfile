FROM rockylinux:9

COPY wazuh-indexer*.rpm /tmp

RUN rpm -i /tmp/wazuh-indexer*.rpm && rm -f /tmp/wazuh-indexer*.rpm

COPY run.sh /usr/local/bin
CMD ["/usr/local/bin/run.sh"]
