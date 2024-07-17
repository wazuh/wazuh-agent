# Build Instructions

1. **Clone the Repository**

    First, clone the repository using the following command:

    ```bash
    git clone https://github.com/wazuh/wazuh-agent.git
    ```

2. **Initialize Submodules**

    The project uses submodules, so you need to initialize and update them. Run the following commands:

    ```bash
    cd wazuh-agent
    git submodule update --init --recursive
    ```

3. **Build the Project**

    Navigate to the `src` folder and create a `build` directory:

    ```bash
    cd src
    mkdir build
    cd build
    ```

    Run `cmake` to configure the project:

    ```bash
    cmake ..
    ```

    If you want to include tests, configure the project with the following command:

    ```bash
    cmake .. -DBUILD_TESTS=1
    ```

4. **Build the Project**

    Finally, build the project using `make`:

    ```bash
    make
    ```

## Notes

- The project uses `vcpkg` as a submodule to manage dependencies. By initializing the submodules, `vcpkg` will automatically fetch the necessary dependencies when running CMake.
- Ensure you have `cmake` and `make` installed on your system. You can install them via your package manager.
