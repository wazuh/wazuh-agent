#!/bin/bash
NUM_CPUS=""
WAZUH_GROUP='wazuh'
WAZUH_USER='wazuh'

get_system(){
    #Function to detect sytem
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macOS"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [[ $(apt --version 2>/dev/null) =~ [0-9] ]]; then
            echo "DEB"
        elif [[ $(yum --version 2>/dev/null) =~ [0-9] ]]; then
            echo "RPM"
        else
            echo "Unknown Linux system"
        fi
    else
        echo "Unknown system"
    fi
}

set_dafault_installation_variables(){
    #Function to set installation variables given a system
    local system=$1
    echo "Setting installation variables for $1"
    case "$system" in
        "macOS")
            NUM_CPUS=$(sysctl -n hw.ncpu)
            BIN_FOLDER=/usr/local/bin
            SETTINGS_FOLDER=/etc/wazuh-agent
            SERVICE_FOLDER=/etc/systemd/system
            DATA_FOLDER=/var/wazuh-agent
            ;;
        "DEB")
            NUM_CPUS=$(nproc)
            BIN_FOLDER=/usr/local/bin
            SETTINGS_FOLDER=/etc/wazuh-agent
            SERVICE_FOLDER=/etc/systemd/system
            DATA_FOLDER=/var/wazuh-agent
            ;;
        "RPM")
            NUM_CPUS=$(nproc)
            BIN_FOLDER=/usr/local/bin
            SETTINGS_FOLDER=/etc/wazuh-agent
            SERVICE_FOLDER=/etc/systemd/system
            DATA_FOLDER=/var/wazuh-agent
            ;;
        *)
            echo "$system is not a supported system"
            ;;
    esac
}

build_binary(){
    #Function to build the agent binary
    echo "Building binary" 
    cd src && git submodule update --init --recursive
    mkdir build && cd build
    cmake .. && make -j $NUM_CPUS
    cd ../..
}

add_user(){
    #Function to add the wazuh user
    echo "Adding wazuh user"
    local system=$1
    case "$system" in
        "macOS")
            add_user_macos
            ;;
        *)
            add_user_linux
            ;;
    esac
}

create_installation_directories(){
    #Function to build the agent's installation directories
    echo "Creating installation directories"
    install -d -m 0770 -o ${WAZUH_USER} -g ${WAZUH_GROUP} ${BIN_FOLDER}
    install -d -m 0770 -o ${WAZUH_USER} -g ${WAZUH_GROUP} ${SETTINGS_FOLDER}
    install -d -m 0770 -o ${WAZUH_USER} -g ${WAZUH_GROUP} ${SERVICE_FOLDER}
    install -d -m 0770 -o ${WAZUH_USER} -g ${WAZUH_GROUP} ${DATA_FOLDER}

    install -d -m 0770 -o ${WAZUH_USER} -g ${WAZUH_GROUP} ${DATA_FOLDER}/dbs
    install -d -m 0770 -o ${WAZUH_USER} -g ${WAZUH_GROUP} ${DATA_FOLDER}/logs

}

install_files(){
    #Function to build the agent's installation directories
    echo "Installing files"
    install -m 0750 -o root -g 0 src/build/wazuh-agent ${BIN_FOLDER}/wazuh-agent
}

add_user_macos(){
    if [[ ! -f "/usr/bin/dscl" ]]; then
        echo "Error, dscl is needed, dying here";
        exit
    fi
            
    DSCL="/usr/bin/dscl";

    # get unique id numbers (uid, gid) that are greater than 100
    unset -v i new_uid new_gid idvar;
    declare -i new_uid=0 new_gid=0 i=100 idvar=0;
    while [[ $idvar -eq 0 ]]; do
        i=$[i+1]
        j=$[i+1]
        k=$[i+2]
        if  [[ -z "$(/usr/bin/dscl . -search /Users uid ${i})" ]] && [[ -z "$(/usr/bin/dscl . -search /Groups gid ${i})" ]] && \
            [[ -z "$(/usr/bin/dscl . -search /Users uid ${j})" ]] && [[ -z "$(/usr/bin/dscl . -search /Groups gid ${j})" ]] && \
            [[ -z "$(/usr/bin/dscl . -search /Users uid ${k})" ]] && [[ -z "$(/usr/bin/dscl . -search /Groups gid ${k})" ]]; then
            new_uid=$i
            new_gid=$i
            idvar=1
            #break
        fi
    done

    echo "UIDs available: $i $j $k";

    # Verify that the uid and gid exist and match
    if [[ $new_uid -eq 0 ]] || [[ $new_gid -eq 0 ]];then
        echo "Getting unique id numbers (uid, gid) failed!";
        exit 1;            
    fi

    if [[ ${new_uid} != ${new_gid} ]]; then
        echo "I failed to find matching free uid and gid!";
        exit 5;
    fi


     # Creating the groups.
    sudo ${DSCL} localhost -create /Local/Default/Groups/${GROUP}
    echo "Error creating group $GROUP" "67"
    sudo ${DSCL} localhost -createprop /Local/Default/Groups/${GROUP} PrimaryGroupID ${new_gid}
    sudo ${DSCL} localhost -createprop /Local/Default/Groups/${GROUP} RealName ${GROUP}
    sudo ${DSCL} localhost -createprop /Local/Default/Groups/${GROUP} RecordName ${GROUP}
    sudo ${DSCL} localhost -createprop /Local/Default/Groups/${GROUP} RecordType: dsRecTypeStandard:Groups
    sudo ${DSCL} localhost -createprop /Local/Default/Groups/${GROUP} Password "*"

    # Creating the users.
    if [[ $(${DSCL} . -read /Users/${USER} 2>/dev/null) ]]; then
        echo "${USER} already exists";
    else
        sudo ${DSCL} localhost -create /Local/Default/Users/${USER}
        echo "Error creating user ${USER}" "87"
        sudo ${DSCL} localhost -createprop /Local/Default/Users/${USER} RecordName ${USER}
        sudo ${DSCL} localhost -createprop /Local/Default/Users/${USER} RealName "${USER}acct"
        sudo ${DSCL} localhost -createprop /Local/Default/Users/${USER} NFSHomeDirectory ${DIR}
        sudo ${DSCL} localhost -createprop /Local/Default/Users/${USER} UniqueID ${i}
        sudo ${DSCL} localhost -createprop /Local/Default/Users/${USER} PrimaryGroupID ${new_gid}
        sudo ${DSCL} localhost -append /Local/Default/Groups/${GROUP} GroupMembership ${USER}
        sudo ${DSCL} localhost -createprop /Local/Default/Users/${USER} Password "*"
    fi

    sudo ${DSCL} . create /Users/wazuh IsHidden 1
}

add_user_linux(){
    groupadd -f "${WAZUH_GROUP}"
    useradd "${WAZUH_USER}" -s /sbin/nologin -g "${WAZUH_GROUP}"
}