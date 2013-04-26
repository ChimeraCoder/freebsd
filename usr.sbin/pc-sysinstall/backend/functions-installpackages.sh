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

# Functions which check and load any optional packages specified in the config

. ${BACKEND}/functions.sh
. ${BACKEND}/functions-parse.sh

# Recursively determine all dependencies for this package
determine_package_dependencies()
{
  local PKGNAME="${1}"
  local DEPFILE="${2}"

  grep -q "${PKGNAME}" "${DEPFILE}"
  if [ $? -ne 0 ]
  then
    echo "${PKGNAME}" >> "${DEPFILE}"
    get_package_dependencies "${PKGNAME}" "1"

    local DEPS="${VAL}"
    for d in ${DEPS}
    do
      determine_package_dependencies "${d}" "${DEPFILE}"
    done
  fi
};

# Fetch packages dependencies from a file
fetch_package_dependencies()
{
  local DEPFILE
  local DEPS
  local SAVEDIR

  DEPFILE="${1}"
  DEPS=`cat "${DEPFILE}"`
  SAVEDIR="${2}"

  for d in ${DEPS}
  do
    get_package_short_name "${d}"
    SNAME="${VAL}"

    get_package_category "${SNAME}"
    CATEGORY="${VAL}"

    fetch_package "${CATEGORY}" "${d}" "${SAVEDIR}"
  done
};

# Check for any packages specified, and begin loading them
install_packages()
{
  echo "Checking for packages to install..."
  sleep 2

  # First, lets check and see if we even have any packages to install
  get_value_from_cfg installPackages

  # Nothing to do?
  if [ -z "${VAL}" ]; then return; fi

  echo "Installing packages..."
  sleep 3

  local PKGPTH

  HERE=`pwd`
  rc_halt "mkdir -p ${FSMNT}${PKGTMPDIR}"

  # Determine the directory we will install packages from
  get_package_location
  rc_halt "cd ${PKGDLDIR}"

  # Set the location of the INDEXFILE
  INDEXFILE="${TMPDIR}/INDEX"

  if [ ! -f "${INDEXFILE}" ]; then
    get_package_index
  fi

  if [ ! -f "${TMPDIR}/INDEX.parsed" -a "$INSTALLMEDIUM" = "ftp" ]; then
    parse_package_index
  fi

  # What extension are we using for pkgs?
  PKGEXT="txz"
  get_value_from_cfg pkgExt
  if [ -n "${VAL}" ]; then 
     strip_white_space ${VAL}
     PKGEXT="$VAL"
  fi
  export PKGEXT
  
  # We dont want to be bothered with scripts asking questions
  PACKAGE_BUILDING=yes
  export PACKAGE_BUILDING

  # Lets start by cleaning up the string and getting it ready to parse
  get_value_from_cfg_with_spaces installPackages
  PACKAGES="${VAL}"
  echo_log "Packages to install: `echo $PACKAGES | wc -w | awk '{print $1}'`"
  for i in $PACKAGES
  do
    if ! get_package_name "${i}"
    then
      echo_log "Unable to locate package ${i}"
      continue
    fi

    PKGNAME="${VAL}"

    # Fetch package + deps, but skip if installing from local media
    if [ "${INSTALLMEDIUM}" = "ftp" ] ; then
      DEPFILE="${FSMNT}/${PKGTMPDIR}/.${PKGNAME}.deps"
      rc_nohalt "touch ${DEPFILE}"
      determine_package_dependencies "${PKGNAME}" "${DEPFILE}"
      fetch_package_dependencies "${DEPFILE}" "${FSMNT}/${PKGTMPDIR}"
    fi

    # Set package location
    case "${INSTALLMEDIUM}" in
      usb|dvd|local) PKGPTH="${PKGTMPDIR}/All/${PKGNAME}" ;;
                  *) PKGPTH="${PKGTMPDIR}/${PKGNAME}" ;;
    esac

    # See if we need to determine the package format we are working with
    if [ -z "${PKGINFO}" ] ; then
      tar tqf "${FSMNT}${PKGPTH}" '+MANIFEST' >/dev/null 2>/dev/null	
      if [ $? -ne 0 ] ; then
        PKGADD="pkg_add -C ${FSMNT}" 
        PKGINFO="pkg_info" 
      else
        PKGADD="pkg -c ${FSMNT} add"
        PKGINFO="pkg info"
        bootstrap_pkgng
      fi
    fi

    # If the package is not already installed, install it!
    if ! run_chroot_cmd "${PKGINFO} -e ${PKGNAME}" >/dev/null 2>/dev/null
    then
      echo_log "Installing package: ${PKGNAME}"
      rc_nohalt "${PKGADD} ${PKGPTH}"
    fi

    if [ "${INSTALLMEDIUM}" = "ftp" ] ; then
      rc_nohalt "rm ${DEPFILE}"
    fi

  done

  echo_log "Package installation complete!"

  # Cleanup after ourselves
  rc_halt "cd ${HERE}"
  if [ "${INSTALLMEDIUM}" = "ftp" ] ; then
    rc_halt "rm -rf ${FSMNT}${PKGTMPDIR}" >/dev/null 2>/dev/null
  else
    rc_halt "umount ${FSMNT}${PKGTMPDIR}" >/dev/null 2>/dev/null
    rc_halt "rmdir ${FSMNT}${PKGTMPDIR}" >/dev/null 2>/dev/null
  fi
};