.$(C:\vcpkg\vcpkg fetch nuget) `
    sources add `
    -Source "https://nuget.pkg.github.com/wazuh/index.json" `
    -Name GitHub `
    -UserName "wazuh" `
    -Password "$Env:GITHUB_TOKEN"
.$(C:\vcpkg\vcpkg fetch nuget) `
    setapikey "$Env:GITHUB_TOKEN" `
    -Source "https://nuget.pkg.github.com/wazuh/index.json"
