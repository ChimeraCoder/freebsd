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
. ${BACKEND}/functions-ftp.sh


get_package_index_by_ftp()
{
  local INDEX_FILE
  local FTP_SERVER

  FTP_SERVER="${1}"
  INDEX_FILE="INDEX"
  USE_BZIP2=0

  if [ -f "/usr/bin/bzip2" ]
  then
    INDEX_FILE="${INDEX_FILE}.bz2"
    USE_BZIP2=1
    INDEX_PATH="${INDEXFILE}.bz2"
  else
    INDEX_PATH="${INDEXFILE}"
  fi

  fetch_file "${FTP_SERVER}/${INDEX_FILE}" "${INDEX_PATH}" "1"
  if [ -f "${INDEX_PATH}" ] && [ "${USE_BZIP2}" -eq "1" ]
  then
    bzip2 -d "${INDEX_PATH}"
  fi
};

get_package_index_by_fs()
{
  if [ "$INSTALLMEDIUM" = "local" ] ; then
    INDEXFILE="${LOCALPATH}/packages/INDEX"
  else
    INDEXFILE="${CDMNT}/packages/INDEX"
  fi
};

get_package_index_size()
{
  if [ -f "${INDEXFILE}" ]
  then
    SIZE=`ls -l ${INDEXFILE} | awk '{ print $5 }'`
  else
    get_ftp_mirror
    FTPHOST="${VAL}"

    FTPDIR="/pub/FreeBSD/releases/${FBSD_ARCH}/${FBSD_BRANCH}"
    FTPPATH="ftp://${FTPHOST}${FTPDIR}/packages"

    fetch -s "${FTPPATH}/INDEX.bz2"
  fi
};

get_package_index()
{
  RES=0

  if [ -z "${INSTALLMODE}" ]
  then
    get_ftp_mirror
    FTPHOST="${VAL}"

    FTPDIR="/pub/FreeBSD/releases/${FBSD_ARCH}/${FBSD_BRANCH}"
    FTPPATH="ftp://${FTPHOST}${FTPDIR}/packages"

    get_package_index_by_ftp "${FTPPATH}"

  else

    case "${INSTALLMEDIUM}" in
    usb|dvd|local) get_package_index_by_fs ;;
              ftp) get_value_from_cfg ftpHost
    		   if [ -z "$VAL" ]; then
      		     exit_err "ERROR: Install medium was set to ftp, but no ftpHost was provided!" 
    	  	   fi
    		   FTPHOST="${VAL}"

    		   get_value_from_cfg ftpDir
    		   if [ -z "$VAL" ]; then
      		     exit_err "ERROR: Install medium was set to ftp, but no ftpDir was provided!" 
    		   fi
    	 	   FTPDIR="${VAL}"
    		   FTPPATH="ftp://${FTPHOST}${FTPDIR}"
                   get_package_index_by_ftp "${FTPPATH}" ;;
             sftp) ;;
                *) RES=1 ;;
    esac

  fi

  return ${RES}
};

parse_package_index()
{
  echo_log "Building package dep list.. Please wait.."
  INDEX_FILE="${PKGDIR}/INDEX"

  exec 3<&0
  exec 0<"${INDEXFILE}"

  while read -r line
  do
    PKGNAME=""
    CATEGORY=""
    PACKAGE=""
    DESC=""
    DEPS=""
    i=0

    SAVE_IFS="${IFS}"
    IFS="|"

    for part in ${line}
    do
      if [ ${i} -eq 0 ]
      then
        PKGNAME="${part}"

      elif [ ${i} -eq 1 ]
      then
        PACKAGE=`basename "${part}"`

      elif [ ${i} -eq 3 ]
      then
        DESC="${part}"

      elif [ ${i} -eq 6 ]
      then
        CATEGORY=`echo "${part}" | cut -f1 -d' '`

      elif [ ${i} -eq 8 ]
      then
        DEPS="${part}"
      fi

      i=$((i+1))
    done

    echo "${CATEGORY}|${PACKAGE}|${DESC}" >> "${INDEX_FILE}.parsed"
    echo "${PACKAGE}|${PKGNAME}|${DEPS}" >> "${INDEX_FILE}.deps"

    IFS="${SAVE_IFS}"
  done

  exec 0<&3
};

show_package_file()
{
  PKGFILE="${1}"

  echo "Available Packages:"

  exec 3<&0
  exec 0<"${PKGFILE}"

  while read -r line
  do
    CATEGORY=`echo "${line}" | cut -f1 -d'|'`
    PACKAGE=`echo "${line}" | cut -f2 -d'|'`
    DESC=`echo "${line}" | cut -f3 -d'|'`

    echo "${CATEGORY}/${PACKAGE}:${DESC}"
  done

  exec 0<&3
};

show_packages_by_category()
{
  CATEGORY="${1}"
  INDEX_FILE="${PKGDIR}/INDEX.parsed"
  TMPFILE="/tmp/.pkg.cat"

  grep "^${CATEGORY}|" "${INDEX_FILE}" > "${TMPFILE}"
  show_package_file "${TMPFILE}"
  rm "${TMPFILE}"
};

show_package_by_name()
{
  CATEGORY="${1}"
  PACKAGE="${2}"
  INDEX_FILE="${PKGDIR}/INDEX.parsed"
  TMPFILE="/tmp/.pkg.cat.pak"

  grep "^${CATEGORY}|${PACKAGE}" "${INDEX_FILE}" > "${TMPFILE}"
  show_package_file "${TMPFILE}"
  rm "${TMPFILE}"
};

show_packages()
{
  show_package_file "${PKGDIR}/INDEX.parsed"
};

get_package_dependencies()
{
  PACKAGE="${1}"
  LONG="${2:-0}"
  RES=0

  INDEX_FILE="${PKGDIR}/INDEX.deps"
  REGEX="^${PACKAGE}|"

  if [ ${LONG} -ne 0 ]
  then
    REGEX="^.*|${PACKAGE}|"
  fi

  LINE=`grep "${REGEX}" "${INDEX_FILE}" 2>/dev/null`
  DEPS=`echo "${LINE}"|cut -f3 -d'|'`

  export VAL="${DEPS}"

  if [ -z "${VAL}" ]
  then
    RES=1
  fi

  return ${RES}
};

get_package_name()
{
  PACKAGE="${1}"
  RES=0
  local PKGPTH

  # If we are on a local medium, we can parse the Latest/ directory
  if [ "${INSTALLMEDIUM}" != "ftp" ] ; then
    case "${INSTALLMEDIUM}" in
      usb|dvd) PKGPTH="${CDMNT}/packages" ;; 
        *) PKGPTH="${LOCALPATH}/packages" ;;
    esac
    
    # Check the /Latest dir for generic names, then look for specific version in All/
    if [ -e "${PKGPTH}/Latest/${PACKAGE}.${PKGEXT}" ] ; then
       NAME=`ls -al ${PKGPTH}/Latest/${PACKAGE}.${PKGEXT} 2>/dev/null | cut -d '>' -f 2 | rev | cut -f1 -d'/' | rev | tr -s ' '`
    else
       NAME=`ls -al ${PKGPTH}/All/${PACKAGE}.${PKGEXT} 2>/dev/null | cut -d '>' -f 2 | rev | cut -f1 -d'/' | rev | tr -s ' '`
    fi
    export VAL="${NAME}"
  else
    # Doing remote fetch, we we will look up, but some generic names like
    # "perl" wont work, since we don't know the default version
    INDEX_FILE="${PKGDIR}/INDEX.deps"
    REGEX="^${PACKAGE}|"
	
    LINE=`grep "${REGEX}" "${INDEX_FILE}" 2>/dev/null`
    NAME=`echo "${LINE}"|cut -f2 -d'|'`

    export VAL="${NAME}"
  fi

  if [ -z "${VAL}" ]
  then
    RES=1
  fi
  return ${RES}
};

get_package_short_name()
{
  PACKAGE="${1}"
  RES=0

  INDEX_FILE="${PKGDIR}/INDEX.deps"
  REGEX="^.*|${PACKAGE}|"
	
  LINE=`grep "${REGEX}" "${INDEX_FILE}" 2>/dev/null`
  NAME=`echo "${LINE}"|cut -f1 -d'|'`

  export VAL="${NAME}"

  if [ -z "${VAL}" ]
  then
    RES=1
  fi

  return ${RES}
};

get_package_category()
{
  PACKAGE="${1}"
  INDEX_FILE="${PKGDIR}/INDEX.parsed"
  RES=0

  LINE=`grep "|${PACKAGE}|" "${INDEX_FILE}" 2>/dev/null`
  NAME=`echo "${LINE}"|cut -f1 -d'|'`

  export VAL="${NAME}"

  if [ -z "${VAL}" ]
  then
    RES=1
  fi

  return ${RES}
};

fetch_package_by_ftp()
{
  CATEGORY="${1}"
  PACKAGE="${2}"
  SAVEDIR="${3}"

  get_value_from_cfg ftpHost
  if [ -z "$VAL" ]
  then
    exit_err "ERROR: Install medium was set to ftp, but no ftpHost was provided!" 
  fi
  FTPHOST="${VAL}"

  get_value_from_cfg ftpDir
  if [ -z "$VAL" ]
  then
    exit_err "ERROR: Install medium was set to ftp, but no ftpDir was provided!" 
  fi
  FTPDIR="${VAL}"

  PACKAGE="${PACKAGE}.${PKGEXT}"
  FTP_SERVER="ftp://${FTPHOST}${FTPDIR}"

  if [ ! -f "${SAVEDIR}/${PACKAGE}" ]
  then
    PKGPATH="${CATEGORY}/${PACKAGE}"
    FTP_PATH="${FTP_HOST}/packages/${PKGPATH}"
    fetch_file "${FTP_PATH}" "${SAVEDIR}/" "0"
  fi
};

fetch_package()
{
  CATEGORY="${1}"
  PACKAGE="${2}"
  SAVEDIR="${3}"

  # Fetch package, but skip if installing from local media
  case "${INSTALLMEDIUM}" in
  usb|dvd|local) return ;;
    ftp) fetch_package_by_ftp "${CATEGORY}" "${PACKAGE}" "${SAVEDIR}" ;;
    sftp) ;;
  esac
};

bootstrap_pkgng()
{
  # Check if we need to boot-strap pkgng
  if run_chroot_cmd "which pkg-static" >/dev/null 2>/dev/null
  then
     return
  fi
  local PKGPTH

  # Ok, lets boot-strap this sucker
  echo_log "Bootstraping pkgng.."
  fetch_package "Latest" "pkg" "${PKGDLDIR}"

  # Figure out real location of "pkg" package
  case "${INSTALLMEDIUM}" in
    usb|dvd|local) PKGPTH="${PKGTMPDIR}/Latest/pkg.${PKGEXT}" ;;
          *) PKGPTH="${PKGTMPDIR}/pkg.${PKGEXT}" ;;
  esac
  rc_halt "pkg -c ${FSMNT} add ${PKGPTH}" ; run_chroot_cmd "pkg2ng"
}

get_package_location()
{
  case "${INSTALLMEDIUM}" in
  usb|dvd) rc_halt "mount_nullfs ${CDMNT}/packages ${FSMNT}${PKGTMPDIR}"
           PKGDLDIR="${FSMNT}${PKGTMPDIR}/All" ;;
    local) rc_halt "mount_nullfs ${LOCALPATH}/packages ${FSMNT}${PKGTMPDIR}"
           PKGDLDIR="${FSMNT}${PKGTMPDIR}/All" ;;
        *) PKGDLDIR="${FSMNT}${PKGTMPDIR}" ;;
  esac
  export PKGDLDIR
}