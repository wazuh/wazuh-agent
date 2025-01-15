# Build Instructions

The following dependencies are required for this project:

- **Git**
- **CMake** at least 3.22
- **Make**
- **C++ compiler** (GCC 10 or Clang 13, coroutines support needed)
- **Zip** (for [vcpkg](https://vcpkg.io))
- **Curl** (for [vcpkg](https://vcpkg.io))
- **Tar** (for [vcpkg](https://vcpkg.io))
- **Ninja-build** (for [vcpkg](https://vcpkg.io))
- **Pkg-config**

## Compilation steps for Linux

1. **Installing Dependencies on Debian**

    To install the necessary dependencies on a Debian-based system, run the following commands:

    ```bash
    sudo apt-get update
    sudo apt-get install cmake make gcc git zip curl tar ninja-build pkg-config wget gnupg lsb-release software-properties-common libbz2-dev
    wget https://apt.llvm.org/llvm.sh
    chmod +x llvm.sh
    sudo ./llvm.sh 18
    sudo apt-get update
    sudo apt-get install -y clang-tidy-18 autopoint libtool zlib1g-dev libgcrypt20-dev libmagic-dev libpopt-dev libmagic-dev libsqlite3-dev liblua5.4-dev gettext libarchive-dev
    ```

2. **Clone the Repository**

    First, clone the repository using the following command:

    ```bash
    git clone https://github.com/wazuh/wazuh-agent.git
    ```

3. **Initialize Submodules**

    The project uses submodules, so you need to initialize and update them. Run the following commands:

    ```bash
    cd wazuh-agent
    git submodule update --init --recursive
    ```

4. **Configure and Build the Project**

    ```bash
    cmake src -B build
    cmake --build build
    ```

    If you want to include tests, configure the project with the following command:

    ```bash
    cmake src -B build -DBUILD_TESTS=1
    cmake --build build
    ```

5. **Run the Agent**

    **To run the agent in the foreground from the CLI**

    You can start and get status with:

    ```bash
    ./wazuh-agent
    ./wazuh-agent --status
    ```

    **To run the agent as a systemd service**

    Copy the file `src/agent/service/wazuh-agent.service` to `/usr/lib/systemd/system/`.
    Replace the placeholder WAZUH_HOME to your wazuh-agent executable directory.
    Reload unit files.

    ```bash
    systemctl daemon-reload
    ```

    Enable service.

    ```bash
    systemctl enable wazuh-agent
    ```

    You can start and stop the agent, and get status from systemctl:

    ```bash
    systemctl start wazuh-agent
    systemctl stop wazuh-agent
    systemctl is-active wazuh-agent
    systemctl status wazuh-agent
    ```

6. **Run tests**

    If built with CMake and `-DBUILD_TESTS=1`, you can run tests with:

    ```bash
    ctest --test-dir build --output-log build
    ```

## Compilation steps for macOS

1. **Install brew, a package manager for macOS**
    ```bash
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    ```

2. **Install dependencies**
    ```bash
    brew install automake autoconf libtool cmake pkg-config openssl
    ```

3. **Clone the Repository**

    Clone the repository:

    ```bash
    git clone https://github.com/wazuh/wazuh-agent.git
    ```

4. **Initialize Submodules**

    The project uses submodules, so you need to initialize and update them. Run the following commands:

    ```bash
    cd wazuh-agent
    git submodule update --init --recursive
    ```

5. **Configure and Build the Project**

    ```bash
    cmake src -B build
    cmake --build build
    ```

    If you want to include tests, configure the project with the following command:

    ```bash
    cmake src -B build -DBUILD_TESTS=1
    cmake --build build
    ```

6. **Run the Agent**

    **To run the agent in the foreground from the CLI**

    You can start and get status with:

    ```bash
    ./wazuh-agent
    ./wazuh-agent --status
    ```

    **To run the agent as a launchd service**

    Copy the property list file `src/agent/service/com.wazuh.agent.plist` to `/Library/LaunchDaemons/`.
    Edit the file and replace the placeholder path with your wazuh-agent executable directory as well as the working directory.

    ```bash
    sudo chown root:wheel /Library/LaunchDaemons/com.wazuh.agent.plist
    sudo chmod 644 /Library/LaunchDaemons/com.wazuh.agent.plist
    ```

    ***Load the service***

    ```bash
    sudo launchctl bootstrap system /Library/LaunchDaemons/com.wazuh.agent.plist
    ```

    This command has superseeded `load` in the legacy syntax. The daemon will run after load as indicated in the property list file.


    ***Unload the service***

    ```bash
    sudo launchctl bootout system /Library/LaunchDaemons/com.wazuh.agent.plist
    ```

    This command has superseeded `unload` in the legacy syntax.


    ***Verify the service is running***

    ```bash
    sudo launchctl print system/com.wazuh.agent
    ```

7. **Run tests**

    If built with CMake and `-DBUILD_TESTS=1`, you can run tests with:

    ```bash
    ctest --test-dir build --output-log build
    ```

8. **Package Installation**

    If you wish to install a package use the following command.

    ```bash
    sudo installer -pkg /path/to/package/package_filename.pkg -target /
    ```

    Then load and manage the service as indicated in point 6 above. Skip the property list file copying and editing as well as the permissions part - the installer takes care of that.

## Compilation steps for Windows

1. **Installing Dependencies**

- Visual Studio Community 2022 (with MSVC 14)
- Chocolatey
    ```bash
    Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    ```
- Make
    ```bash
    choco install make
    ```
- Cmake 3.30.x
    ```bash
    choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
    ```
- OpenSSL
    ```bash
    choco install openssl
    ```
- Git

2. **Clone the Repository**

    First, clone the repository using the following command:

    ```bash
    git clone https://github.com/wazuh/wazuh-agent.git
    ```

3. **Initialize Submodules**

    The project uses submodules, so you need to initialize and update them. Run the following commands:

    ```bash
    cd wazuh-agent
    git submodule update --init --recursive
    ```

4. **Configure and Build the Project**

    ```bash
    cmake src -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build
    ```

    If you want to include tests, configure the project with the following command:

    ```bash
    cmake src -B build -DBUILD_TESTS=1 -G "Visual Studio 17 2022" -A x64
    cmake --build build --config RelWithDebInfo
    ```

5. **Run the Agent**

    **Install windows service**

    ```bash
    .\RelWithDebInfo\wazuh-agent --install-service
    ```
    You can start, stop or restart the service from Windows SCM.

    Or from the CLI
    ```bash
    .\RelWithDebInfo\wazuh-agent
    .\RelWithDebInfo\wazuh-agent --status
    ```

    **To remove the service**
    ```bash
    .\RelWithDebInfo\wazuh-agent --remove-service
    ```

6. **Run tests**

    If built with CMake and `-DBUILD_TESTS=1`, you can run tests with:

    ```bash
    ctest -C RelWithDebInfo --test-dir build --output-log build
    ```

### Options

|Option|Description|Default|
|---|---|---|
|`BUILD_TESTS`|Enable tests compilation|`OFF`|
|`COVERAGE`|Enable coverage report|`OFF`|
|`ENABLE_CLANG_TIDY`|Check code with _clang-tidy_|`ON`|
|`ENABLE_INVENTORY`|Enable Inventory module |`ON`|
|`ENABLE_LOGCOLLECTOR`|Enable Logcollector module|`ON`|

## Notes

- The project uses `vcpkg` as a submodule to manage dependencies. By initializing the submodules, `vcpkg` will automatically fetch the necessary dependencies when running CMake.
