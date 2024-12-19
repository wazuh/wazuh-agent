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


# This function is called only when uninstalling the product.
# Remove everything, but a few specified items.

function Remove-All {
    param (
        [string]$CustomActionData
    )

    # Retrieve the parameters
    $args = $CustomActionData -split ","
    $homeDir = $args[0].Trim('"') # APPLICATIONFOLDER

    # Check and delete specific files
    $filesToDelete = @(
        "ossec.conf.save", "client.keys.save",
        "local_internal_options.conf.save", "installer.log.save"
    )
    $filesToRename = @(
        "ossec.conf", "client.keys",
        "local_internal_options.conf", "installer.log"
    )

    foreach ($file in $filesToDelete) {
        $filePath = Join-Path $homeDir $file
        if (Test-Path $filePath) {
            Remove-Item -Path $filePath -Force
        }
    }

    foreach ($file in $filesToRename) {
        $filePath = Join-Path $homeDir $file
        $newFilePath = Join-Path $homeDir "$file.save"
        if (Test-Path $filePath) {
            Rename-Item -Path $filePath -NewName "$file.save" -Force
        }
    }

    # Delete all other files and folders except specified
    if (Test-Path $homeDir) {
        $filesToKeep = @(
            "ossec.conf.save", "client.keys.save",
            "local_internal_options.conf.save", "installer.log.save"
        )
        $subfoldersToKeep = @("backup", "upgrade")

        # Delete files
        Get-ChildItem -Path $homeDir -File | ForEach-Object {
            if ($filesToKeep -notcontains $_.Name) {
                Remove-Item -Path $_.FullName -Force -ErrorAction SilentlyContinue
            }
        }

        # Delete subfolders
        Get-ChildItem -Path $homeDir -Directory | ForEach-Object {
            if ($subfoldersToKeep -notcontains $_.Name) {
                Remove-Item -Path $_.FullName -Recurse -Force -ErrorAction SilentlyContinue
            }
        }
    }

    return 0
}
