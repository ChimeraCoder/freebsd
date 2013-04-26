#! @LOCALPREFIX@/bin/perl
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
# $FreeBSD$
if ($< != 0) {
	print STDERR "must be root to invoke this\n";
	exit 1;
}

$mode = 'add';
while ($i = shift @ARGV) {
	if ($i eq '-d') {
		$mode = 'delete';
	} else {
		print STDERR "usage: scriptdump [-d]\n";
		exit 1;
	}
}

open(IN, "setkey -D |") || die;
foreach $_ (<IN>) {
	if (/^[^\t]/) {
		($src, $dst) = split(/\s+/, $_);
	} elsif (/^\t(esp|ah) mode=(\S+) spi=(\d+).*reqid=(\d+)/) {
		($proto, $ipsecmode, $spi, $reqid) = ($1, $2, $3, $4);
	} elsif (/^\tE: (\S+) (.*)/) {
		$ealgo = $1;
		$ekey = $2;
		$ekey =~ s/\s//g;
		$ekey =~ s/^/0x/g;
	} elsif (/^\tA: (\S+) (.*)/) {
		$aalgo = $1;
		$akey = $2;
		$akey =~ s/\s//g;
		$akey =~ s/^/0x/g;
	} elsif (/^\tseq=(0x\d+) replay=(\d+) flags=(0x\d+) state=/) {
		print "$mode $src $dst $proto $spi";
		$replay = $2;
		print " -u $reqid" if $reqid;
		if ($mode eq 'add') {
			print " -m $ipsecmode -r $replay" if $replay;
			if ($proto eq 'esp') {
				print " -E $ealgo $ekey" if $ealgo;
				print " -A $aalgo $akey" if $aalgo;
			} elsif ($proto eq 'ah') {
				print " -A $aalgo $akey" if $aalgo;
			}
		} 
		print ";\n";

		$src = $dst = $upper = $proxy = '';
		$ealgo = $ekey = $aalgo = $akey = '';
	}
}
close(IN);

exit 0;