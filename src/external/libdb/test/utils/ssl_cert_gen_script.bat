:: ssl_cert_gen_script.bat
::
:: This script is for generating ssl certificates for Replication Manager 
:: testing on windows. 

@echo off
set cert_dir=certs
set PK_PASS=someRandomPass

IF EXIST %cert_dir% (
 	rd /S /Q %cert_dir%
)

mkdir %cert_dir%

:: Generate the Private key for CA(Certificate Authority).
openssl genrsa -passout pass:%PK_PASS% -des3	-out %cert_dir%/rootCA.key 2048

:: Generate the Self signed Certificate for the CA.
openssl req -x509 -new -nodes -key %cert_dir%/rootCA.key -text -sha256 -days 1024 -out %cert_dir%/rootCA.crt -passin pass:%PK_PASS% -subj "/C=US/ST=New York/L=Brooklyn/O=Example Brooklyn Company/CN=server"

:: Generate the private key for the replication nodes. We are
:: using the same private key for all nodes. In practice
:: private key and certificate for each node would be
:: different.
openssl genrsa -passout pass:%PK_PASS% -aes256 -out %cert_dir%/repNode.key 2048


:: Generate the CSR(certificate signing request) for the repnode.
openssl req -new -key %cert_dir%/repNode.key -out %cert_dir%/repNode.csr -passin pass:%PK_PASS% -subj "/C=US/ST=New York/L=Brooklyn/O=Example Brooklyn Company/CN=repnode"

:: Generate the signed certificate for the repnode
:: (signed using private key of CA).
openssl x509 -req -in %cert_dir%/repNode.csr -CA %cert_dir%/rootCA.crt -CAkey %cert_dir%/rootCA.key -text -CAcreateserial -text -out %cert_dir%/repNode.crt -days 500 -passin pass:%PK_PASS%

:: following would verify the repnode certificate against the
:: CA certificate.
openssl verify -CAfile %cert_dir%/rootCA.crt %cert_dir%/repNode.crt
