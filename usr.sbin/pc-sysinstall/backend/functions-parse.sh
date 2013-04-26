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

# functions.sh
# Library of functions which pc-sysinstall may call upon for parsing the config

# which gets the value of a setting in the provided line
get_value_from_string()
{
  if [ -n "${1}" ]
  then
    export VAL="`echo ${1} | cut -d '=' -f 2-`"
  else
    echo "Error: Did we forgot to supply a string to parse?"
    exit 1
  fi
};

# Get the value from the cfg file including spaces
get_value_from_cfg_with_spaces()
{
  if [ -n "${1}" ]
  then
    export VAL="`grep ^${1}= ${CFGF} | head -n 1 | cut -d '=' -f 2-`"
  else
    exit_err "Error: Did we forgot to supply a setting to grab?"
  fi
};


# Get the value from the cfg file
get_value_from_cfg()
{
  if [ -n "${1}" ]
  then
    export VAL=`grep "^${1}=" ${CFGF} | head -n 1 | cut -d '=' -f 2- | tr -d ' '`
  else
    exit_err "Error: Did we forgot to supply a setting to grab?"
  fi
};

# Checks the value of a setting in the provided line with supplied possibilities
# 1 = setting we are checking,  2 = list of valid values
if_check_value_exists()
{
  if [ -n "${1}" -a -n "${2}" ]
  then
    # Get the first occurrence of the setting from the config, strip out whitespace

    VAL=`grep "^${1}" ${CFGF} | head -n 1 | cut -d '=' -f 2- | tr -d ' '`
    if [ -z "${VAL}" ]
    then
      # This value doesn't exist, lets return
      return 0
    fi


    VALID="1"
    for i in ${2}
    do
      VAL=`echo "$VAL"|tr A-Z a-z`
      if [ "$VAL" = "${i}" ]
      then 
        VALID="0"
      fi
    done 
    if [ "$VALID" = "1" ]
    then
      exit_err "Error: ${1} is set to unknown value $VAL"
    fi
  else
    exit_err "Error: Did we forgot to supply a string to parse and setting to grab?"
  fi
};

# Checks the value of a setting in the provided line with supplied possibilities
# 1 = setting we are checking,  2 = list of valid values
check_value()
{
  if [ -n "${1}" -a -n "${2}" ]
  then
    # Get the first occurrence of the setting from the config, strip out whitespace
    VAL=`grep "^${1}" ${CFGF} | head -n 1 | cut -d '=' -f 2- | tr -d ' '`
    VALID="1"
    for i in ${2}
    do
      if [ "$VAL" = "${i}" ]
      then 
        VALID="0"
      fi
    done 
    if [ "$VALID" = "1" ]
    then
      exit_err "Error: ${1} is set to unknown value $VAL"
    fi
  else
    exit_err "Error: Did we forgot to supply a string to parse and setting to grab?"
  fi
};

# Checks for the presence of the supplied arguments in the config file
# 1  = values to confirm exist
file_sanity_check()
{
  if [ -n "$CFGF" -a -n "$1" ]
  then
    for i in $1
    do
      grep -q "^${i}=" $CFGF 2>/dev/null
      if [ $? -eq 0 ]
      then
        LN=`grep "^${i}=" ${CFGF} | head -n 1 | cut -d '=' -f 2- | tr -d ' '`
        if [ -z "${LN}" ]
        then
          echo "Error: Config fails sanity test! ${i}= is empty"
          exit 1
        fi
      else
        echo "Error: Config fails sanity test! Missing ${i}="
        exit 1
      fi
    done
  else
    echo "Error: Missing config file, and / or values to sanity check for!"
    exit 1
  fi
};


# Function which merges the contents of a new config into the specified old one
# Only works with <val>= type configurations
merge_config()
{
  OLDCFG="${1}"
  NEWCFG="${2}"
  FINALCFG="${3}"

  # Copy our oldcfg to the new one, which will be used as basis
  cp ${OLDCFG} ${FINALCFG}

  # Remove blank lines from new file
  cat ${NEWCFG} | sed '/^$/d' > ${FINALCFG}.tmp

  # Set our marker if we've found any
  FOUNDMERGE="NO"

  while read newline
  do
   echo ${newline} | grep -q "^#" 2>/dev/null
   if [ $? -ne 0 ] ; then
     VAL="`echo ${newline} | cut -d '=' -f 1`"
     cat ${OLDCFG} | grep -q ${VAL} 2>/dev/null
     if [ $? -ne 0 ] ; then
       if [ "${FOUNDMERGE}" = "NO" ] ; then
         echo "" >> ${FINALCFG}
         echo "# Auto-merged values from newer ${NEWCFG}" >> ${FINALCFG}
         FOUNDMERGE="YES"
       fi
       echo "${newline}" >> ${FINALCFG}
     fi
   fi
  done < ${FINALCFG}.tmp
  rm ${FINALCFG}.tmp

};

# Loop to check for a specified mount-point in a list
check_for_mount()
{
  MNTS="${1}"
  FINDMNT="${2}"

  # Check if we found a valid root partition
  for CHECKMNT in `echo ${MNTS} | sed 's|,| |g'`
  do
    if [ "${CHECKMNT}" = "${FINDMNT}" ] ; then
      return 0
    fi
  done
    
  return 1
};

# Function which returns the next line in the specified config file
get_next_cfg_line()
{
  CURFILE="$1"
  CURLINE="$2"

  FOUND="1"
  
  while read line
  do
    if [ "$FOUND" = "0" ] ; then
      export VAL="$line"
      return
    fi
    if [ "$line" = "${CURLINE}" ] ; then
      FOUND="0"
    fi
  done <${CURFILE}

  # Got here, couldn't find this line or at end of file, set VAL to ""
  export VAL=""
};