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

## Helper functions
usage () {
	echo "Usage: $0 <dvd_img_name> <cd_img_name <cd_img_name ..>>"
}

# Copy data from the cd
# $1 os name
# $2 disk image name
# $3 mount dir
# $4 destination dir
copycd () {
	# Set some variables
	md=""
	_os="${1}"
	_img="${2}"
	_mnt="${3}"
	_dst="${4}"
	if [ $# -ne "4" ]
	then
		echo "Error: function ${0} takes exactly four parameters."
		exit 1
	fi
	if [ "${_os}" != "FreeBSD" -a "${_os}" != "Linux" ]
	then
		echo "Error: do not know how to handle ${_os} os."
		exit 1
	fi
	if [ ! -f "${_img}" ]
	then
		echo "Error: file ${_img} does not exists or not a regula file."
		exit 1
	fi
	if [ ! -r "${_img}" ]
	then
		echo "Error: you do not have the read permissions."
		exit 1
	fi
	if [ ! -d "${_mnt}" ]
	then
		echo "Error: ${_mnt} is not a directory or does not exists."
	fi
	if [ ! -d "${_dst}" ]
	then
		echo "Error: ${_dst} is not a directory or does not exists."
	fi
	if [ ! -w "${_dst}" ]
	then
		echo "Error: you do not have write permissions granted for ${_dst} directory."
	fi
	if [ "${_os}" != "Linux" ]
	then
		md=`mdconfig -a -t vnode -f ${_img}` || exit 1
		mount_cd9660 /dev/${md} ${_mnt} || exit 1
	else
		mount -o loop ${_img} ${_mnt} || exit 1
	fi
	if [ ! -f ${_mnt}/cdrom.inf ]
	then
		echo "Error: Failed to find cdrom.inf for ${_img}."
		exit 1
	fi
	cdvol=`grep "^CD_VOLUME.*" ${_mnt}/cdrom.inf | sed "s/CD_VOLUME[[:space:]]*=[[:space:]]*//"`
	if test -z "${cdvol}" -o ! "${cdvol}" -eq "${cdvol}" 2> /dev/null
	then
		echo "Error: failed to get volume id for ${_img}."
		exit 1
	fi
	cdver=`grep "^CD_VERSION.*" ${_mnt}/cdrom.inf | sed "s/CD_VERSION[[:space:]]*=[[:space:]]*//"`
	if test -z "${cdver}"
	then
		echo "Error: failed to get version id for ${_img}."
		exit 1
	fi
	if [ -z "${VERID}" ]
	then
		VERID="${cdver}"
		_exclude=""
	else
		if [ "${VERID}" != "${cdver}" ]
		then
			echo "Error: cd version ids mismatch while processing ${_img}."
			exit 1
		fi
#		_exclude="--exclude ./cdrom.inf --exclude ./packages/INDEX"
		_exclude="! -regex ./cdrom.inf ! -regex ./packages/INDEX"
	fi
	echo "Merging ${_img}:"
# --quite -u -V
	(cd "${_mnt}" && find . ${_exclude} | cpio -p -d -m -V --quiet "${_dst}") || exit 1
#	(cd "${_mnt}" && tar ${_exclude} -cvf - .) | (cd "${_dst}" && tar xvf -) || exit 1
	if [ "${_os}" != "Linux" ]
	then	
		umount /dev/${md} || exit 1
		mdconfig -d -u "${md}" || exit 1
	else
		umount ${_mnt} || exit 1
	fi
#	exit 0
}

# Clear mounted image
# $1 mounted directory
# $2 error code
clearmount ()
{
	if [ $# -ne "2" ]
	then
		echo "Error: function ${0} takes exactly two parameters."
		exit 1
	fi
	if [ -z "${1}" ]
	then
		test -z "${2}" || exit "${2}"
	else
		# Ignore errors
		umount "${1}" 2>/dev/null
		test -z "${2}" || exit "${2}"
	fi
}

# Clear CD image allocation
# $1 os name
# $2 md
# $3 error code
clearmd ()
{
	if [ $# -ne "3" ]
	then
		echo "Error: function ${0} takes exactly three parameters."
		exit 1
	fi
	if [ "${1}" != "FreeBSD" -o -z "${2}" ]
	then
		test -z "${3}" || exit "${3}"
	else
		# Ignore errors
		mdconfig -d -u "${2}" 2>/dev/null
		test -z "${3}" || exit "${3}"
	fi
}

## Check params
if [ $# -lt 3 ]
then
	usage
	echo "Error, this script should take more than two parameters."
	exit 1
fi

# Check if zero
if [ -z "${1}" ]; then
	usage
	exit 1
fi

# Check if already exists
if [ -e "${1}" ]; then
	if [ ! -f "${1}" ]; then
		echo "Destination DVD image file already exists and is not a regular file."
		exit 1
	fi
	while echo "The ${1} file exists. Overwrite? (y/n)"
	do
		read line
		case "${line}" in
		y|Y)
			rm -rf "${1}"
			touch "${1}"
			break
			;;
		n|N)
			echo "Please, run program again with a new value."
			exit 1
			;;
		esac
	done
fi
DVDIMAGE="${1}"

shift

count=0
for i in "$@"
do
	# Skip empty params.
	if test -z "${i}"; then
		continue
	fi
	if [ ! -f "${i}" -o ! -r "${i}" ]
	then
		echo "Error: The ${i} is not readable, do not exists or not a regular file."
		exit 1
	fi
	count=`expr ${count} \+ 1`
done

# Check if we have at the least two CD images
if [ "${count}" -lt 2 ]
then
	echo "Error: less than two CD images specified."
fi

## Some useful variables
pwd=`pwd`
tmpdirin="${pwd}/tmp-$$-in"
tmpdirout="${pwd}/tmp-$$-out"
system=`uname -s`
md=""

# set the trap options
trap 'echo ""; echo "Cleaning up"; clearmount "${tmpdirin}" ""; clearmd "${system}" "${md}" ""; rm -rf "${tmpdirin}" "${tmpdirout}";' 0 1 2 3 15
mkdir "${tmpdirin}" || (echo "Error: failed to create tempory ${tmpdirin}"; exit 1)
mkdir "${tmpdirout}" || (echo "Error: failed to create tempory ${tmpdirout}"; exit 1)

for i in "$@"
do
	# Skip empty params.
	if test -z "${i}"; then
		continue
	fi
	copycd "${system}" "${i}" "${tmpdirin}" "${tmpdirout}"
	mv "${tmpdirout}"/packages/INDEX "${tmpdirout}"/packages/INDEX~ || exit 1
	cat "${tmpdirout}"/packages/INDEX~ | sed "s/^\(.*\)|${cdvol}$/\1|1/" > "${tmpdirout}"/packages/INDEX || exit 1
	rm "${tmpdirout}"/packages/INDEX~ || exit 1
done

mv "${tmpdirout}"/cdrom.inf "${tmpdirout}"/cdrom.inf~ || exit 1
cat "${tmpdirout}"/cdrom.inf~ | sed "s/^\(CD_VOLUME[[:space:]]\{0,\}=[[:space:]]\{0,\}\)[[:digit:]]\{1,\}/\11/" > "${tmpdirout}"/cdrom.inf || exit 1
rm "${tmpdirout}"/cdrom.inf~ || exit 1

mkisofs -b boot/cdboot -no-emul-boot -r -J \
	-V "FreeBSD_Install" \
	-publisher "The FreeBSD Project.  http://www.freebsd.org/" \
	-o ${DVDIMAGE} "${tmpdirout}" \
	|| exit 1 

exit 0