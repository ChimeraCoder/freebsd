#!/bin/sh
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Functions which runs commands on the system

. ${BACKEND}/functions.sh
. ${BACKEND}/functions-parse.sh


# Function which checks and sets up auto-login for a user if specified
check_autologin()
{
  get_value_from_cfg autoLoginUser
  if [ -n "${VAL}"  -a "${INSTALLTYPE}" = "PCBSD" ]
  then
    AUTOU="${VAL}"
    # Add the auto-login user line
    sed -i.bak "s/AutoLoginUser=/AutoLoginUser=${AUTOU}/g" ${FSMNT}/usr/local/kde4/share/config/kdm/kdmrc

    # Add the auto-login user line
    sed -i.bak "s/AutoLoginEnable=false/AutoLoginEnable=true/g" ${FSMNT}/usr/local/kde4/share/config/kdm/kdmrc

  fi
};

# Function which actually runs the adduser command on the filesystem
add_user()
{
 ARGS="${1}"

 if [ -e "${FSMNT}/.tmpPass" ]
 then
   # Add a user with a supplied password
   run_chroot_cmd "cat /.tmpPass | pw useradd ${ARGS}"
   rc_halt "rm ${FSMNT}/.tmpPass"
 else
   # Add a user with no password
   run_chroot_cmd "cat /.tmpPass | pw useradd ${ARGS}"
 fi

};

# Function which reads in the config, and adds any users specified
setup_users()
{

  # We are ready to start setting up the users, lets read the config
  while read line
  do

    echo $line | grep -q "^userName=" 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "${line}"
      USERNAME="$VAL"
    fi

    echo $line | grep -q "^userComment=" 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "${line}"
      USERCOMMENT="$VAL"
    fi

    echo $line | grep -q "^userPass=" 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "${line}"
      USERPASS="$VAL"
    fi

    echo $line | grep -q "^userEncPass=" 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "${line}"
      USERENCPASS="$VAL"
    fi

    echo $line | grep -q "^userShell=" 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "${line}"
      strip_white_space "$VAL"
      USERSHELL="$VAL"
    fi

    echo $line | grep -q "^userHome=" 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "${line}"
      USERHOME="$VAL"
    fi

    echo $line | grep -q "^userGroups=" 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "${line}"
      USERGROUPS="$VAL"
    fi


    echo $line | grep -q "^commitUser" 2>/dev/null
    if [ $? -eq 0 ]
    then
      # Found our flag to commit this user, lets check and do it
      if [ -n "${USERNAME}" ]
      then

        # Now add this user to the system, by building our args list
        ARGS="-n ${USERNAME}"

        if [ -n "${USERCOMMENT}" ]
        then
          ARGS="${ARGS} -c \"${USERCOMMENT}\""
        fi
         
        if [ -n "${USERPASS}" ]
        then
          ARGS="${ARGS} -h 0"
          echo "${USERPASS}" >${FSMNT}/.tmpPass
	elif [ -n "${USERENCPASS}" ] 
	then
          ARGS="${ARGS} -H 0"
          echo "${USERENCPASS}" >${FSMNT}/.tmpPass
        else
          ARGS="${ARGS} -h -"
          rm ${FSMNT}/.tmpPass 2>/dev/null 2>/dev/null
        fi

        if [ -n "${USERSHELL}" ]
        then
          ARGS="${ARGS} -s \"${USERSHELL}\""
        else
          ARGS="${ARGS} -s \"/nonexistant\""
        fi
         
        if [ -n "${USERHOME}" ]
        then
          ARGS="${ARGS} -m -d \"${USERHOME}\""
        fi

        if [ -n "${USERGROUPS}" ]
        then
          ARGS="${ARGS} -G \"${USERGROUPS}\""
        fi

        add_user "${ARGS}"

        # Unset our vars before looking for any more users
        unset USERNAME USERCOMMENT USERPASS USERENCPASS USERSHELL USERHOME USERGROUPS
      else
        exit_err "ERROR: commitUser was called without any userName= entry!!!" 
      fi
    fi

  done <${CFGF}


  # Check if we need to enable a user to auto-login to the desktop
  check_autologin

};