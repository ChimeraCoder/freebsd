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

# Functions related mounting the newly formatted disk partitions

# Mounts all the specified partition to the mount-point
mount_partition()
{
  if [ -z "${1}" -o -z "${2}" -o -z "${3}" ]
  then
    exit_err "ERROR: Missing arguments for mount_partition"
  fi

  PART="${1}"
  PARTFS="${2}"
  MNTPOINT="${3}"
  MNTFLAGS="${4}"

  # Setup the MNTOPTS
  if [ -z "${MNTOPTS}" ]
  then
    MNTFLAGS="-o rw"
  else
    MNTFLAGS="-o rw,${MNTFLAGS}"
  fi


  #We are on ZFS, lets setup this mount-point
  if [ "${PARTFS}" = "ZFS" ]
  then
    ZPOOLNAME=$(get_zpool_name "${PART}")

    # Check if we have multiple zfs mounts specified
    for ZMNT in `echo ${MNTPOINT} | sed 's|,| |g'`
    do
      # Check for any ZFS specific mount options
      ZMNTOPTS="`echo $ZMNT | cut -d '(' -f 2 | cut -d ')' -f 1`" 
      if [ "$ZMNTOPTS" = "$ZMNT" ] ; then ZMNTOPTS="" ; fi

      # Reset ZMNT with options removed
      ZMNT="`echo $ZMNT | cut -d '(' -f 1`"

      # First make sure we create the mount point
      if [ ! -d "${FSMNT}${ZMNT}" ] ; then
        mkdir -p ${FSMNT}${ZMNT} >>${LOGOUT} 2>>${LOGOUT}
      fi

      # Check for any volsize args
      zcopt=""
      for ZOPT in `echo $ZMNTOPTS | sed 's/|/ /g'`
      do
        echo "$ZOPT" | grep -q volsize
        if [ $? -eq 0 ] ; then
          volsize=`echo $ZOPT | cut -d '=' -f 2`
          zcopt="-V $volsize"
        fi
      done

      if [ "${ZMNT}" = "/" ] ; then
	# If creating ZFS / dataset, give it name that beadm works with
        ZNAME="/ROOT/default"
        ZMKMNT=""
        echo_log "zfs create $zcopt -p ${ZPOOLNAME}/ROOT"
        rc_halt "zfs create $zcopt -p ${ZPOOLNAME}/ROOT"
        echo_log "zfs create $zcopt -p ${ZPOOLNAME}${ZNAME}"
        rc_halt "zfs create $zcopt -p ${ZPOOLNAME}${ZNAME}"
      else
        ZNAME="${ZMNT}"
        ZMKMNT="${ZMNT}"
        echo_log "zfs create $zcopt -p ${ZPOOLNAME}${ZNAME}"
        rc_halt "zfs create $zcopt -p ${ZPOOLNAME}${ZNAME}"
      fi
      sleep 2
      if [ -z "$zcopt" ] ; then
        rc_halt "zfs set mountpoint=${FSMNT}${ZMKMNT} ${ZPOOLNAME}${ZNAME}"
      fi

      # Do we need to make this / zfs dataset bootable?
      if [ "$ZMNT" = "/" ] ; then
        echo_log "Stamping ${ZPOOLNAME}/ROOT/default as bootfs"
        rc_halt "zpool set bootfs=${ZPOOLNAME}/ROOT/default ${ZPOOLNAME}"
      fi

      # Do we need to make this /boot zfs dataset bootable?
      if [ "$ZMNT" = "/boot" ] ; then
        echo_log "Stamping ${ZPOOLNAME}${ZMNT} as bootfs"
        rc_halt "zpool set bootfs=${ZPOOLNAME}${ZMNT} ${ZPOOLNAME}"
      fi

      # If no ZFS options, we can skip
      if [ -z "$ZMNTOPTS" ] ; then continue ; fi

      # Parse any ZFS options now
      for ZOPT in `echo $ZMNTOPTS | sed 's/|/ /g'`
      do
        echo "$ZOPT" | grep -q volsize
        if [ $? -eq 0 ] ; then continue ; fi
        rc_halt "zfs set $ZOPT ${ZPOOLNAME}${ZNAME}"
      done
    done # End of adding ZFS mounts

  else
    # If we are not on ZFS, lets do the mount now
    # First make sure we create the mount point
    if [ ! -d "${FSMNT}${MNTPOINT}" ]
    then
      mkdir -p ${FSMNT}${MNTPOINT} >>${LOGOUT} 2>>${LOGOUT}
    fi

    echo_log "mount ${MNTFLAGS} ${PART} -> ${FSMNT}${MNTPOINT}"
    sleep 2
    rc_halt "mount ${MNTFLAGS} ${PART} ${FSMNT}${MNTPOINT}"
  fi

};

# Mounts all the new file systems to prepare for installation
mount_all_filesystems()
{
  # Make sure our mount point exists
  mkdir -p ${FSMNT} >/dev/null 2>/dev/null

  # First lets find and mount the / partition
  #########################################################
  for PART in `ls ${PARTDIR}`
  do
    PARTDEV=`echo $PART | sed 's|-|/|g'` 
    PARTFS="`cat ${PARTDIR}/${PART} | cut -d '#' -f 1`"
    if [ ! -e "${PARTDEV}" -a "${PARTFS}" != "ZFS" ]
    then
      exit_err "ERROR: The partition ${PARTDEV} does not exist. Failure in bsdlabel?"
    fi 

    PARTMNT="`cat ${PARTDIR}/${PART} | cut -d '#' -f 2`"
    PARTENC="`cat ${PARTDIR}/${PART} | cut -d '#' -f 3`"

    if [ "${PARTENC}" = "ON" ]
    then
      EXT=".eli"
    else
      EXT=""
    fi

    # Check for root partition for mounting, including ZFS "/,/usr" type 
    echo "$PARTMNT" | grep "/," >/dev/null
    if [ "$?" = "0" -o "$PARTMNT" = "/" ]
    then
      case ${PARTFS} in
        UFS) mount_partition ${PARTDEV}${EXT} ${PARTFS} ${PARTMNT} "noatime" ;;
        UFS+S) mount_partition ${PARTDEV}${EXT} ${PARTFS} ${PARTMNT} "noatime" ;;
        UFS+SUJ) mount_partition ${PARTDEV}${EXT} ${PARTFS} ${PARTMNT} "noatime" ;;
        UFS+J) mount_partition ${PARTDEV}${EXT}.journal ${PARTFS} ${PARTMNT} "async,noatime" ;;
        ZFS) mount_partition ${PARTDEV} ${PARTFS} ${PARTMNT} ;;
        IMAGE) mount_partition ${PARTDEV} ${PARTFS} ${PARTMNT} ;;
        *) exit_err "ERROR: Got unknown file-system type $PARTFS" ;;
      esac
    fi
  done

  # Now that we've mounted "/" lets do any other remaining mount-points
  ##################################################################
  for PART in `ls ${PARTDIR}`
  do
    PARTDEV=`echo $PART | sed 's|-|/|g'`
    PARTFS="`cat ${PARTDIR}/${PART} | cut -d '#' -f 1`"
    if [ ! -e "${PARTDEV}" -a "${PARTFS}" != "ZFS" ]
    then
      exit_err "ERROR: The partition ${PARTDEV} does not exist. Failure in bsdlabel?"
    fi 
     
    PARTMNT="`cat ${PARTDIR}/${PART} | cut -d '#' -f 2`"
    PARTENC="`cat ${PARTDIR}/${PART} | cut -d '#' -f 3`"

    if [ "${PARTENC}" = "ON" ]
    then
      EXT=".eli"
    else
      EXT=""
    fi

    # Check if we've found "/" again, don't need to mount it twice
    echo "$PARTMNT" | grep "/," >/dev/null
    if [ "$?" != "0" -a "$PARTMNT" != "/" ]
    then
       case ${PARTFS} in
         UFS) mount_partition ${PARTDEV}${EXT} ${PARTFS} ${PARTMNT} "noatime" ;;
         UFS+S) mount_partition ${PARTDEV}${EXT} ${PARTFS} ${PARTMNT} "noatime" ;;
         UFS+SUJ) mount_partition ${PARTDEV}${EXT} ${PARTFS} ${PARTMNT} "noatime" ;;
         UFS+J) mount_partition ${PARTDEV}${EXT}.journal ${PARTFS} ${PARTMNT} "async,noatime" ;;
         ZFS) mount_partition ${PARTDEV} ${PARTFS} ${PARTMNT} ;;
         SWAP)
		   # Lets enable this swap now
           if [ "$PARTENC" = "ON" ]
           then
             echo_log "Enabling encrypted swap on ${PARTDEV}"
             rc_halt "geli onetime -d -e 3des ${PARTDEV}"
             sleep 5
             rc_halt "swapon ${PARTDEV}.eli"
           else
             echo_log "swapon ${PARTDEV}"
             sleep 5
             rc_halt "swapon ${PARTDEV}"
            fi
            ;;
         IMAGE)
           if [ ! -d "${PARTMNT}" ]
           then
             mkdir -p "${PARTMNT}" 
           fi 
           mount_partition ${PARTDEV} ${PARTFS} ${PARTMNT}
           ;;
         *) exit_err "ERROR: Got unknown file-system type $PARTFS" ;;
      esac
    fi
  done
};