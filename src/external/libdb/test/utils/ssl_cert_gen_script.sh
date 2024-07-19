#!/bin/bash

# This script is used to generate the certificates and private key
# files etc. required for testing ssl support for repmgr.
# This script uses openssl interface to achieve this.
# The files are generated in the "certs" directory. To change 
# destination directory or private keyfile password change the 
# varibles 'cert_dir' or 'PK_PASS' respectively.

cert_dir=certs
PK_PASS=someRandomPass

if [ -d $cert_dir ]; then
	rm -rf $cert_dir
fi

mkdir $cert_dir

# Generate the Private key for CA(Certificate Authority).
openssl genrsa -passout pass:$PK_PASS -des3 \
	-out $cert_dir/rootCA.key 2048

# Generate the Self signed Certificate for the CA.
openssl req -x509 -new -nodes -key \
	$cert_dir/rootCA.key -text -sha256 -days 1024 \
	-out $cert_dir/rootCA.crt -passin pass:$PK_PASS \
	-subj   "/C=US/ST=New York/L=Brooklyn/O=Example \
	Brooklyn Company/CN=server"

# Generate the private key for the replication nodes. We are
# using the same private key for all nodes. In practice
# private key and certificate for each node would be
# different.
openssl genrsa -passout pass:$PK_PASS \
	-aes256 -out $cert_dir/repNode.key 2048


# Generate the CSR(certificate signing request) for the repnode.
openssl req -new -key $cert_dir/repNode.key \
	-out $cert_dir/repNode.csr -passin pass:$PK_PASS \
	-subj "/C=US/ST=New York/L=Brooklyn/O=Example \
	Brooklyn Company/CN=repnode"

# Generate the signed certificate for the repnode
# (signed using private key of CA).
openssl x509 -req -in $cert_dir/repNode.csr \
	-CA $cert_dir/rootCA.crt -CAkey $cert_dir/rootCA.key \
	-text -CAcreateserial -text -out $cert_dir/repNode.crt \
	-days 500 -passin pass:$PK_PASS

# following would verify the repnode certificate against the
# CA certificate.
openssl verify -CAfile \
	$cert_dir/rootCA.crt $cert_dir/repNode.crt

tee -a config_nix << EOF
disable_ssl false
ca_cert $cert_dir/rootCA.crt
node_cert $cert_dir/repNode.crt
node_pkey $cert_dir/repNode.key
pkey_passwd $PK_PASS
verify_depth 6
EOF

