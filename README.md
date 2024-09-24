# Wazuh Agent

[![Slack](https://img.shields.io/badge/slack-join-blue.svg)](https://wazuh.com/community/join-us-on-slack/)
[![Email](https://img.shields.io/badge/email-join-blue.svg)](https://groups.google.com/forum/#!forum/wazuh)
[![Documentation](https://img.shields.io/badge/docs-view-green.svg)](https://documentation.wazuh.com)
[![Web](https://img.shields.io/badge/web-view-green.svg)](https://wazuh.com)
[![Twitter](https://img.shields.io/twitter/follow/wazuh?style=social)](https://twitter.com/wazuh)
[![YouTube](https://img.shields.io/youtube/views/peTSzcAueEc?style=social)](https://www.youtube.com/watch?v=peTSzcAueEc)

>[!NOTE]
**Work in progress:** This project is currently under development. It is not functional and is not compatible with the official release version of the Wazuh manager.

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [3rd Party Software Used](#3rd-party-software-used)
4. [License](#license)

## Introduction

Wazuh is a free and open-source platform for threat prevention, detection, and response, capable of protecting workloads across on-premises, virtualized, containerized, and cloud-based environments.

This repository contains the Wazuh Agent, a key component in the Wazuh solution. The agent is deployed on monitored systems to collect data, which is then sent to the Wazuh Server for analysis. Wazuh has full integration with OpenSearch, offering powerful search capabilities and visualization tools for navigating security alerts.

## Installation

To install the Wazuh Agent, follow the steps below:

1. Clone the repository:
    ```bash
    git clone https://github.com/wazuh/wazuh-agent.git
    cd wazuh-agent
    ```
2. Build and install the agent: see [BUILD.md](BUILD.md).

For more detailed installation instructions, please refer to the Wazuh documentation.

## 3rd Party Software Used

This project uses the following third-party software:

| Software                                                                      | Description                                                      | License                        | Version |
| ----------------------------------------------------------------------------- | ---------------------------------------------------------------- | ------------------------------ | ------- |
| [Boost Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) | Cross-platform C++ library for network programming               | Boost Software License 1.0     | 1.86.0  |
| [Boost Beast](https://www.boost.org/doc/libs/release/libs/beast/)             | Library built on Boost Asio for HTTP and WebSocket communication | Boost Software License 1.0     | 1.86.0  |
| [Boost Uuid](https://www.boost.org/doc/libs/release/libs/uuid/)               | Provides support for universally unique identifiers (UUIDs)      | Boost Software License 1.0     | 1.86.0  |
| [fmt](https://fmt.dev/)                                                       | A formatting library for C++                                     | MIT License                    | 11.0    |
| [gtest](https://github.com/google/googletest)                                 | Google's C++ testing framework                                   | BSD-3-Clause                   | 1.15.2  |
| [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)                              | C++ library for handling JSON Web Tokens (JWT)                   | MIT License                    | 0.7.0   |
| [librpm](https://github.com/rpm-software-management/rpm)                      | RPM package manager                                              | GPL-2.0                        | 4.18.2  |
| [nlohmann-json](https://github.com/nlohmann/json)                             | JSON parsing and serialization library for C++                   | MIT License                    | 3.11.3  |
| [OpenSSL](https://www.openssl.org/)                                           | Toolkit for SSL/TLS protocols                                    | Apache 2.0 and OpenSSL License | 3.3.2   |
| [procps](https://github.com/warmchang/procps)                                 | Utilities for monitoring system processes and resources          | GPL-2.0                        | 3.3.0   |
| [spdlog](https://github.com/gabime/spdlog)                                    | Fast C++ logging library                                         | MIT License                    | 1.14.1  |
| [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp)                           | C++ wrapper around the SQLite database library                   | MIT License                    | 3.3.2   |
| [toml11](https://github.com/ToruNiina/toml11)                                 | C++11 TOML parser library                                        | MIT License                    | 4.0.0   |

## License

© 2024 Wazuh Inc.

This project is licensed under the [AGPL-3.0](https://www.gnu.org/licenses/agpl-3.0.html) License.
