# Build Packages

The script `generate_package.sh` automates the process of building Wazuh Agent packages for various architectures within a Docker container.

**Features:**

- Selectable architectures (amd64, x86_64, arm64, aarch64).
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
| -a, --architecture         | Target architecture of the package [amd64, x86_64, arm64, aarch64] (optional)                                               |                 |
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

***Note 1:** If we don't use this flag, the script will use the current directory where `generate_package.sh` is located.

****Note 2:** If the package is not a release package, a short hash commit based on the git command `git rev-parse --short HEAD` will be appended to the end of the name. The default length of the short hash is determined by the Git command [`git rev-parse --short[=length]`](https://git-scm.com/docs/git-rev-parse#Documentation/git-rev-parse.txt---shortlength:~:text=interpreted%20as%20usual.-,%2D%2Dshort%5B%3Dlength%5D,-Same%20as%20%2D%2Dverify).

**Example Usage:**

1. Build a manager package for amd64 architecture:
```
./generate_package.sh -a amd64 -s /tmp --system rpm
```

2. Build a debug agent package for arm64 architecture with checksum generation:
```
./generate_package.sh -t agent -a arm64 -s /tmp -d -c --system rpm
```

3. Build a package using local Wazuh source code:
```
./generate_package.sh -a amd64 --sources /path/to/wazuh/source --system rpm
```

**Notes:**
- For `--dont-build-docker` to work effectively, ensure a Docker image with the necessary build environment is already available.
- For RPM packages, we use the following architecture equivalences:
    * amd64 -> x86_64
    * arm64 -> aarch64

# Workflow

## Generate and push builder images to GH

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
