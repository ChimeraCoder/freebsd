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

usage() {
	echo "usage: ssh-copy-id [-l] [-i keyfile] [-o option] [-p port] [user@]hostname" >&2
	exit 1
}

sendkey() {
	local h="$1"
	local k="$2"
	printf "%s\n" "$k" | ssh $port -S none $options "$user$h" /bin/sh -c \'' \
		set -e; \
		umask 077; \
		keyfile=$HOME/.ssh/authorized_keys ; \
		mkdir -p -- "$HOME/.ssh/" ; \
		while read alg key comment ; do \
			[ -n "$key" ] || continue; \
			if ! grep -sqwF "$key" "$keyfile"; then \
				printf "$alg $key $comment\n" >> "$keyfile" ; \
			fi ; \
		done \
	'\' 
}

agentKeys() {
	keys="$(ssh-add -L | grep -v 'The agent has no identities.')$nl$keys"
}

keys=""
host=""
hasarg=""
user=""
port=""
nl="
"
options=""

IFS=$nl

while getopts 'i:lo:p:' arg; do
	case $arg in
	i)	
		hasarg="x"
		if [ -r "$OPTARG" ]; then
			keys="$(cat -- "$OPTARG")$nl$keys"
		else
			echo "File $OPTARG not found" >&2
			exit 1
		fi
		;;
	l)	
		hasarg="x"
		agentKeys
		;;
	p)	
		port=-p$nl$OPTARG
		;;
	o)	
		options=$options$nl-o$nl$OPTARG
		;;
	*)	
		usage
		;;
	esac
done >&2

shift $((OPTIND-1))

if [ -z "$hasarg" ]; then
	agentKeys
fi
if [ -z "$keys" ] || [ "$keys" = "$nl" ]; then
	echo "no keys found" >&2
	exit 1
fi
if [ "$#" -eq 0 ]; then
	usage
fi

for host in "$@"; do
	sendkey "$host" "$keys"
done