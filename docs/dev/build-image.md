# Build a Container Image

This guide describes how to build Docker containers that install the Wazuh Agent from package files. Two container
setups are provided: one for RPM-based systems (RHEL) and another for DEB-based systems (Ubuntu).

## Prerequisites

- **Docker:** Ensure Docker is installed and running on your system.
- **Data Directory:** Create a directory named `data` in your working folder and place the corresponding Wazuh Agent package:
  - **RPM Container:** Place `wazuh-agent*.rpm` in the `data` folder.
  - **DEB Container:** Place `wazuh-agent*.deb` in the `data` folder.

## RPM-Based Container (RHEL)

### Dockerfile

```dockerfile
FROM generic/rhel9

# Copy the data directory containing the RPM package
COPY data /tmp/data

# Install the Wazuh Agent RPM, download and extract mitmproxy,
# and attempt to stop/disable firewalld
RUN rpm -i /tmp/data/wazuh-agent*.rpm && \
    curl -L https://downloads.mitmproxy.org/11.0.0/mitmproxy-11.0.0-linux-x86_64.tar.gz -o /tmp/mitmproxy.tar.gz && \
    tar -zxf /tmp/mitmproxy.tar.gz -C /usr/local/bin && \
    (systemctl stop firewalld || true) && \
    (systemctl disable firewalld || true)

CMD ["/bin/bash"]
```

### Build and Run (RPM Container)

1. **Build the Image:**

    ```bash
    docker build -t rpm-agent .
    ```

2. **Run the Container with a Custom Hostname:**

    ```bash
    docker run --hostname rpm-agent -it rpm-agent
    ```

## DEB-Based Container (Ubuntu)

### Dockerfile

```dockerfile
FROM ubuntu:24.04

# Update repositories and install required dependencies
RUN apt-get update && apt-get install -y lsb-release adduser curl

# Copy the data directory containing the DEB package
COPY data /tmp/data

# Install the Wazuh Agent DEB package
RUN dpkg -i /tmp/data/wazuh-agent*.deb

# Download and extract mitmproxy
RUN curl -L https://downloads.mitmproxy.org/11.0.0/mitmproxy-11.0.0-linux-x86_64.tar.gz -o /tmp/mitmproxy.tar.gz && \
    tar -zxf /tmp/mitmproxy.tar.gz -C /usr/local/bin

CMD ["/bin/bash"]
```

### Build and Run (DEB Container)

1. **Build the Image:**

    ```bash
    docker build -t deb-agent .
    ```

2. **Run the Container with a Custom Hostname:**

    ```bash
    docker run --hostname deb-agent -it deb-agent
    ```
