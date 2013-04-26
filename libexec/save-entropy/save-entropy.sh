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

# This script is called by cron to store bits of randomness which are
# then used to seed /dev/random on boot.

# Originally developed by Doug Barton, dougb@FreeBSD.org

PATH=/bin:/usr/bin

# If there is a global system configuration file, suck it in.
#
if [ -r /etc/defaults/rc.conf ]; then
	. /etc/defaults/rc.conf
	source_rc_confs 2>/dev/null
elif [ -r /etc/rc.conf ]; then
	. /etc/rc.conf 2>/dev/null
fi

case ${entropy_dir} in
[Nn][Oo])
	exit 0
	;;
*)
	entropy_dir=${entropy_dir:-/var/db/entropy}
	;;
esac

entropy_save_sz=${entropy_save_sz:-2048}
entropy_save_num=${entropy_save_num:-8}

if [ ! -d "${entropy_dir}" ]; then
	install -d -o operator -g operator -m 0700 "${entropy_dir}" || {
		logger -is -t "$0" The entropy directory "${entropy_dir}" does \
		    not exist, and cannot be created. Therefore no entropy can \
		    be saved.; exit 1; }
fi

cd "${entropy_dir}" || {
	logger -is -t "$0" Cannot cd to the entropy directory: "${entropy_dir}". \
	    Entropy file rotation is aborted.; exit 1; }

for f in saved-entropy.*; do
	case "${f}" in saved-entropy.\*) continue ;; esac	# No files match
	[ ${f#saved-entropy\.} -ge ${entropy_save_num} ] && unlink ${f}
done

umask 377

n=$(( ${entropy_save_num} - 1 ))
while [ ${n} -ge 1 ]; do
	if [ -f "saved-entropy.${n}" ]; then
		mv "saved-entropy.${n}" "saved-entropy.$(( ${n} + 1 ))"
	elif [ -e "saved-entropy.${n}" -o -L "saved-entropy.${n}" ]; then
		logger -is -t "$0" \
	"${entropy_dir}/saved-entropy.${n}" is not a regular file, and so \
	    it will not be rotated. Entropy file rotation is aborted.
		exit 1
	fi
	n=$(( ${n} - 1 ))
done

dd if=/dev/random of=saved-entropy.1 bs=${entropy_save_sz} count=1 2>/dev/null

exit 0