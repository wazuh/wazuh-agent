# Run Tests

## Compilation steps for Linux and macOS

1. **Configure and Build the Project**

    ```bash
    cd wazuh-agent
    cmake src -B build -DBUILD_TESTS=1
    cmake --build build
    ```

2. **Run tests**

    ```bash
    ctest --test-dir build --output-log build
    ```

## Compilation steps for Windows

1. **Configure and Build the Project**

    ```bash
    cd wazuh-agent
    cmake src -B build -DBUILD_TESTS=1 -G "Visual Studio 17 2022" -A x64
    cmake --build build --config RelWithDebInfo
    ```

2. **Run tests**

    ```bash
    ctest -C RelWithDebInfo --test-dir build --output-log build
    ```
