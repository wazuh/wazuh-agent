# Security

## Certificate Validation

The agent now includes functionality to validate whether the server's certificate is signed by a trusted Certificate Authority (CA).

### Validation Modes

The agent supports three validation modes:

- `full`: Verifies both that the certificate is signed by a trusted CA and that the SAN/CN belongs to the host being connected to
- `certificate`: Verifies only that the certificate is signed by a trusted CA
- `none`: Skips all certificate validation (default)

### Usage

#### During Agent Enrollment

Use the `--verification-mode` flag when enrolling the agent:

```bash
./wazuh-agent --enroll --user wazuh --password wazuh --enroll-url https://server-ip:55000 --verification-mode certificate
```

#### During Agent Connection

Edit or add the `verification_mode` configuration option in your agent configuration:

```yaml
agent:
  thread_count: 4
  server_url: https://server-ip:27000
  retry_interval: 10s
  verification_mode: full
```

### Adding Custom Certificates

#### Ubuntu

1. Copy the certificate to the CA certificates directory:
   ```bash
   sudo cp myca.crt /usr/local/share/ca-certificates/
   ```

2. Update the CA certificates:
   ```bash
   sudo update-ca-certificates
   ```

   Example output:
   ```
   Updating certificates in /etc/ssl/certs...
   rehash: warning: skipping ca-certificates.crt,it does not contain exactly one certificate or CRL
   1 added, 0 removed; done.
   Running hooks in /etc/ca-certificates/update.d...
   Adding debian:myca.pem
   done.
   done.
   ```

#### RHEL/CentOS

1. Copy the certificate to the trust anchors directory:
   ```bash
   sudo cp MyCA.crt /usr/share/pki/ca-trust-source/anchors/
   ```

2. Update the CA trust store:
   ```bash
   sudo update-ca-trust
   ```

#### macOS 14

1. Copy the certificate to a local directory
2. Import the certificate to macOS Keychain Access:
   ```bash
   sudo security add-trusted-cert -d -r trustRoot -k /Library/Keychains/System.keychain MyCA.crt
   ```

#### Windows 11

There are two methods to import certificates:

##### GUI Method
1. Copy the certificate to a local directory
2. Double-click the certificate file
3. Click "Install Certificate" and follow the UI prompts

##### MMC Method
1. Open the Local Machine Certificate Manager (Win+R, type `certlm.msc`)
2. Navigate to "Trusted Root Certification Authorities" ? "Certificates"
3. Right-click, select "All Tasks" ? "Import"
4. Follow the Certificate Import Wizard prompts
