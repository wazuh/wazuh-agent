FROM rockylinux:9

COPY wazuh-agent*.rpm /tmp
RUN rpm -i /tmp/wazuh-agent*.rpm && rm -f /tmp/wazuh-agent*.rpm
COPY run.sh /usr/local/bin

CMD [ "/usr/local/bin/run.sh" ]
