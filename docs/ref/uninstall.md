# Uninstall

## Uninstall the Agent on Linux

- DEB

    ```bash
    sudo dpkg -r wazuh-agent
    ```

- RPM

    ```bash
    sudo rpm -e wazuh-agent
    ```

## Uninstall the Agent on macOS

- PKG
    <!-- To do -->

## Uninstall the Agent on Windows

- MSI
    ```powershell
    msiexec /x '.\wazuh-agent_x.y.z.msi' /l*v uninstall.log
    ```
**Note:** The package can also be uninstalled on Windows through the OS's "Add or Remove Programs" section.
