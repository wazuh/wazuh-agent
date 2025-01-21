FROM rockylinux:9

COPY wazuh-server*.rpm /tmp

RUN rpm -i /tmp/wazuh-server*.rpm && rm -f /tmp/wazuh-server*.rpm

RUN <<EOF
DESTDIR=/etc/wazuh-server/certs
NAME=server
openssl genrsa -out $DESTDIR/root-ca-key.pem 2048
openssl req -new -x509 -days 365 -key $DESTDIR/root-ca-key.pem -out $DESTDIR/root-ca.pem -nodes -subj "/CN=root-ca"
openssl genrsa -out $DESTDIR/$NAME-key.pem 2048
openssl req -new -key $DESTDIR/$NAME-key.pem -out $DESTDIR/$NAME.csr -subj "/CN=$NAME"
openssl x509 -req -in $DESTDIR/$NAME.csr -CA $DESTDIR/root-ca.pem -CAkey $DESTDIR/root-ca-key.pem -CAcreateserial -out $DESTDIR/$NAME.pem -days 365

chown wazuh-server $DESTDIR/*
chmod 600 $DESTDIR/*
rm -f $DESTDIR/{$NAME.csr,root-ca{.srl,-key.pem}}

# TODO: Remove this when the issue is fixed
chmod +x /usr/share/wazuh-server/api/scripts/wazuh-apid.py
EOF

CMD [ "/usr/share/wazuh-server/bin/wazuh-server", "start"]
