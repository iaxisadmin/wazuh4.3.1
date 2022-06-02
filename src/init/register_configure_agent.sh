#!/bin/bash

# Copyright (C) 2015, Wazuh Inc.
# March 6, 2019.
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

# Global variables
INSTALLDIR=${1}
CONF_FILE="${INSTALLDIR}/etc/ossec.conf"
TMP_ENROLLMENT="${INSTALLDIR}/tmp/autoenrollment.conf"


# Set default sed alias
sed="sed -ri"
# By default, use gnu sed (gsed).
use_unix_sed="False"

# Special function to use generic sed
unix_sed() {
    sed_expression="$1"
    target_file="$2"
    special_args="$3"

    sed ${special_args} ${sed_expression} ${target_file} > ${target_file}.tmp
    cat "${target_file}.tmp" > "${target_file}"
    rm "${target_file}.tmp"
}

# Update the value of a XML tag inside the ossec.conf
edit_value_tag() {

    file=""

    if [ -z "$3" ]; then
        file="${CONF_FILE}"
    else
        file="${TMP_ENROLLMENT}"
    fi

    if [ ! -z "$1" ] && [ ! -z "$2" ]; then
        start_config="$(grep -n "<$1>" ${file} | cut -d':' -f 1)"
        end_config="$(grep -n "</$1>" ${file} | cut -d':' -f 1)"
        if [ ! -n "${start_config}" ] && [ ! -n "${end_config}" ] && [ "${file}" = "${TMP_ENROLLMENT}" ]; then
            echo "      <$1>$2</$1>" >> ${file}
        elif [ "${use_unix_sed}" = "False" ] ; then
            ${sed} "s#<$1>.*</$1>#<$1>$2</$1>#g" "${file}"
        else
            unix_sed "s#<$1>.*</$1>#<$1>$2</$1>#g" "${file}"
        fi
    fi
    
    if [ "$?" != "0" ]; then
        echo "$(date '+%Y/%m/%d %H:%M:%S') agent-auth: Error updating $2 with variable $1." >> ${INSTALLDIR}/logs/ossec.log
    fi
}

delete_blank_lines() {
    file=$1
    if [ "${use_unix_sed}" = "False" ] ; then
        ${sed} '/^$/d' "${file}"
    else
        unix_sed '/^$/d' "${file}"
    fi
}
delete_auto_enrollment_tag() {
    # Delete the configuration tag if its value is empty
    # This will allow using the default value
    if [ "${use_unix_sed}" = "False" ] ; then
        ${sed} "s#.*<$1>.*</$1>.*##g" "${TMP_ENROLLMENT}"
    else
        unix_sed "s#.*<$1>.*</$1>.*##g" "${TMP_ENROLLMENT}"
    fi

    cat -s "${TMP_ENROLLMENT}" > "${TMP_ENROLLMENT}.tmp"
    mv "${TMP_ENROLLMENT}.tmp" "${TMP_ENROLLMENT}"
}

# Change address block of the ossec.conf
add_adress_block() {
    # Getting function parameters on new variable
    SET_ADDRESSES="$@"

    # Remove the server configuration
    if [ "${use_unix_sed}" = "False" ] ; then
        ${sed} "/<server>/,/\/server>/d" ${CONF_FILE}
    else
        unix_sed "/<server>/,/\/server>/d" "${CONF_FILE}"
    fi

    # Get the client configuration generated by gen_ossec.sh
    start_config="$(grep -n "<client>" ${CONF_FILE} | head -n 1 | cut -d':' -f 1)"
    end_config="$(grep -n "</client>" ${CONF_FILE} | head -n 1 | cut -d':' -f 1)"
    start_config=$(( start_config + 1 ))
    end_config=$(( end_config - 1 ))
    client_config="$(sed -n "${start_config},${end_config}p" ${CONF_FILE})"

    # Remove the client configuration
    if [ "${use_unix_sed}" = "False" ] ; then
        ${sed} "/<client>/,/\/client>/d" ${CONF_FILE}
    else
        unix_sed "/<client>/,/\/client>/d" "${CONF_FILE}"
    fi

    # Write the client configuration block
    echo "<ossec_config>" >> ${CONF_FILE}
    echo "  <client>" >> ${CONF_FILE}
    for i in ${SET_ADDRESSES};
    do
        echo "    <server>" >> ${CONF_FILE}
        echo "      <address>$i</address>" >> ${CONF_FILE}
        echo "      <port>1514</port>" >> ${CONF_FILE}
        echo "      <protocol>tcp</protocol>" >> ${CONF_FILE}
        echo "    </server>" >> ${CONF_FILE}
    done

    echo "${client_config}" >> ${CONF_FILE}
    echo "  </client>" >> ${CONF_FILE}
    echo "</ossec_config>" >> ${CONF_FILE}
}

add_parameter () {
    if [ ! -z "$3" ]; then
        OPTIONS="$1 $2 $3"
    fi
    echo ${OPTIONS}
}

get_deprecated_vars () {
    if [ ! -z "${IRONCLOUD_MANAGER_IP}" ] && [ -z "${IRONCLOUD_MANAGER}" ]; then
        IRONCLOUD_MANAGER=${IRONCLOUD_MANAGER_IP}
    fi
    if [ ! -z "${IRONCLOUD_AUTHD_SERVER}" ] && [ -z "${IRONCLOUD_REGISTRATION_SERVER}" ]; then
        IRONCLOUD_REGISTRATION_SERVER=${IRONCLOUD_AUTHD_SERVER}
    fi
    if [ ! -z "${IRONCLOUD_AUTHD_PORT}" ] && [ -z "${IRONCLOUD_REGISTRATION_PORT}" ]; then
        IRONCLOUD_REGISTRATION_PORT=${IRONCLOUD_AUTHD_PORT}
    fi
    if [ ! -z "${IRONCLOUD_PASSWORD}" ] && [ -z "${IRONCLOUD_REGISTRATION_PASSWORD}" ]; then
        IRONCLOUD_REGISTRATION_PASSWORD=${IRONCLOUD_PASSWORD}
    fi
    if [ ! -z "${IRONCLOUD_NOTIFY_TIME}" ] && [ -z "${IRONCLOUD_KEEP_ALIVE_INTERVAL}" ]; then
        IRONCLOUD_KEEP_ALIVE_INTERVAL=${IRONCLOUD_NOTIFY_TIME}
    fi
    if [ ! -z "${IRONCLOUD_CERTIFICATE}" ] && [ -z "${IRONCLOUD_REGISTRATION_CA}" ]; then
        IRONCLOUD_REGISTRATION_CA=${IRONCLOUD_CERTIFICATE}
    fi
    if [ ! -z "${IRONCLOUD_PEM}" ] && [ -z "${IRONCLOUD_REGISTRATION_CERTIFICATE}" ]; then
        IRONCLOUD_REGISTRATION_CERTIFICATE=${IRONCLOUD_PEM}
    fi
    if [ ! -z "${IRONCLOUD_KEY}" ] && [ -z "${IRONCLOUD_REGISTRATION_KEY}" ]; then
        IRONCLOUD_REGISTRATION_KEY=${IRONCLOUD_KEY}
    fi
    if [ ! -z "${IRONCLOUD_GROUP}" ] && [ -z "${IRONCLOUD_AGENT_GROUP}" ]; then
        IRONCLOUD_AGENT_GROUP=${IRONCLOUD_GROUP}
    fi
}

set_vars () {
    export IRONCLOUD_MANAGER=$(launchctl getenv IRONCLOUD_MANAGER)
    export IRONCLOUD_MANAGER_PORT=$(launchctl getenv IRONCLOUD_MANAGER_PORT)
    export IRONCLOUD_PROTOCOL=$(launchctl getenv IRONCLOUD_PROTOCOL)
    export IRONCLOUD_REGISTRATION_SERVER=$(launchctl getenv IRONCLOUD_REGISTRATION_SERVER)
    export IRONCLOUD_REGISTRATION_PORT=$(launchctl getenv IRONCLOUD_REGISTRATION_PORT)
    export IRONCLOUD_REGISTRATION_PASSWORD=$(launchctl getenv IRONCLOUD_REGISTRATION_PASSWORD)
    export IRONCLOUD_KEEP_ALIVE_INTERVAL=$(launchctl getenv IRONCLOUD_KEEP_ALIVE_INTERVAL)
    export IRONCLOUD_TIME_RECONNECT=$(launchctl getenv IRONCLOUD_TIME_RECONNECT)
    export IRONCLOUD_REGISTRATION_CA=$(launchctl getenv IRONCLOUD_REGISTRATION_CA)
    export IRONCLOUD_REGISTRATION_CERTIFICATE=$(launchctl getenv IRONCLOUD_REGISTRATION_CERTIFICATE)
    export IRONCLOUD_REGISTRATION_KEY=$(launchctl getenv IRONCLOUD_REGISTRATION_KEY)
    export IRONCLOUD_AGENT_NAME=$(launchctl getenv IRONCLOUD_AGENT_NAME)
    export IRONCLOUD_AGENT_GROUP=$(launchctl getenv IRONCLOUD_AGENT_GROUP)
    export ENROLLMENT_DELAY=$(launchctl getenv ENROLLMENT_DELAY)

    # The following variables are yet supported but all of them are deprecated
    export IRONCLOUD_MANAGER_IP=$(launchctl getenv IRONCLOUD_MANAGER_IP)
    export IRONCLOUD_NOTIFY_TIME=$(launchctl getenv IRONCLOUD_NOTIFY_TIME)
    export IRONCLOUD_AUTHD_SERVER=$(launchctl getenv IRONCLOUD_AUTHD_SERVER)
    export IRONCLOUD_AUTHD_PORT=$(launchctl getenv IRONCLOUD_AUTHD_PORT)
    export IRONCLOUD_PASSWORD=$(launchctl getenv IRONCLOUD_PASSWORD)
    export IRONCLOUD_GROUP=$(launchctl getenv IRONCLOUD_GROUP)
    export IRONCLOUD_CERTIFICATE=$(launchctl getenv IRONCLOUD_CERTIFICATE)
    export IRONCLOUD_KEY=$(launchctl getenv IRONCLOUD_KEY)
    export IRONCLOUD_PEM=$(launchctl getenv IRONCLOUD_PEM)
}

unset_vars() {

    OS=$1

    vars=(IRONCLOUD_MANAGER_IP IRONCLOUD_PROTOCOL IRONCLOUD_MANAGER_PORT IRONCLOUD_NOTIFY_TIME \
          IRONCLOUD_TIME_RECONNECT IRONCLOUD_AUTHD_SERVER IRONCLOUD_AUTHD_PORT IRONCLOUD_PASSWORD \
          IRONCLOUD_AGENT_NAME IRONCLOUD_GROUP IRONCLOUD_CERTIFICATE IRONCLOUD_KEY IRONCLOUD_PEM \
          IRONCLOUD_MANAGER IRONCLOUD_REGISTRATION_SERVER IRONCLOUD_REGISTRATION_PORT \
          IRONCLOUD_REGISTRATION_PASSWORD IRONCLOUD_KEEP_ALIVE_INTERVAL IRONCLOUD_REGISTRATION_CA \
          IRONCLOUD_REGISTRATION_CERTIFICATE IRONCLOUD_REGISTRATION_KEY IRONCLOUD_AGENT_GROUP \
          ENROLLMENT_DELAY)

    for var in "${vars[@]}"; do
        if [ "${OS}" = "Darwin" ]; then
            launchctl unsetenv ${var}
        fi
        unset ${var}
    done
}

# Function to convert strings to lower version
tolower () {
    echo $1 | tr '[:upper:]' '[:lower:]'
}


# Add auto-enrollment configuration block
add_auto_enrollment () {
    start_config="$(grep -n "<auto_enrollment>" ${INSTALLDIR}/etc/ossec.conf | cut -d':' -f 1)"
    end_config="$(grep -n "</auto_enrollment>" ${INSTALLDIR}/etc/ossec.conf | cut -d':' -f 1)"
    if [ -n "${start_config}" ] && [ -n "${end_config}" ]; then
        start_config=$(( start_config + 1 ))
        end_config=$(( end_config - 1 ))
        sed -n "${start_config},${end_config}p" ${INSTALLDIR}/etc/ossec.conf >> "${TMP_ENROLLMENT}"
    else
        # Write the client configuration block
        echo "<ossec_config>" >> "${TMP_ENROLLMENT}"
        echo "  <client>" >> "${TMP_ENROLLMENT}"
        echo "    <enrollment>" >> "${TMP_ENROLLMENT}"
        echo "      <enabled>yes</enabled>" >> "${TMP_ENROLLMENT}"
        echo "      <manager_address>MANAGER_IP</manager_address>" >> "${TMP_ENROLLMENT}"
        echo "      <port>1515</port>" >> "${TMP_ENROLLMENT}"
        echo "      <agent_name>agent</agent_name>" >> "${TMP_ENROLLMENT}"
        echo "      <groups>Group1</groups>" >> "${TMP_ENROLLMENT}"
        echo "      <server_ca_path>/path/to/server_ca</server_ca_path>" >> "${TMP_ENROLLMENT}"
        echo "      <agent_certificate_path>/path/to/agent.cert</agent_certificate_path>" >> "${TMP_ENROLLMENT}"
        echo "      <agent_key_path>/path/to/agent.key</agent_key_path>" >> "${TMP_ENROLLMENT}"
        echo "      <delay_after_enrollment>20</delay_after_enrollment>" >> "${TMP_ENROLLMENT}"
        echo "    </enrollment>" >> "${TMP_ENROLLMENT}"
        echo "  </client>" >> "${TMP_ENROLLMENT}"
        echo "</ossec_config>" >> "${TMP_ENROLLMENT}"
    fi
}

# Add the auto_enrollment block to the configuration file
concat_conf(){
    start_config="$(grep -n "<auto_enrollment>" ${INSTALLDIR}/etc/ossec.conf | cut -d':' -f 1)"
    end_config="$(grep -n "</auto_enrollment>" ${INSTALLDIR}/etc/ossec.conf | cut -d':' -f 1)"
    if [ -n "${start_config}" ] && [ -n "${end_config}" ]; then
        # Remove the server configuration
        if [ "${use_unix_sed}" = "False" ] ; then
            ${sed} -e "/<auto_enrollment>/,/<\/auto_enrollment>/{ /<auto_enrollment>/{p; r ${TMP_ENROLLMENT}
            }; /<\/auto_enrollment>/p; d }" ${CONF_FILE}
        else
            unix_sed "/<auto_enrollment>/,/<\/auto_enrollment>/{ /<auto_enrollment>/{p; r ${TMP_ENROLLMENT}
            }; /<\/auto_enrollment>/p; d }" "${CONF_FILE}" "-e"
        fi
    else
        cat ${TMP_ENROLLMENT} >> ${CONF_FILE}
    fi

    rm -f ${TMP_ENROLLMENT}
}

# Set autoenrollment configuration
set_auto_enrollment_tag_value () {
    tag="$1"
    value="$2"

    if [ ! -z "${value}" ]; then
        edit_value_tag "${tag}" ${value} "auto_enrollment"
    else
        delete_auto_enrollment_tag "${tag}" "auto_enrollment"
    fi
}

# Main function the script begin here
main () {
    uname_s=$(uname -s)

    # Check what kind of system we are working with
    if [ "${uname_s}" = "Darwin" ]; then
        sed="sed -ire"
        set_vars
    elif [ "${uname_s}" = "AIX" ] || [ "${uname_s}" = "SunOS" ] || [ "${uname_s}" = "HP-UX" ]; then
        use_unix_sed="True"
    fi

    get_deprecated_vars

    if [ ! -z ${IRONCLOUD_REGISTRATION_SERVER} ] || [ ! -z ${IRONCLOUD_REGISTRATION_PORT} ] || [ ! -z ${IRONCLOUD_REGISTRATION_CA} ] || [ ! -z ${IRONCLOUD_REGISTRATION_CERTIFICATE} ] || [ ! -z ${IRONCLOUD_REGISTRATION_KEY} ] || [ ! -z ${IRONCLOUD_AGENT_NAME} ] || [ ! -z ${IRONCLOUD_AGENT_GROUP} ]; then
        add_auto_enrollment
        set_auto_enrollment_tag_value "manager_address" ${IRONCLOUD_REGISTRATION_SERVER}
        set_auto_enrollment_tag_value "port" ${IRONCLOUD_REGISTRATION_PORT}
        set_auto_enrollment_tag_value "server_ca_path" ${IRONCLOUD_REGISTRATION_CA}
        set_auto_enrollment_tag_value "agent_certificate_path" ${IRONCLOUD_REGISTRATION_CERTIFICATE}
        set_auto_enrollment_tag_value "agent_key_path" ${IRONCLOUD_REGISTRATION_KEY}
        set_auto_enrollment_tag_value "agent_name" ${IRONCLOUD_AGENT_NAME}
        set_auto_enrollment_tag_value "groups" ${IRONCLOUD_AGENT_GROUP}
        set_auto_enrollment_tag_value "delay_after_enrollment" ${ENROLLMENT_DELAY}
        delete_blank_lines ${TMP_ENROLLMENT}
        concat_conf
    fi

            
    if [ ! -z ${IRONCLOUD_REGISTRATION_PASSWORD} ]; then
        echo ${IRONCLOUD_REGISTRATION_PASSWORD} > "${INSTALLDIR}/etc/authd.pass"
    fi

    if [ ! -z ${IRONCLOUD_MANAGER} ]; then
        if [ ! -f ${INSTALLDIR}/logs/ossec.log ]; then
            touch -f ${INSTALLDIR}/logs/ossec.log
            chmod 660 ${INSTALLDIR}/logs/ossec.log
            chown root:wazuh ${INSTALLDIR}/logs/ossec.log
        fi

        # Check if multiples IPs are defined in variable IRONCLOUD_MANAGER
        IRONCLOUD_MANAGER=$(echo ${IRONCLOUD_MANAGER} | sed "s#,#;#g")
        ADDRESSES="$(echo ${IRONCLOUD_MANAGER} | awk '{split($0,a,";")} END{ for (i in a) { print a[i] } }' |  tr '\n' ' ')"
        if echo ${ADDRESSES} | grep ' ' > /dev/null 2>&1 ; then
            # Get uniques values
            ADDRESSES=$(echo "${ADDRESSES}" | tr ' ' '\n' | sort -u | tr '\n' ' ')
            add_adress_block "${ADDRESSES}"
            if [ -z ${IRONCLOUD_REGISTRATION_SERVER} ]; then
                IRONCLOUD_REGISTRATION_SERVER="$(echo $ADDRESSES | cut -d ' ' -f 1)"
            fi
        else
            # Single address
            edit_value_tag "address" ${IRONCLOUD_MANAGER}
            if [ -z ${IRONCLOUD_REGISTRATION_SERVER} ]; then
                IRONCLOUD_REGISTRATION_SERVER="${IRONCLOUD_MANAGER}"
            fi
        fi
    fi

    # Options to be modified in ossec.conf
    edit_value_tag "protocol" "$(tolower ${IRONCLOUD_PROTOCOL})"
    edit_value_tag "port" ${IRONCLOUD_MANAGER_PORT}
    edit_value_tag "notify_time" ${IRONCLOUD_KEEP_ALIVE_INTERVAL}
    edit_value_tag "time-reconnect" ${IRONCLOUD_TIME_RECONNECT}

    unset_vars ${uname_s}
}

# Start script execution
main "$@"
