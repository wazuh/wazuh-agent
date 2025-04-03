# Set up the Development Environment

## Dependencies

The following dependencies are required for building this project:

- **Git**
- **CMake** at least 3.22
- **C++ compiler** (GCC 10 or Clang 13, coroutines support needed)
- **Vcpkg**
    - **Zip** (for [vcpkg](https://vcpkg.io))
    - **Unzip** (for [vcpkg](https://vcpkg.io))
    - **Curl** (for [vcpkg](https://vcpkg.io))
    - **Tar** (for [vcpkg](https://vcpkg.io))
    - **Ninja-build** (for [vcpkg](https://vcpkg.io))
- Plus some additional packages listed below.

## **Installing Dependencies on Debian**

To install the necessary dependencies on a Debian-based system, run the following commands:

```bash
sudo apt-get update
sudo apt-get install cmake make g++ gcc git zip unzip curl tar ninja-build pkg-config wget lsb-release libsystemd-dev autopoint autoconf libtool gettext
```

## **Installing Dependencies on macOS**

To install the necessary dependencies on macOS, run the following commands:

1. **Install brew**
    ```bash
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    ```

2. **Install dependencies**
    ```bash
    brew install automake autoconf libtool cmake pkg-config openssl
    ```

## **Installing Dependencies on Windows**

To install the necessary dependencies on Windows, run the following commands:

1. **Visual Studio Community 2022 (with MSVC 14)**

2. **Install chocolatey**
    ```bash
    Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    ```

3. **Install dependencies**
    ```bash
    choco install make
    choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
    choco install openssl
    ```

## Set up the Toolchain

In addition to the requirements listed above, our development process mandates the use of
`clang-format`, `clang-tidy`, and `cmake-format`, with checks integrated into our CI pipelines.

On Linux, these tools can be installed with the following commands:

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18
sudo apt-get update
sudo apt-get install -y clang-tidy-18 clang-format-18
sudo apt-get install -y cmake-format
```

Additionally, it is possible to check out Vcpkg to a separate path and use that instance for builds:

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT=$(pwd)
```

## Set up Editor / Debugger

Any editor and debugger can be used, except on Windows, where it is listed as a dependency because it is required to build the solution correctly.
