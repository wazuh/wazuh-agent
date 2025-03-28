# Build Packages

## Linux Packages

The script `generate_package.sh` automates the process of building Wazuh Agent packages for various architectures within a Docker container.

**Features:**

- Selectable architectures (amd64, arm64).
- Optional debug builds.
- Generates checksums for built packages.
- Uses local source code or downloads from GitHub.

**Requirements:**

- Docker installed and running.

**Usage:**
```
wazuh# cd packages
./generate_package.sh [OPTIONS]
```

**Options:**

| Option                     | Description                                                                                                | Default         |
|----------------------------|------------------------------------------------------------------------------------------------------------|-----------------|
| -b, --branch               | Select Git branch (optional)                                                                               | main            |
| -a, --architecture         | Target architecture of the package [amd64, arm64] (optional)                                               |                 |
| -j, --jobs                 | Change number of parallel jobs when compiling the manager or agent (optional)                              | 2               |
| -r, --revision             | Package revision (optional)                                                                                | 0               |
| -s, --store                | Set the destination path of package (optional). By default, an output folder will be created.              | Output folder   |
| -p, --path                 | Installation path for the package (optional)                                                               | /               |
| -d, --debug                | Build the binaries with debug symbols (optional)                                                           | no              |
| -c, --checksum             | Generate checksum on the same directory as the package (optional)                                          | no              |
| -e, --entrypoint           | Script to execute as entrypoint (optional)                                                                 |                 |
| --dont-build-docker        | Use locally built docker image instead of generating a new one (optional)                                  |                 |
| --vcpkg-binary-caching-key | VCPK remote binary caching repository key (optional)                                                       |                 |
| --tag                      | Tag to use with the docker image (optional)                                                                |                 |
|*--sources                  | Absolute path containing wazuh source code. Uses local source instead of downloading from GitHub (optional)| Script path     |
|**--is_stage                | Use release name in package (optional)                                                                     |                 |
| --system                   | Select Package OS [rpm, deb] (optional)                                                                    | deb             |
| --src                      | Generate the source package in the destination directory (optional)                                        |                 |
| --future                   | Build test future package x.30.0 for development purposes (optional)                                       |                 |
| --verbose                  | Print commands and their arguments as they are executed (optional)                                         |                 |
| -h, --help                 | Show this help                                                                                             |                 |

***Note 1:** If we don't use this flag and no `-b` parameter is given, the script will use the current directory where `generate_package.sh` is located.

****Note 2:** If the package is not a release package, a short hash commit based on the git command `git rev-parse --short HEAD` will be appended to the end of the name. The default length of the short hash is determined by the Git command [`git rev-parse --short[=length]`](https://git-scm.com/docs/git-rev-parse#Documentation/git-rev-parse.txt---shortlength:~:text=interpreted%20as%20usual.-,%2D%2Dshort%5B%3Dlength%5D,-Same%20as%20%2D%2Dverify).

****Note 3:** `--future` and `--src` options are not actively maintained and may be removed or modified.

**Example Usage:**

1. Build a DEB package for amd64 architecture:
```
./generate_package.sh -a amd64 -s /tmp --system deb
```

2. Build a RPM agent package for arm64 architecture with checksum generation:
```
./generate_package.sh -t agent -a arm64 -s /tmp -c --system rpm
```

**Notes:**
- For `--dont-build-docker` to work effectively, ensure a Docker image with the necessary build environment is already available.
- For RPM packages, we use the following architecture equivalences:
    * amd64 -> x86_64
    * arm64 -> aarch64


## Mac OS Packages

**Usage:**
```
wazuh# cd packages/macos
./generate_wazuh_packages.sh [OPTIONS]
```

**Options:**

| Option                     | Description                                                                                                | Default         |
|----------------------------|------------------------------------------------------------------------------------------------------------|-----------------|
| -a, --architecture         | Target architecture of the package [intel64, arm64] (optional)                                             | intel64         |
| -b, --branch               | Select Git branch (optional)                                                                               |                 |
| -j, --jobs                 | Number of parallel jobs when compiling (optional)                                                          | 2               |
| -r, --revision             | Package revision (optional)                                                                                | 0               |
| -s, --store-path           | Set the destination path of package (optional). By default, an output folder will be created               | Output folder   |
| -d, --debug                | Build the binaries with debug symbols (optional)                                                           | no              |
| -c, --checksum             | Generate checksum on the same directory as the package (optional)                                          | no              |
| --vcpkg-binary-caching-key | VCPK remote binary caching repository key (optional)                                                       |                 |
|--is_stage                  | Use release name in package (optional)                                                                     |                 |
| -v, --verbose              | Print commands and their arguments as they are executed (optional)                                         |                 |
| -h, --help                 | Show this help                                                                                             |                 |
| -i, --install-deps         | Install build dependencies                                                                                 |                 |
| -x, --install-xcode        | Install X-Code and brew. Can't be executed as root                                                         |                 |

**Signing options:**

| Option                     | Description                                                                                                | Default         |
|----------------------------|------------------------------------------------------------------------------------------------------------|-----------------|
| --keychain                 | Keychain where the Certificates are installed (optional)                                                   |                 |
| --keychain-password        | Password of the keychain (optional)                                                                        |                 |
| --application-certificate  | Apple Developer ID certificate name to sign Apps and binaries (optional)                                   |                 |
| --installer-certificate    | Apple Developer ID certificate name to sign pkg (optional)                                                 |                 |
| --notarize                 | Notarize the package for its distribution on macOS (optional)                                              | no              |
| --notarize-path            | Path of the package to be notarized (optional)                                                             |                 |
| --developer-id             | Your Apple Developer ID (optional)                                                                         |                 |
| --team-id                  | Your Apple Team ID (optional)                                                                              |                 |
| --altool-password          | Temporary password to use altool from Xcode (optional)                                                     |                 |


**Note 1:** When building the package, run script with `-i` argument and `-x` argument to fully cover the packaging dependencies.

**Note 2:** When no `-b` parameter is given, the current repository will be used to build the package, ensure the repo is not already compiled, otherwise, if will use the already compiled binaries to build the package.

**Example Usage:**

1. Build a package for arm64 architecture using 3 cores to compile and store the package in the `/tmp` folder, showing verbose output of the building procedure:
```
./macos/generate_wazuh_packages.sh -a arm64 -j 3 -r 0 -s /tmp --verbose 
```

## Windows Packages

**Usage:**
```
wazuh# cd packages/windows
$ ./generate_compiled_windows_agent.ps1 [COMPILATION-OPTIONS]
$ ./packages/windows/generate_wazuh_msi.ps1 [PACKAGE-OPTIONS]
```

**Compilation options:**

| Option                     | Description                                                                                                | Default         |
|----------------------------|------------------------------------------------------------------------------------------------------------|-----------------|
| -MSI_NAME                  | MSI package name output                                                                                    | wazuh-agent     |
| -BUILD_TESTS               | Define test mode action (0 or 1)                                                                           | 0               |
| -CMAKE_CONFIG              | Cmake config type, Debug, Release, RelWithDebInfo or MinSizeRel                                            | Debug           |
| -TOKEN_VCPKG               | VCPKG remote binary caching repository key. By default is empty, no binary caching funcionality will be used.                                               |                 |

**Packaging options:**

| Option                     | Description                                                                                                | Default         |
|----------------------------|------------------------------------------------------------------------------------------------------------|-----------------|
| -MSI_NAME                  | MSI package name output                                                                                    | wazuh-agent     |
| -SIGN                      | Define sign action process (yes or no)                                                                     | no              |
| -DEBUG                     | Define debug symbols generation process (yes or no)                                                        | no              |
| -CMAKE_CONFIG              | Cmake config type, Debug, Release, RelWithDebInfo or MinSizeRel                                            | Debug           |
| -SIGN_TOOLS_PATH           | Sign tools path. Not needed if PATH env var is correctly configured                                        |                 |
| -CV2PDB_PATH               | Debug symbols tools path. Not needed if PATH env var is correctly configured                               |                 |
| -CERTIFICATE_PATH          | Path to the .pfx certificate file. If not specified, signtool /a parameter will be used                    |                 |
| -CERTIFICATE_PASSWORD      | Password for the .pfx certificate file. If not specified, signtool /a parameter will be used               |                 |


**Example Usage:**
1. Build a package with `wazuh-agent_4.9.0-0_windows_0ceb378` name without signing it:

```
$ ./generate_wazuh_msi.ps1 -MSI_NAME wazuh-agent_4.9.0-0_windows_0ceb378 -SIGN no
```

# Workflows

Packages can also be built using GHA Workflows from the Github interface which let manually set every avaialble field as seen in the following links:

[Linux Packages WF](https://github.com/wazuh/wazuh-agent/actions/workflows/5_builderpackage_agent-linux.yml)

[Mac OS Packages WF](https://github.com/wazuh/wazuh-agent/actions/workflows/5_builderpackage_agent-macos.yml)

[Windows Packages WF](https://github.com/wazuh/wazuh-agent/actions/workflows/5_builderpackage_agent-win.yml)

This workflows can be run using the GitHub web interface, by clicking the `Run workflow` or requesting them to be run using API calls or using the GitHub CLI. Each available for option for all dispatchable workflow is descripted in the input field of the GitHub web interface.

Following are some examples of related dispatchable workflows.

## Generate and push builder images to GH (Linux packages)

```bash
curl -L -X POST -H "Accept: application/vnd.github+json" -H "Authorization: Bearer $GH_WORKFLOW_TOKEN" -H "X-GitHub-Api-Version: 2022-11-28" --data-binary "@$(pwd)/wazuh-agent-test-amd64-rpm.json" "https://api.github.com/repos/wazuh/wazuh-agent/actions/workflows/packages-upload-images.yml/dispatches"
```

Where the JSON looks like this:

```json
{
    "ref":"x.y.z",
    "inputs":
        {
         "tag":"auto",
         "architecture":"amd64",
         "system":"rpm",
         "revision":"test",
         "is_stage":"false",
         "legacy":"false"
        }
}
```

## Generate packages

```bash
curl -L -X POST -H "Accept: application/vnd.github+json" -H "Authorization: Bearer $GH_WORKFLOW_TOKEN" -H "X-GitHub-Api-Version: 2022-11-28" --data-binary "@$(pwd)/wazuh-agent-test-amd64-rpm.json" "https://api.github.com/repos/wazuh/wazuh-agent/actions/workflows/packages-build-linux-agent.yml/dispatches"
```

Where the JSON looks like this:
```json
{
    "ref":"x.y.z",
    "inputs":
        {
         "docker_image_tag":"auto",
         "architecture":"amd64",
         "system":"deb",
         "revision":"test",
         "is_stage":"false",
         "legacy":"false",
         "checksum":"false",
     }
}
```

## Run workflows using GitHub CLI

Workflows can be executed directly from the command line using the `gh` command instead of `curl`. For example, while in the repository directory, the workflow can be triggered with:

```bash
gh workflow run packages-build-linux-agent-amd.yml -r enhancement/484-linux-rpmdeb-arm-package-creation -f architecture=arm64 -f source_reference=enhancement/484-linux-rpmdeb-arm-package-creation -f revision=3 -f is_stage=false -f system=rpm -f id=test_arm64_deps -f upload_to=artifact
```
