#!/bin/sh -
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

if [ $# -eq 0 ]; then
	echo 'usage: shar file ...' 1>&2
	exit 64			# EX_USAGE
fi

for i
do
	if [ ! \( -d $i -o -r $i \) ]; then
		echo "$i inaccessible or not exist" 1>&2
		exit 66		# EX_NOINPUT
	fi
done

cat << EOF
# This is a shell archive.  Save it in a file, remove anything before
# this line, and then unpack it by entering "sh file".  Note, it may
# create directories; files and directories will be owned by you and
# have default permissions.
#
# This archive contains:
#
EOF

for i
do
	echo "#	$i"
done

echo "#"

for i
do
	if [ -d $i ]; then
		echo "echo c - $i"
		echo "mkdir -p $i > /dev/null 2>&1"
	else
		md5sum=`echo -n $i | md5`
		echo "echo x - $i"
		echo "sed 's/^X//' >$i << '$md5sum'"
		sed 's/^/X/' $i || exit
		echo "$md5sum"
	fi
done
echo exit
echo ""

exit 0