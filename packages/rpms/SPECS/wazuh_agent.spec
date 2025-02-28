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
%attr(440, wazuh, wazuh) %{_localstatedir}etc/wazuh-agent/VERSION.json

%changelog
* Wed Jul 10 2024 support <info@wazuh.com> - 4.9.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-9-0.html
* Wed Jun 26 2024 support <info@wazuh.com> - 4.8.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-8-1.html
* Wed Jun 12 2024 support <info@wazuh.com> - 4.8.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-8-0.html
* Thu May 30 2024 support <info@wazuh.com> - 4.7.5
- More info: https://documentation.wazuh.com/current/release-notes/release-4-7-5.html
* Thu Apr 25 2024 support <info@wazuh.com> - 4.7.4
- More info: https://documentation.wazuh.com/current/release-notes/release-4-7-4.html
* Tue Feb 27 2024 support <info@wazuh.com> - 4.7.3
- More info: https://documentation.wazuh.com/current/release-notes/release-4-7-3.html
* Tue Jan 09 2024 support <info@wazuh.com> - 4.7.2
- More info: https://documentation.wazuh.com/current/release-notes/release-4-7-2.html
* Wed Dec 13 2023 support <info@wazuh.com> - 4.7.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-7-1.html
* Tue Nov 21 2023 support <info@wazuh.com> - 4.7.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-7-0.html
* Tue Oct 31 2023 support <info@wazuh.com> - 4.6.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-6-0.html
* Tue Oct 24 2023 support <info@wazuh.com> - 4.5.4
- More info: https://documentation.wazuh.com/current/release-notes/release-4-5-4.html
* Tue Oct 10 2023 support <info@wazuh.com> - 4.5.3
- More info: https://documentation.wazuh.com/current/release-notes/release-4-5-3.html
* Thu Aug 31 2023 support <info@wazuh.com> - 4.5.2
- More info: https://documentation.wazuh.com/current/release-notes/release-4-5-2.html
* Thu Aug 24 2023 support <info@wazuh.com> - 4.5.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-5.1.html
* Thu Aug 10 2023 support <info@wazuh.com> - 4.5.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-5-0.html
* Mon Jul 10 2023 support <info@wazuh.com> - 4.4.5
- More info: https://documentation.wazuh.com/current/release-notes/release-4-4-5.html
* Tue Jun 13 2023 support <info@wazuh.com> - 4.4.4
- More info: https://documentation.wazuh.com/current/release-notes/release-4-4-4.html
* Thu May 25 2023 support <info@wazuh.com> - 4.4.3
- More info: https://documentation.wazuh.com/current/release-notes/release-4-4-3.html
* Mon May 08 2023 support <info@wazuh.com> - 4.4.2
- More info: https://documentation.wazuh.com/current/release-notes/release-4-4-2.html
* Mon Apr 24 2023 support <info@wazuh.com> - 4.3.11
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3.11.html
* Mon Apr 17 2023 support <info@wazuh.com> - 4.4.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-4-1.html
* Wed Jan 18 2023 support <info@wazuh.com> - 4.4.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-4-0.html
* Thu Nov 10 2022 support <info@wazuh.com> - 4.3.10
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-10.html
* Mon Oct 03 2022 support <info@wazuh.com> - 4.3.9
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-9.html
* Wed Sep 21 2022 support <info@wazuh.com> - 3.13.6
- More info: https://documentation.wazuh.com/current/release-notes/release-3-13-6.html
* Mon Sep 19 2022 support <info@wazuh.com> - 4.3.8
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-8.html
* Wed Aug 24 2022 support <info@wazuh.com> - 3.13.5
- More info: https://documentation.wazuh.com/current/release-notes/release-3-13-5.html
* Mon Aug 08 2022 support <info@wazuh.com> - 4.3.7
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-7.html
* Thu Jul 07 2022 support <info@wazuh.com> - 4.3.6
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-6.html
* Wed Jun 29 2022 support <info@wazuh.com> - 4.3.5
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-5.html
* Tue Jun 07 2022 support <info@wazuh.com> - 4.3.4
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-4.html
* Tue May 31 2022 support <info@wazuh.com> - 4.3.3
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-3.html
* Mon May 30 2022 support <info@wazuh.com> - 4.3.2
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-2.html
* Mon May 30 2022 support <info@wazuh.com> - 3.13.4
- More info: https://documentation.wazuh.com/current/release-notes/release-3-13-4.html
* Sun May 29 2022 support <info@wazuh.com> - 4.2.7
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-7.html
* Wed May 18 2022 support <info@wazuh.com> - 4.3.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-1.html
* Thu May 05 2022 support <info@wazuh.com> - 4.3.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-3-0.html
* Fri Mar 25 2022 support <info@wazuh.com> - 4.2.6
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-6.html
* Mon Nov 15 2021 support <info@wazuh.com> - 4.2.5
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-5.html
* Thu Oct 21 2021 support <info@wazuh.com> - 4.2.4
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-4.html
* Wed Oct 06 2021 support <info@wazuh.com> - 4.2.3
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-3.html
* Tue Sep 28 2021 support <info@wazuh.com> - 4.2.2
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-2.html
* Sat Sep 25 2021 support <info@wazuh.com> - 4.2.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-1.html
* Mon Apr 26 2021 support <info@wazuh.com> - 4.2.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-2-0.html
* Sat Apr 24 2021 support <info@wazuh.com> - 3.13.3
- More info: https://documentation.wazuh.com/current/release-notes/release-3-13-3.html
* Thu Apr 22 2021 support <info@wazuh.com> - 4.1.5
- More info: https://documentation.wazuh.com/current/release-notes/release-4-1-5.html
* Mon Mar 29 2021 support <info@wazuh.com> - 4.1.4
- More info: https://documentation.wazuh.com/current/release-notes/release-4-1-4.html
* Sat Mar 20 2021 support <info@wazuh.com> - 4.1.3
- More info: https://documentation.wazuh.com/current/release-notes/release-4-1-3.html
* Mon Mar 08 2021 support <info@wazuh.com> - 4.1.2
- More info: https://documentation.wazuh.com/current/release-notes/release-4-1-2.html
* Fri Mar 05 2021 support <info@wazuh.com> - 4.1.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-1-1.html
* Tue Jan 19 2021 support <info@wazuh.com> - 4.1.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-1-0.html
* Mon Nov 30 2020 support <info@wazuh.com> - 4.0.3
- More info: https://documentation.wazuh.com/current/release-notes/release-4-0-3.html
* Mon Nov 23 2020 support <info@wazuh.com> - 4.0.2
- More info: https://documentation.wazuh.com/current/release-notes/release-4-0-2.html
* Sat Oct 31 2020 support <info@wazuh.com> - 4.0.1
- More info: https://documentation.wazuh.com/current/release-notes/release-4-0-1.html
* Mon Oct 19 2020 support <info@wazuh.com> - 4.0.0
- More info: https://documentation.wazuh.com/current/release-notes/release-4-0-0.html
* Fri Aug 21 2020 support <info@wazuh.com> - 3.13.2
- More info: https://documentation.wazuh.com/current/release-notes/release-3-13-2.html
* Tue Jul 14 2020 support <info@wazuh.com> - 3.13.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-13-1.html
* Mon Jun 29 2020 support <info@wazuh.com> - 3.13.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-13-0.html
* Wed May 13 2020 support <info@wazuh.com> - 3.12.3
- More info: https://documentation.wazuh.com/current/release-notes/release-3-12-3.html
* Thu Apr 9 2020 support <info@wazuh.com> - 3.12.2
- More info: https://documentation.wazuh.com/current/release-notes/release-3-12-2.html
* Wed Apr 8 2020 support <info@wazuh.com> - 3.12.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-12-1.html
* Wed Mar 25 2020 support <info@wazuh.com> - 3.12.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-12-0.html
* Mon Feb 24 2020 support <info@wazuh.com> - 3.11.4
- More info: https://documentation.wazuh.com/current/release-notes/release-3-11-4.html
* Wed Jan 22 2020 support <info@wazuh.com> - 3.11.3
- More info: https://documentation.wazuh.com/current/release-notes/release-3-11-3.html
* Tue Jan 7 2020 support <info@wazuh.com> - 3.11.2
- More info: https://documentation.wazuh.com/current/release-notes/release-3-11-2.html
* Thu Dec 26 2019 support <info@wazuh.com> - 3.11.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-11-1.html
* Mon Oct 7 2019 support <info@wazuh.com> - 3.11.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-11-0.html
* Mon Sep 23 2019 support <support@wazuh.com> - 3.10.2
- More info: https://documentation.wazuh.com/current/release-notes/release-3-10-2.html
* Thu Sep 19 2019 support <support@wazuh.com> - 3.10.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-10-1.html
* Mon Aug 26 2019 support <support@wazuh.com> - 3.10.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-10-0.html
* Thu Aug 8 2019 support <support@wazuh.com> - 3.9.5
- More info: https://documentation.wazuh.com/current/release-notes/release-3-9-5.html
* Fri Jul 12 2019 support <support@wazuh.com> - 3.9.4
- More info: https://documentation.wazuh.com/current/release-notes/release-3-9-4.html
* Tue Jul 02 2019 support <support@wazuh.com> - 3.9.3
- More info: https://documentation.wazuh.com/current/release-notes/release-3-9-3.html
* Tue Jun 11 2019 support <support@wazuh.com> - 3.9.2
- More info: https://documentation.wazuh.com/current/release-notes/release-3-9-2.html
* Sat Jun 01 2019 support <support@wazuh.com> - 3.9.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-9-1.html
* Mon Feb 25 2019 support <support@wazuh.com> - 3.9.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-9-0.html
* Wed Jan 30 2019 support <support@wazuh.com> - 3.8.2
- More info: https://documentation.wazuh.com/current/release-notes/release-3-8-2.html
* Thu Jan 24 2019 support <support@wazuh.com> - 3.8.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-8-1.html
* Fri Jan 18 2019 support <support@wazuh.com> - 3.8.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-8-0.html
* Wed Nov 7 2018 support <support@wazuh.com> - 3.7.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-7-0.html
* Mon Sep 10 2018 support <info@wazuh.com> - 3.6.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-6-1.html
* Fri Sep 7 2018 support <support@wazuh.com> - 3.6.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-6-0.html
* Wed Jul 25 2018 support <support@wazuh.com> - 3.5.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-5-0.html
* Wed Jul 11 2018 support <support@wazuh.com> - 3.4.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-4-0.html
* Mon Jun 18 2018 support <support@wazuh.com> - 3.3.1
- More info: https://documentation.wazuh.com/current/release-notes/release-3-3-1.html
* Mon Jun 11 2018 support <support@wazuh.com> - 3.3.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-3-0.html
* Wed May 30 2018 support <support@wazuh.com> - 3.2.4
- More info: https://documentation.wazuh.com/current/release-notes/release-3-2-4.html
* Thu May 10 2018 support <support@wazuh.com> - 3.2.3
- More info: https://documentation.wazuh.com/current/release-notes/release-3-2-3.html
* Mon Apr 09 2018 support <support@wazuh.com> - 3.2.2
- More info: https://documentation.wazuh.com/current/release-notes/release-3-2-2.html
* Wed Feb 21 2018 support <support@wazuh.com> - 3.2.1
- More info: https://documentation.wazuh.com/current/release-notes/rerlease-3-2-1.html
* Wed Feb 07 2018 support <support@wazuh.com> - 3.2.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-2-0.html
* Thu Dec 21 2017 support <support@wazuh.com> - 3.1.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-1-0.html
* Mon Nov 06 2017 support <support@wazuh.com> - 3.0.0
- More info: https://documentation.wazuh.com/current/release-notes/release-3-0-0.html
* Tue Jun 06 2017 support <support@wazuh.com> - 2.0.1
- Changed random data generator for a secure OS-provided generator.
- Changed Windows installer file name (depending on version).
- Linux distro detection using standard os-release file.
- Changed some URLs to documentation.
- Disable synchronization with SQLite databases for Syscheck by default.
- Minor changes at Rootcheck formatter for JSON alerts.
- Added debugging messages to Integrator logs.
- Show agent ID when possible on logs about incorrectly formatted messages.
- Use default maximum inotify event queue size.
- Show remote IP on encoding format errors when unencrypting messages.
- Fix permissions in agent-info folder
- Fix permissions in rids folder.
* Fri Apr 21 2017 Jose Luis Ruiz <jose@wazuh.com> - 2.0
- Changed random data generator for a secure OS-provided generator.
- Changed Windows installer file name (depending on version).
- Linux distro detection using standard os-release file.
- Changed some URLs to documentation.
- Disable synchronization with SQLite databases for Syscheck by default.
- Minor changes at Rootcheck formatter for JSON alerts.
- Added debugging messages to Integrator logs.
- Show agent ID when possible on logs about incorrectly formatted messages.
- Use default maximum inotify event queue size.
- Show remote IP on encoding format errors when unencrypting messages.
- Fixed resource leaks at rules configuration parsing.
- Fixed memory leaks at rules parser.
- Fixed memory leaks at XML decoders parser.
- Fixed TOCTOU condition when removing directories recursively.
- Fixed insecure temporary file creation for old POSIX specifications.
- Fixed missing agentless devices identification at JSON alerts.
- Fixed FIM timestamp and file name issue at SQLite database.
- Fixed cryptographic context acquirement on Windows agents.
- Fixed debug mode for Analysisd.
- Fixed bad exclusion of BTRFS filesystem by Rootcheck.
- Fixed compile errors on macOS.
- Fixed option -V for Integrator.
- Exclude symbolic links to directories when sending FIM diffs (by Stephan Joerrens).
- Fixed daemon list for service reloading at wazuh-control.
- Fixed socket waiting issue on Windows agents.
- Fixed PCI_DSS definitions grouping issue at Rootcheck controls.
