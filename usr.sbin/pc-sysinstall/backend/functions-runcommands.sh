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

run_chroot_cmd()
{
  CMD="$@"
  echo_log "Running chroot command: ${CMD}"
  echo "$CMD" >${FSMNT}/.runcmd.sh
  chmod 755 ${FSMNT}/.runcmd.sh
  chroot ${FSMNT} sh /.runcmd.sh
  RES=$?

  rm ${FSMNT}/.runcmd.sh
  return ${RES}
};

run_chroot_script()
{
  SCRIPT="$@"
  SBASE=`basename $SCRIPT`

  cp ${SCRIPT} ${FSMNT}/.$SBASE
  chmod 755 ${FSMNT}/.${SBASE}

  echo_log "Running chroot script: ${SCRIPT}"
  chroot ${FSMNT} /.${SBASE}
  RES=$?

  rm ${FSMNT}/.${SBASE}
  return ${RES}
};


run_ext_cmd()
{
  CMD="$@"
  # Make sure to export FSMNT, in case cmd needs it
  export FSMNT
  echo_log "Running external command: ${CMD}"
  echo "${CMD}"> ${TMPDIR}/.runcmd.sh
  chmod 755 ${TMPDIR}/.runcmd.sh
  sh ${TMPDIR}/.runcmd.sh
  RES=$?

  rm ${TMPDIR}/.runcmd.sh
  return ${RES}
};


# Starts the user setup
run_commands()
{
  while read line
  do
    # Check if we need to run any chroot command
    echo $line | grep -q ^runCommand=  2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "$line"
      run_chroot_cmd "$VAL"
    fi

    # Check if we need to run any chroot script
    echo $line | grep -q ^runScript= 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "$line"
      run_chroot_script "$VAL"
    fi

    # Check if we need to run any chroot command
    echo $line | grep -q ^runExtCommand= 2>/dev/null
    if [ $? -eq 0 ]
    then
      get_value_from_string "$line"
      run_ext_cmd "$VAL"
    fi

  done <${CFGF}

};