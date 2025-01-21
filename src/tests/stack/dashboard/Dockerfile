FROM debian:12

COPY wazuh-dashboard*.deb /tmp

RUN <<EOF
apt-get update
apt-get install -f -y /tmp/wazuh-dashboard*.deb
rm -f /tmp/wazuh-dashboard*.deb
rm -rf /var/lib/apt/lists/*
EOF

COPY run.sh /usr/local/bin

CMD ["/usr/local/bin/run.sh"]
