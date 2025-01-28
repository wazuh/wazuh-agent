FROM rockylinux:9

COPY wazuh-dashboard*.rpm /tmp

RUN rpm -i --nodeps /tmp/wazuh-dashboard*.rpm && rm -f /tmp/wazuh-dashboard*.rpm

COPY run.sh /usr/local/bin

CMD ["/usr/local/bin/run.sh"]
