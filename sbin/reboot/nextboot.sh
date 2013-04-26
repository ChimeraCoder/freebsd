#! /bin/sh
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

delete="NO"
kenv=
force="NO"
nextboot_file="/boot/nextboot.conf"

add_kenv()
{
	local var value

	var=$1
	# strip literal quotes if passed in
	value=${2%\"*}
	value=${value#*\"}

	if [ -n "${kenv}" ]; then
		kenv="${kenv}
"
	fi
	kenv="${kenv}${var}=\"${value}\""
}

display_usage() {
	echo "Usage: nextboot [-e variable=value] [-f] [-k kernel] [-o options]"
	echo "       nextboot -D"
}

while getopts "De:fk:o:" argument ; do
	case "${argument}" in
	D)
		delete="YES"
		;;
	e)
		var=${OPTARG%%=*}
		value=${OPTARG#*=}
		if [ -z "$var" -o -z "$value" ]; then
			display_usage
			exit 1
		fi
		add_kenv "$var" "$value"
		;;
	f)
		force="YES"
		;;
	k)
		kernel="${OPTARG}"
		add_kenv kernel "$kernel"
		;;
	o)
		add_kenv kernel_options "${OPTARG}"
		;;
	*)
		display_usage
		exit 1
		;;
	esac
done

if [ ${delete} = "YES" ]; then
	rm -f ${nextboot_file}
	exit 0
fi

if [ -z "${kenv}" ]; then
	display_usage
	exit 1
fi

if [ -n "${kernel}" -a ${force} = "NO" -a ! -d /boot/${kernel} ]; then
	echo "Error: /boot/${kernel} doesn't exist. Use -f to override."
	exit 1
fi

df -Tn "/boot/" 2>/dev/null | while read _fs _type _other ; do
	[ "zfs" = "${_type}" ] || continue
	cat 1>&2 <<-EOF
		WARNING: loader(8) has only R/O support for ZFS
		nextboot.conf will NOT be reset in case of kernel boot failure
	EOF
done

cat > ${nextboot_file} << EOF
nextboot_enable="YES"
$kenv
EOF