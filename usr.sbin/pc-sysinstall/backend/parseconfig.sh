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

# Main install configuration parsing script
#

# Source our functions scripts
. ${BACKEND}/functions.sh
. ${BACKEND}/functions-bsdlabel.sh
. ${BACKEND}/functions-cleanup.sh
. ${BACKEND}/functions-disk.sh
. ${BACKEND}/functions-extractimage.sh
. ${BACKEND}/functions-installcomponents.sh
. ${BACKEND}/functions-installpackages.sh
. ${BACKEND}/functions-localize.sh
. ${BACKEND}/functions-mountdisk.sh
. ${BACKEND}/functions-networking.sh
. ${BACKEND}/functions-newfs.sh
. ${BACKEND}/functions-packages.sh
. ${BACKEND}/functions-parse.sh
. ${BACKEND}/functions-runcommands.sh
. ${BACKEND}/functions-ftp.sh
. ${BACKEND}/functions-unmount.sh
. ${BACKEND}/functions-upgrade.sh
. ${BACKEND}/functions-users.sh

# Check that the config file exists
if [ ! -e "${1}" ]
then
  echo "ERROR: Install configuration $1 does not exist!"
  exit 1
fi

# Set our config file variable
CFGF="$1"

# Resolve any relative pathing
CFGF="`realpath ${CFGF}`"
export CFGF

# Start by doing a sanity check, which will catch any obvious mistakes in the config
file_sanity_check "installMode installType installMedium packageType"

# We passed the Sanity check, lets grab some of the universal config settings and store them
check_value installMode "fresh upgrade extract"
check_value installType "PCBSD FreeBSD"
check_value installMedium "dvd usb ftp rsync image local"
check_value packageType "uzip tar rsync split dist"
if_check_value_exists mirrorbal "load prefer round-robin split"

# We passed all sanity checks! Yay, lets start the install
echo "File Sanity Check -> OK"

# Lets load the various universal settings now
get_value_from_cfg installMode
export INSTALLMODE="${VAL}"

get_value_from_cfg installType
export INSTALLTYPE="${VAL}"

get_value_from_cfg installMedium
export INSTALLMEDIUM="${VAL}"

get_value_from_cfg packageType
export PACKAGETYPE="${VAL}"

# Check if we are doing any networking setup
start_networking

# If we are not doing an upgrade, lets go ahead and setup the disk
case "${INSTALLMODE}" in
  fresh)
    if [ "${INSTALLMEDIUM}" = "image" ]
    then
      install_image
    else
      install_fresh
    fi
    ;;

  extract)
    # Extracting only, make sure we have a valid target directory
    get_value_from_cfg installLocation
    export FSMNT="${VAL}"
    if [ -z "$FSMNT" ] ; then exit_err "Missing installLocation=" ; fi
    if [ ! -d "$FSMNT" ] ; then exit_err "No such directory: $FSMNT" ; fi

    install_extractonly
    ;;

  upgrade)
    install_upgrade
    ;;

  *)
    exit 1
    ;;
esac

exit 0