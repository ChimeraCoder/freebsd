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

usage()
{
	echo "usage: crashinfo [-d crashdir] [-n dumpnr] [-k kernel] [core]"
	exit 1
}

find_kernel()
{
	local ivers k kvers

	ivers=$(awk '
	/Version String/ {
		print
		nextline=1
		next
	}
	nextline==1 {
		if ($0 ~ "^  [A-Za-z ]+: ") {
			nextline=0
		} else {
			print
		}
	}' $INFO)

	# Look for a matching kernel version.
	for k in `sysctl -n kern.bootfile` $(ls -t /boot/*/kernel); do
		kvers=$(echo 'printf "  Version String: %s", version' | \
		    gdb -x /dev/stdin -batch $k 2>/dev/null)
		if [ "$ivers" = "$kvers" ]; then
			KERNEL=$k
			break
		fi
	done
}

CRASHDIR=/var/crash
DUMPNR=
KERNEL=

while getopts "d:n:k:" opt; do
	case "$opt" in
	d)
		CRASHDIR=$OPTARG
		;;
	n)
		DUMPNR=$OPTARG
		;;
	k)
		KERNEL=$OPTARG
		;;
	\?)
		usage
		;;
	esac
done

shift $((OPTIND - 1))

if [ $# -eq 1 ]; then
	if [ -n "$DUMPNR" ]; then
		echo "-n and an explicit vmcore are mutually exclusive"
		usage
	fi

	# Figure out the crash directory and number from the vmcore name.
	CRASHDIR=`dirname $1`
	DUMPNR=$(expr $(basename $1) : 'vmcore\.\([0-9]*\)$')
	if [ -z "$DUMPNR" ]; then
		echo "Unable to determine dump number from vmcore file $1."
		exit 1
	fi
elif [ $# -gt 1 ]; then
	usage
else
	# If we don't have an explicit dump number, operate on the most
	# recent dump.
	if [ -z "$DUMPNR" ]; then
		if ! [ -r $CRASHDIR/bounds ]; then
			echo "No crash dumps in $CRASHDIR."
			exit 1
		fi			
		next=`cat $CRASHDIR/bounds`
		if [ -z "$next" ] || [ "$next" -eq 0 ]; then
			echo "No crash dumps in $CRASHDIR."
			exit 1
		fi
		DUMPNR=$(($next - 1))
	fi
fi

VMCORE=$CRASHDIR/vmcore.$DUMPNR
INFO=$CRASHDIR/info.$DUMPNR
FILE=$CRASHDIR/core.txt.$DUMPNR
HOSTNAME=`hostname`

if [ ! -e $VMCORE ]; then
	echo "$VMCORE not found"
	exit 1
fi

if [ ! -e $INFO ]; then
	echo "$INFO not found"
	exit 1
fi

# If the user didn't specify a kernel, then try to find one.
if [ -z "$KERNEL" ]; then
	find_kernel
	if [ -z "$KERNEL" ]; then
		echo "Unable to find matching kernel for $VMCORE"
		exit 1
	fi
elif [ ! -e $KERNEL ]; then
	echo "$KERNEL not found"
	exit 1
fi

echo "Writing crash summary to $FILE."

umask 077

# Simulate uname
ostype=$(echo -e printf '"%s", ostype' | gdb -x /dev/stdin -batch $KERNEL)
osrelease=$(echo -e printf '"%s", osrelease' | gdb -x /dev/stdin -batch $KERNEL)
version=$(echo -e printf '"%s", version' | gdb -x /dev/stdin -batch $KERNEL | \
    tr '\t\n' '  ')
machine=$(echo -e printf '"%s", machine' | gdb -x /dev/stdin -batch $KERNEL)

exec > $FILE 2>&1

echo "$HOSTNAME dumped core - see $VMCORE"
echo
date
echo
echo "$ostype $HOSTNAME $osrelease $version $machine"
echo
sed -ne '/^  Panic String: /{s//panic: /;p;}' $INFO
echo

# XXX: /bin/sh on 7.0+ is broken so we can't simply pipe the commands to
# kgdb via stdin and have to use a temporary file instead.
file=`mktemp /tmp/crashinfo.XXXXXX`
if [ $? -eq 0 ]; then
	echo "bt" >> $file
	echo "quit" >> $file
	kgdb $KERNEL $VMCORE < $file
	rm -f $file
	echo
fi
echo

echo "------------------------------------------------------------------------"
echo "ps -axl"
echo
ps -M $VMCORE -N $KERNEL -axl
echo

echo "------------------------------------------------------------------------"
echo "vmstat -s"
echo
vmstat -M $VMCORE -N $KERNEL -s
echo

echo "------------------------------------------------------------------------"
echo "vmstat -m"
echo
vmstat -M $VMCORE -N $KERNEL -m
echo

echo "------------------------------------------------------------------------"
echo "vmstat -z"
echo
vmstat -M $VMCORE -N $KERNEL -z
echo

echo "------------------------------------------------------------------------"
echo "vmstat -i"
echo
vmstat -M $VMCORE -N $KERNEL -i
echo

echo "------------------------------------------------------------------------"
echo "pstat -T"
echo
pstat -M $VMCORE -N $KERNEL -T
echo

echo "------------------------------------------------------------------------"
echo "pstat -s"
echo
pstat -M $VMCORE -N $KERNEL -s
echo

echo "------------------------------------------------------------------------"
echo "iostat"
echo
iostat -M $VMCORE -N $KERNEL
echo

echo "------------------------------------------------------------------------"
echo "ipcs -a"
echo
ipcs -C $VMCORE -N $KERNEL -a
echo

echo "------------------------------------------------------------------------"
echo "ipcs -T"
echo
ipcs -C $VMCORE -N $KERNEL -T
echo

# XXX: This doesn't actually work in 5.x+
if false; then
echo "------------------------------------------------------------------------"
echo "w -dn"
echo
w -M $VMCORE -N $KERNEL -dn
echo
fi

echo "------------------------------------------------------------------------"
echo "nfsstat"
echo
nfsstat -M $VMCORE -N $KERNEL
echo

echo "------------------------------------------------------------------------"
echo "netstat -s"
echo
netstat -M $VMCORE -N $KERNEL -s
echo

echo "------------------------------------------------------------------------"
echo "netstat -m"
echo
netstat -M $VMCORE -N $KERNEL -m
echo

echo "------------------------------------------------------------------------"
echo "netstat -id"
echo
netstat -M $VMCORE -N $KERNEL -id
echo

echo "------------------------------------------------------------------------"
echo "netstat -anr"
echo
netstat -M $VMCORE -N $KERNEL -anr
echo

echo "------------------------------------------------------------------------"
echo "netstat -anA"
echo
netstat -M $VMCORE -N $KERNEL -anA
echo

echo "------------------------------------------------------------------------"
echo "netstat -aL"
echo
netstat -M $VMCORE -N $KERNEL -aL
echo

echo "------------------------------------------------------------------------"
echo "fstat"
echo
fstat -M $VMCORE -N $KERNEL
echo

echo "------------------------------------------------------------------------"
echo "dmesg"
echo
dmesg -a -M $VMCORE -N $KERNEL
echo

echo "------------------------------------------------------------------------"
echo "kernel config"
echo
config -x $KERNEL

echo
echo "------------------------------------------------------------------------"
echo "ddb capture buffer"
echo

ddb capture -M $VMCORE -N $KERNEL print