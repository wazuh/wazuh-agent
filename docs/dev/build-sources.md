# Build from Sources

## Compilation steps for Linux and macOS

1. **Clone the Repository**

    First, clone the repository using the following command:

    ```bash
    git clone https://github.com/wazuh/wazuh-agent.git
    ```

2. **Vcpkg**

    The project uses Vcpkg for dependency management. If Vcpkg's repository is already checked out, the configuration process can be slightly accelerated by ensuring that `VCPKG_ROOT` is set to its path. Otherwise, it will be checked out during CMake's configuration phase.

3. **Configure and Build the Project**

    ```bash
    cd wazuh-agent
    cmake src -B build
    cmake --build build
    ```

## Compilation steps for Windows

1. **Clone the Repository**

    First, clone the repository using the following command:

    ```bash
    git clone https://github.com/wazuh/wazuh-agent.git
    ```

2. **Vcpkg**

    The project uses Vcpkg for dependency management. If Vcpkg's repository is already checked out, the configuration process can be slightly accelerated by ensuring that `VCPKG_ROOT` is set to its path. Otherwise, it will be checked out during CMake's configuration phase.

3. **Configure and Build the Project**

    ```bash
    cd wazuh-agent
    cmake src -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build
    ```

## Options

|Option|Description|Default|
|---|---|---|
|`BUILD_TESTS`|Enable tests compilation|`OFF`|
|`COVERAGE`|Enable coverage report|`OFF`|
|`ENABLE_CLANG_TIDY`|Check code with _clang-tidy_ (requires `clang-tidy-18`) |`ON`|
|`ENABLE_INVENTORY`|Enable Inventory module |`ON`|
|`ENABLE_LOGCOLLECTOR`|Enable Logcollector module|`ON`|
|`ENABLE_SCA`|Enable SCA module|`OFF`|

