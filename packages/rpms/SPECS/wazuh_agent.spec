%if "%{_debugenabled}" == "yes"
  %global _enable_debug_package 0
  %global debug_package %{nil}
  %global __os_install_post %{nil}
  %define __strip /bin/true
%endif

%if "%{_isstage}" == "no"
  %define _rpmfilename %%{NAME}_%%{VERSION}-%%{RELEASE}_%%{ARCH}_%{_hashcommit}.rpm
%else
  %define _rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm
%endif

Summary:     Wazuh helps you to gain security visibility into your infrastructure by monitoring hosts at an operating system and application level. It provides the following capabilities: log analysis, file integrity monitoring, intrusions detection and policy and compliance monitoring
Name:        wazuh-agent
Version:     %{_version}
Release:     %{_release}
License:     GPL
Group:       System Environment/Daemons
Source0:     %{name}-%{version}.tar.gz
URL:         https://www.wazuh.com/
BuildRoot:   %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Vendor:      Wazuh, Inc <info@wazuh.com>
Packager:    Wazuh, Inc <info@wazuh.com>
Requires(pre):    /usr/sbin/groupadd /usr/sbin/useradd
Requires(postun): /usr/sbin/groupdel /usr/sbin/userdel
AutoReqProv: no

ExclusiveOS: linux

%description
Wazuh helps you to gain security visibility into your infrastructure by monitoring
hosts at an operating system and application level. It provides the following capabilities:
log analysis, file integrity monitoring, intrusions detection and policy and compliance monitoring

%prep
%setup -q

%build
if [ ! -d "$(pwd)/src/build" ]; then 
  cmake src -B $(pwd)/src/build 
  cmake --build $(pwd)/src/build --parallel %{_threads}
fi

%install
# Clean BUILDROOT
rm -fr %{buildroot}
cmake --install "$(pwd)/src/build" --prefix %{buildroot}%{_localstatedir}
install -D /usr/lib/systemd/system/wazuh-agent.service %{buildroot}%{_localstatedir}usr/lib/systemd/system/wazuh-agent.service
sed -i "s|%{buildroot}%{_localstatedir}|/|g" %{buildroot}%{_localstatedir}usr/lib/systemd/system/wazuh-agent.service
mkdir -p %{buildroot}/usr/bin
ln -sf %{_localstatedir}usr/share/wazuh-agent/bin/wazuh-agent %{buildroot}/usr/bin/wazuh-agent
exit 0

%pre
# Create the wazuh group if it doesn't exists
if command -v getent > /dev/null 2>&1 && ! getent group wazuh > /dev/null 2>&1; then
  groupadd -r wazuh
elif ! getent group wazuh > /dev/null 2>&1; then
  groupadd -r wazuh
fi

# Create the wazuh user if it doesn't exists
if ! getent passwd wazuh > /dev/null 2>&1; then
  useradd -g wazuh -G wazuh -d %{_localstatedir} -r -s /sbin/nologin wazuh
fi

## STOP AGENT HERE IF IT EXIST
if command -v systemctl > /dev/null 2>&1 && systemctl > /dev/null 2>&1 && systemctl is-active --quiet wazuh-agent > /dev/null 2>&1; then
  systemctl stop wazuh-agent > /dev/null 2>&1
# Check for SysV
elif command -v service > /dev/null 2>&1 && service wazuh-agent status 2>/dev/null | grep "is running" > /dev/null 2>&1; then
  service wazuh-agent stop > /dev/null 2>&1
elif /usr/share/wazuh-agent/bin/wazuh-agent --status 2>/dev/null | grep "is running" > /dev/null 2>&1; then
  pid=$(ps -ef | grep "${BINARY_DIR}wazuh-agent" | grep -v grep | awk '{print $2}')
  if [ -n "$pid" ]; then
    kill -SIGTERM "$pid" 2>/dev/null
  fi
fi

%post
# If the package is being upgraded

# If the package is being installed
if [ $1 = 1 ]; then
  if command -v systemctl > /dev/null 2>&1 && systemctl > /dev/null 2>&1 && systemctl is-active --quiet wazuh-agent > /dev/null 2>&1; then
    systemctl daemon-reload
    systemctl enable wazuh-agent
  fi
fi

## SCA RELATED

#AlmaLinux
if [ -r "/etc/almalinux-release" ]; then
  DIST_NAME=almalinux
  DIST_VER=`sed -rn 's/.* ([0-9]{1,2})\.*[0-9]{0,2}.*/\1/p' /etc/almalinux-release`
#Rocky
elif [ -r "/etc/rocky-release" ]; then
  DIST_NAME=rocky
  DIST_VER=`sed -rn 's/.* ([0-9]{1,2})\.*[0-9]{0,2}.*/\1/p' /etc/rocky-release`
# CentOS
elif [ -r "/etc/centos-release" ]; then
  if grep -q "AlmaLinux" /etc/centos-release; then
    DIST_NAME=almalinux
  elif grep -q "Rocky" /etc/centos-release; then
    DIST_NAME=almalinux
  else
    DIST_NAME="centos"
  fi
  DIST_VER=`sed -rn 's/.* ([0-9]{1,2})\.*[0-9]{0,2}.*/\1/p' /etc/centos-release`
# Fedora
elif [ -r "/etc/fedora-release" ]; then
    DIST_NAME="fedora"
    DIST_VER=`sed -rn 's/.* ([0-9]{1,2})\.*[0-9]{0,2}.*/\1/p' /etc/fedora-release`
# Oracle Linux
elif [ -r "/etc/oracle-release" ]; then
    DIST_NAME="ol"
    DIST_VER=`sed -rn 's/.* ([0-9]{1,2})\.*[0-9]{0,2}.*/\1/p' /etc/oracle-release`
# RedHat
elif [ -r "/etc/redhat-release" ]; then
  if grep -q "AlmaLinux" /etc/redhat-release; then
    DIST_NAME=almalinux
  elif grep -q "Rocky" /etc/redhat-release; then
    DIST_NAME=almalinux
  elif grep -q "CentOS" /etc/redhat-release; then
      DIST_NAME="centos"
  else
      DIST_NAME="rhel"
  fi
  DIST_VER=`sed -rn 's/.* ([0-9]{1,2})\.*[0-9]{0,2}.*/\1/p' /etc/redhat-release`
# SUSE
elif [ -r "/etc/SuSE-release" ]; then
  if grep -q "openSUSE" /etc/SuSE-release; then
      DIST_NAME="generic"
      DIST_VER=""
  else
      DIST_NAME="sles"
      DIST_VER=`sed -rn 's/.*VERSION = ([0-9]{1,2}).*/\1/p' /etc/SuSE-release`
  fi
elif [ -r "/etc/os-release" ]; then
  . /etc/os-release
  DIST_NAME=$ID
  DIST_VER=$(echo $VERSION_ID | sed -rn 's/[^0-9]*([0-9]+).*/\1/p')
  if [ "X$DIST_VER" = "X" ]; then
      DIST_VER="0"
  fi
  if [ "$DIST_NAME" = "amzn" ] && [ "$DIST_VER" != "2" ] && [ "$DIST_VER" != "2023" ]; then
      DIST_VER="1"
  fi
  DIST_SUBVER=$(echo $VERSION_ID | sed -rn 's/[^0-9]*[0-9]+\.([0-9]+).*/\1/p')
  if [ "X$DIST_SUBVER" = "X" ]; then
      DIST_SUBVER="0"
  fi
else
  DIST_NAME="generic"
  DIST_VER=""
fi

SCA_DIR="${DIST_NAME}/${DIST_VER}"
SCA_BASE_DIR="%{_localstatedir}/tmp/sca-%{version}-%{release}-tmp"
mkdir -p %{_localstatedir}/ruleset/sca


SCA_TMP_DIR="${SCA_BASE_DIR}/${SCA_DIR}"
# Install the configuration files needed for this hosts
if [ -r "${SCA_BASE_DIR}/${DIST_NAME}/${DIST_VER}/${DIST_SUBVER}/sca.files" ]; then
  SCA_TMP_DIR="${SCA_BASE_DIR}/${DIST_NAME}/${DIST_VER}/${DIST_SUBVER}"
elif [ -r "${SCA_BASE_DIR}/${DIST_NAME}/${DIST_VER}/sca.files" ]; then
  SCA_TMP_DIR="${SCA_BASE_DIR}/${DIST_NAME}/${DIST_VER}"
elif [ -r "${SCA_BASE_DIR}/${DIST_NAME}/sca.files" ]; then
  SCA_TMP_DIR="${SCA_BASE_DIR}/${DIST_NAME}"
else
  SCA_TMP_DIR="${SCA_BASE_DIR}/generic"
fi

SCA_TMP_FILE="${SCA_TMP_DIR}/sca.files"
if [ -r ${SCA_TMP_FILE} ]; then

  rm -f %{_localstatedir}/ruleset/sca/* || true

  for sca_file in $(cat ${SCA_TMP_FILE}); do
    if [ -f ${SCA_BASE_DIR}/${sca_file} ]; then
      mv ${SCA_BASE_DIR}/${sca_file} %{_localstatedir}/ruleset/sca
    fi
  done
fi

%preun
if [ $1 = 0 ]; then
  # Stop the services before uninstall the package
  # Check for systemd
  if command -v systemctl > /dev/null 2>&1 && systemctl > /dev/null 2>&1 && systemctl is-active --quiet wazuh-agent > /dev/null 2>&1; then
    systemctl stop wazuh-agent > /dev/null 2>&1
  # Check for SysV
  elif command -v service > /dev/null 2>&1 && service wazuh-agent status 2>/dev/null | grep "is running" > /dev/null 2>&1; then
    service wazuh-agent stop > /dev/null 2>&1
  elif /usr/share/wazuh-agent/bin/wazuh-agent --status 2>/dev/null | grep "is running" > /dev/null 2>&1; then
    pid=$(ps -ef | grep "${BINARY_DIR}wazuh-agent" | grep -v grep | awk '{print $2}')
    if [ -n "$pid" ]; then
      kill -SIGTERM "$pid" 2>/dev/null
    fi
  fi

  # Remove the SELinux policy
  if command -v getenforce > /dev/null 2>&1 && command -v semodule > /dev/null 2>&1; then
    if [ $(getenforce) != "Disabled" ]; then
      if (semodule -l | grep wazuh > /dev/null); then
        semodule -r wazuh > /dev/null
      fi
    fi
  fi
  # Remove the service file for SUSE hosts
  if [ -f /etc/os-release ]; then
    sles=$(grep "\"sles" /etc/os-release)
  elif [ -f /etc/SuSE-release ]; then
    sles=$(grep "SUSE Linux Enterprise Server" /etc/SuSE-release)
  fi
  if [ ! -z "$sles" ]; then
    rm -f /etc/init.d/wazuh-agent
  fi

  # Remove SCA files
  rm -f %{_localstatedir}/ruleset/sca/*
fi

%postun
# If the package is being uninstalled or we want to delete wazuh user and group
if [ $1 = 0 ]; then
  # Remove the wazuh user if it exists
  if getent passwd wazuh > /dev/null 2>&1; then
    userdel wazuh >/dev/null 2>&1
  fi
  # Remove the wazuh group if it exists
  if command -v getent > /dev/null 2>&1 && getent group wazuh > /dev/null 2>&1; then
    groupdel wazuh >/dev/null 2>&1
  elif getent group wazuh > /dev/null 2>&1; then
    groupdel wazuh >/dev/null 2>&1
  fi

  if [ $1 = 0 ];then
    # Remove lingering folders and files
    rm -f %{_localstatedir}usr/bin/wazuh-agent
    rm -rf %{_localstatedir}usr/share/wazuh-agent/bin/wazuh-agent
    rm -f %{_localstatedir}usr/lib/systemd/system/wazuh-agent.service
    rm -rf %{_localstatedir}etc/wazuh-agent
    rm -rf %{_localstatedir}var/lib/wazuh-agent
  fi
fi

# posttrans code is the last thing executed in a install/upgrade
%posttrans
systemctl daemon-reload > /dev/null 2>&1

%clean
rm -fr %{buildroot}

%files
%defattr(-,root,root)
%attr(750, root, wazuh) %{_localstatedir}usr/share/wazuh-agent/bin/wazuh-agent
%attr(750, root, wazuh) %{_localstatedir}usr/lib/systemd/system/wazuh-agent.service
%dir %attr(770, root, wazuh) %{_localstatedir}etc/wazuh-agent
%dir %attr(750, root, wazuh) %{_localstatedir}var/lib/wazuh-agent
%attr(750, root, wazuh) %{_localstatedir}etc/wazuh-agent/wazuh-agent.yml
%attr(440, wazuh, wazuh) %{_localstatedir}usr/share/wazuh-agent/VERSION.json
%attr(755, root, root) %{_localstatedir}usr/bin/wazuh-agent

%changelog
* Wed Jan 01 2025 Wazuh Inc <info@wazuh.com> - 5.0.0
- More info: https://github.com/wazuh/wazuh-agent/blob/main/README.md
