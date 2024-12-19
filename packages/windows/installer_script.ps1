# Script for configuration Windows agent.
# Copyright (C) 2015, Wazuh Inc. <support@wazuh.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
#
# ------------------------------------------------

function Get-UniqueArrayValues {
    param (
        [array]$Array
    )

    $hashSet = @{}
    foreach ($item in $Array) {
        $hashSet[$item] = $null
    }

    return $hashSet.Keys
}

function Config {
    param (
        [string]$CustomActionData
    )

    $ForReading = 1
    $ForWriting = 2

    # Parse custom parameters
    $args = $CustomActionData -split "/\+/"
    $homeDir = $args[0].Trim('"')
    $OS_VERSION = $args[1].Trim('"')
    $WAZUH_MANAGER = $args[2].Trim('"')
    $WAZUH_MANAGER_PORT = $args[3].Trim('"')
    $WAZUH_PROTOCOL = $args[4].Trim('"')
    $NOTIFY_TIME = $args[5].Trim('"')
    $WAZUH_REGISTRATION_SERVER = $args[6].Trim('"')
    $WAZUH_REGISTRATION_PORT = $args[7].Trim('"')
    $WAZUH_REGISTRATION_PASSWORD = $args[8].Trim('"')
    $WAZUH_KEEP_ALIVE_INTERVAL = $args[9].Trim('"')
    $WAZUH_TIME_RECONNECT = $args[10].Trim('"')
    $WAZUH_REGISTRATION_CA = $args[11].Trim('"')
    $WAZUH_REGISTRATION_CERTIFICATE = $args[12].Trim('"')
    $WAZUH_REGISTRATION_KEY = $args[13].Trim('"')
    $WAZUH_AGENT_NAME = $args[14].Trim('"')
    $WAZUH_AGENT_GROUP = $args[15].Trim('"')
    $ENROLLMENT_DELAY = $args[16].Trim('"')

    # Create client.keys if it doesn't exist
    if (!(Test-Path -Path "$homeDir\client.keys")) {
        New-Item -ItemType File -Path "$homeDir\client.keys" | Out-Null
    }

    if (Test-Path -Path "$homeDir\ossec.conf") {
        # Read the ossec.conf file
        $strText = Get-Content -Path "$homeDir\ossec.conf" -Raw

        # Update ossec.conf based on custom parameters
        if ($WAZUH_MANAGER -or $WAZUH_MANAGER_PORT -or $WAZUH_PROTOCOL -or $WAZUH_KEEP_ALIVE_INTERVAL -or $WAZUH_TIME_RECONNECT) {
            $protocolList = if ($WAZUH_PROTOCOL -and $WAZUH_PROTOCOL -match ",") {
                $WAZUH_PROTOCOL.ToLower().Split(",")
            } else {
                @($WAZUH_PROTOCOL.ToLower())
            }

            if ($WAZUH_MANAGER) {
                # Update server configuration
                $ipList = if ($WAZUH_MANAGER -match ",") {
                    $WAZUH_MANAGER.Split(",")
                } else {
                    @($WAZUH_MANAGER)
                }

                $formattedList = "`n"
                for ($i = 0; $i -lt $ipList.Count; $i++) {
                    if ($ipList[$i]) {
                        $protocol = if ($i -lt $protocolList.Count -and $protocolList[$i]) {
                            $protocolList[$i].ToLower()
                        } else {
                            "tcp"
                        }
                        $formattedList += "    <server>`n"
                        $formattedList += "      <address>$($ipList[$i])</address>`n"
                        $formattedList += "      <port>1514</port>`n"
                        $formattedList += "      <protocol>$protocol</protocol>`n"
                        $formattedList += if ($i -eq $ipList.Count - 1) {
                            "    </server>"
                        } else {
                            "    </server>`n"
                        }
                    }
                }
                $strText = $strText -replace "\s+<server>(.|\n)+?</server>", $formattedList
            }

            if ($WAZUH_MANAGER_PORT) {
                $strText = $strText -replace "<port>1514</port>", "<port>$WAZUH_MANAGER_PORT</port>"
            }

            if ($WAZUH_KEEP_ALIVE_INTERVAL) {
                $strText = $strText -replace "<notify_time>.*</notify_time>", "<notify_time>$WAZUH_KEEP_ALIVE_INTERVAL</notify_time>"
            }

            if ($WAZUH_TIME_RECONNECT) {
                $strText = $strText -replace "<time-reconnect>.*</time-reconnect>", "<time-reconnect>$WAZUH_TIME_RECONNECT</time-reconnect>"
            }
        }

        # Handle enrollment configuration
        if ($WAZUH_REGISTRATION_SERVER -or $WAZUH_REGISTRATION_PORT -or $WAZUH_REGISTRATION_PASSWORD -or $WAZUH_REGISTRATION_CA -or $WAZUH_REGISTRATION_CERTIFICATE -or $WAZUH_REGISTRATION_KEY -or $WAZUH_AGENT_NAME -or $WAZUH_AGENT_GROUP -or $ENROLLMENT_DELAY) {
            $enrollmentList = @"
    <enrollment>
      <enabled>yes</enabled>
    </enrollment>
  </client>
"@
            $strText = $strText.Replace("  </client>", $enrollmentList)

            if ($WAZUH_REGISTRATION_SERVER) {
                $strText = $strText.Replace("    </enrollment>", "      <manager_address>$WAZUH_REGISTRATION_SERVER</manager_address>`n    </enrollment>")
            }

            if ($WAZUH_REGISTRATION_PORT) {
                $strText = $strText.Replace("    </enrollment>", "      <port>$WAZUH_REGISTRATION_PORT</port>`n    </enrollment>")
            }

            if ($WAZUH_REGISTRATION_PASSWORD) {
                Set-Content -Path "$homeDir\authd.pass" -Value $WAZUH_REGISTRATION_PASSWORD
                $strText = $strText.Replace("    </enrollment>", "      <authorization_pass_path>authd.pass</authorization_pass_path>`n    </enrollment>")
            }

            if ($WAZUH_REGISTRATION_CA) {
                $strText = $strText.Replace("    </enrollment>", "      <server_ca_path>$WAZUH_REGISTRATION_CA</server_ca_path>`n    </enrollment>")
            }

            if ($WAZUH_REGISTRATION_CERTIFICATE) {
                $strText = $strText.Replace("    </enrollment>", "      <agent_certificate_path>$WAZUH_REGISTRATION_CERTIFICATE</agent_certificate_path>`n    </enrollment>")
            }

            if ($WAZUH_REGISTRATION_KEY) {
                $strText = $strText.Replace("    </enrollment>", "      <agent_key_path>$WAZUH_REGISTRATION_KEY</agent_key_path>`n    </enrollment>")
            }

            if ($WAZUH_AGENT_NAME) {
                $strText = $strText.Replace("    </enrollment>", "      <agent_name>$WAZUH_AGENT_NAME</agent_name>`n    </enrollment>")
            }

            if ($WAZUH_AGENT_GROUP) {
                $strText = $strText.Replace("    </enrollment>", "      <groups>$WAZUH_AGENT_GROUP</groups>`n    </enrollment>")
            }

            if ($ENROLLMENT_DELAY) {
                $strText = $strText.Replace("    </enrollment>", "      <delay_after_enrollment>$ENROLLMENT_DELAY</delay_after_enrollment>`n    </enrollment>")
            }
        }

        # Write the updated ossec.conf
        Set-Content -Path "$homeDir\ossec.conf" -Value $strText
    }

    Set-WazuhPermissions

    return 0
}

function Get-Version {
    $osVersion = (Get-CimInstance -ClassName Win32_OperatingSystem).Version
    return $osVersion.Split('.')[0]
}

function Check-SvcRunning {
    $serviceNames = @("OssecSvc", "WazuhSvc")
    foreach ($serviceName in $serviceNames) {
        $service = Get-CimInstance -ClassName Win32_Service | Where-Object { $_.Name -eq $serviceName }
        if ($service) {
            $state = $service.State
            if ($serviceName -eq "OssecSvc") {
                $env:OSSECRUNNING = $state
            } elseif ($serviceName -eq "WazuhSvc") {
                $env:WAZUHRUNNING = $state
            }
        }
    }
    return 0
}

function Kill-GUITask {
    Stop-Process -Name "win32ui" -Force -ErrorAction SilentlyContinue
    return 0
}

function Start-WazuhSvc {
    Start-Service -Name "WazuhSvc"
    return 0
}

function Set-WazuhPermissions {
    param (
        [string]$CustomActionData
    )

    $args = $CustomActionData -split "/\+/"
    $homeDir = $args[0].Trim('"')

    if ([int](Get-Version) -ge 6) {
        $installDir = $homeDir.TrimEnd('\')

        icacls "$installDir" /reset /t
        icacls "$installDir" /inheritancelevel:r /q
        icacls "$installDir" /grant *S-1-5-32-544:(OI)(CI)F
        icacls "$installDir" /grant *S-1-5-18:(OI)(CI)F
        icacls "$installDir\*" /grant *S-1-5-11:(OI)(CI)RX
        icacls "$installDir\*" /grant *S-1-5-11:RX
        icacls "$installDir" /grant *S-1-5-11:RX

        icacls "$homeDir*ossec.conf" /remove *S-1-5-11 /q
        icacls "$homeDir\client.keys" /remove *S-1-5-11 /q
        icacls "$homeDir\tmp" /remove:g *S-1-5-11 /q
    }
    return 0
}

function Create-DumpRegistryKey {
    $regPath = "HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\wazuh-agent.exe"
    if (-not (Test-Path -Path $regPath)) {
        New-Item -Path $regPath -Force | Out-Null
    }

    Set-ItemProperty -Path $regPath -Name "DumpFolder" -Value "%LOCALAPPDATA%\WazuhCrashDumps"
    Set-ItemProperty -Path $regPath -Name "DumpType" -Value 2
    return 0
}
