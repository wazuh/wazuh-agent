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
2. Use one of the following options:

    - [Build from sources](docs/dev/build-sources.md)
    - [Build packages](docs/dev/build-packages.md)
    - [Build a Docker container image](docs/dev/build-image.md)

For more detailed installation instructions, please refer to the Wazuh documentation.

## 3rd Party Software Used

This project uses the following third-party software:

| Software                                                                              | Description                                                      | License                        | Version |
| ------------------------------------------------------------------------------------  | ---------------------------------------------------------------- | ------------------------------ | ------- |
| [Boost Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)         | Cross-platform C++ library for network programming               | Boost Software License 1.0     | 1.85.0  |
| [Boost Beast](https://www.boost.org/doc/libs/release/libs/beast/)                     | Library built on Boost Asio for HTTP and WebSocket communication | Boost Software License 1.0     | 1.85.0  |
| [Boost Program Options](https://www.boost.org/doc/libs/release/libs/program_options/) | Command-line options library for C++                             | Boost Software License 1.0     | 1.85.0  |
| [Boost Uuid](https://www.boost.org/doc/libs/release/libs/uuid/)                       | Provides support for universally unique identifiers (UUIDs)      | Boost Software License 1.0     | 1.85.0  |
| [Boost Url](https://www.boost.org/doc/libs/release/libs/url/)                         | Provides containers and algorithms which model a URL             | Boost Software License 1.0     | 1.85.0  |
| [cjson](https://github.com/DaveGamble/cJSON)                                          | Ultralightweight JSON parser in ANSI C                           | MIT License                    | 1.7.17  |
| [fmt](https://fmt.dev/)                                                               | A formatting library for C++                                     | MIT License                    | 10.2.1  |
| [gtest](https://github.com/google/googletest)                                         | Google's C++ testing framework                                   | BSD-3-Clause                   | 1.15.2  |
| [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)                                      | C++ library for handling JSON Web Tokens (JWT)                   | MIT License                    | 0.7.0   |
| [libarchive](https://www.libarchive.org)                                              | Library for reading and writing streaming archives               | 3-Clause New BSD License       | 3.7.5   |
| [libdb](https://github.com/yasuhirokimura/db18)                                       | Database management library (Linux only)                         | AGPL-3.0                       | 18.1.40 |
| [libplist](https://libimobiledevice.org/)                                             | A library to handle Apple Property List format (macOS only)      | LGPL-2.1-or-later              | 2023-06-15#1 |
| [libpopt](https://github.com/rpm-software-management/popt)                            | Library for parsing command line parameters                      | MIT License                    | 1.16#17 |
| [librpm](https://github.com/rpm-software-management/rpm)                              | RPM package manager (Linux only)                                 | GPL-2.0                        | 4.18.2  |
| [nlohmann-json](https://github.com/nlohmann/json)                                     | JSON parsing and serialization library for C++                   | MIT License                    | 3.11.3  |
| [OpenSSL](https://www.openssl.org/)                                                   | Toolkit for SSL/TLS protocols                                    | Apache 2.0 and OpenSSL License | 3.3.2   |
| [procps](https://github.com/warmchang/procps)                                         | Utilities for monitoring system processes and resources (Linux only) | GPL-2.0                   | 3.3.0   |
| [spdlog](https://github.com/gabime/spdlog)                                            | Fast C++ logging library                                         | MIT License                    | 1.14.0  |
| [sqlite3](https://sqlite.org/)                                                        | Self-contained SQL database engine                               | Public Domain (no restrictions) | 3.45.0#0 |
| [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp)                                   | C++ wrapper around the SQLite database library                   | MIT License                    | 3.3.2   |
| [yaml-cpp](https://github.com/jbeder/yaml-cpp)                                        | YAML parser and emitter for C++                                  | MIT License                    | 0.8.0   |

## License

© 2024 Wazuh Inc.

This project is licensed under the [AGPL-3.0](https://www.gnu.org/licenses/agpl-3.0.html) License.
