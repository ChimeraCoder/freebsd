#!/usr/bin/perl
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

#
# Read and decode a SCSI disk's primary or grown defect list.
#

sub usage
{
    die "usage: scsi-defects raw-device-name [Glist|Plist]\n";
}


#
# Main
#

&usage if $#ARGV < 0 || $#ARGV > 1;

$ENV{'PATH'} = "/bin:/usr/bin:/sbin:/usr/sbin";

$dev = $ARGV[0];

# generic device name given?
if ($dev =~ /^[so]d\d+$/) { $dev = "/dev/r${dev}.ctl"; }

#
# Select what you want to read.  PList include the primary defect list
# from the factory.  GList is grown defects only.
#
if ($#ARGV > 0) {
    if ($ARGV[1] =~ /^[Gg]/) { $glist = 1; $plist = 0; }
    elsif ($ARGV[1] =~ /^[Pp]/) { $glist = 0; $plist = 1; }
    else { &usage; }
} else {
    $glist = 1; $plist = 0;
}

open(PIPE, "scsi -f $dev " .
     "-c '{ Op code} 37 0 0:3 v:1 v:1 5:3 0 0 0 0 4:i2 0' $plist $glist " .
     "-i 4 '{ stuff } *i2 { Defect list length } i2' |") ||
    die "Cannot pipe to scsi(8)\n";
chop($amnt = <PIPE>);
close(PIPE);

if ($amnt == 0) {
    print "There are no defects (in this list).\n";
    exit 0;
}

print "There are " . $amnt / 8 . " defects in this list.\n";

$amnt += 4;

open(PIPE, "scsi -f $dev " .
     "-c '{ Op code} 37 0 0:3 v:1 v:1 5:3 0 0 0 0 v:i2 0' $plist $glist " .
     "$amnt -i $amnt - |") ||
    die "Cannot pipe to scsi(8)\n";

read(PIPE, $buf, 4);		# defect list header

print "cylinder head  sector\n";

while(read(PIPE, $buf, 8)) {
    ($cylhi, $cyllo, $head, $sec) = unpack("CnCN", $buf);
    printf "%8u %4u  %6u\n", $cylhi*65536+$cyllo, $head, $sec;
}
close(PIPE);