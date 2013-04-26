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
# Attempt to generate a similar HTML file akin to vop_table.tcl output.
# This is all fairly poor perl code... but whatever, will be cleaned up
# in the near future.
#
# (c) 2004 Andrew R. Reiter <arr@watson.org>
# All Rights Reserved.
#
# $FreeBSD$
# XXX todo: Make $src_dir modificationable

$src_dir = "/usr/src";
$srcsys_dir = $src_dir."/sys";
$vnode_if_awk = $srcsys_dir."/tools/vnode_if.awk";
$vnode_if_src = $srcsys_dir."/kern/vnode_if.src";

# Temporary input file... generated by a find(1) based command.
$infile = "/tmp/vt.$$";

# Output HTML
$outfile = "vop_vectors.html";

# Get a bunch of the vop_ routines out of this file.
`awk -f $vnode_if_awk $vnode_if_src -q`;
if ($?) {
	print "Incapable of writing out the typedef file.\n";
	exit(1);
}

# Eat the typedefs file into memory 
open TD, "vnode_if_typedef.h" || die "Unable to open typedef file: $!\n";
@vop_tdefs = <TD>;
close TD;
`rm -f vnode_if_typedef.h`;

# List of available vnode operations
@available_vn_ops = ();
push @available_vn_ops, "default";
foreach $vt (@vop_tdefs) {
	if ($vt =~ m/typedef\s+(\S+)\s+vop_(\S+)_t\(/) {
		push @available_vn_ops, $2;
	}
}
# Alpha sort.
@available_vn_ops = sort(@available_vn_ops);

# Array of { file , fs }
@fs = ();
$#fs = 0;

# Array of available vops for a given fs
@fsvnops = ();
$#fsvnops = 0;

#Begin output.
open OF, ">$outfile";
print OF "<HTML><HEAD><TITLE>File system vnode Operations</TITLE>";
print OF "</HEAD><BODY>\n";
print OF "<TABLE BORDER WIDTH=\"100%\" HEIGHT=\"100%\" >\n";
print OF "<TR><TH><font size=-1>F<br>i<br>l<br>e<br><br>S<br>y<br>s<br>t<br>e<br>m<br></font>\n";
print OF "<TH><font size=-1>V<br>e<br>c<br>t<br>o<br>r<br><br>V<br>a<br>r<br>i<br>a<br>b<br>l<br>e<br></font>\n";
foreach $avops (@available_vn_ops) {
	@ao = split //,$avops;
	print OF "<TH><font size=-1> ";
	foreach $aoz (@ao) {
		print OF "$aoz<br>\n";
	}
	print OF "</font>\n";
}
print OF "</TR></font>\n";

#Generate $infile; sketchy
`find /usr/src/sys -type f -iname *.c -print | xargs grep 'vnodeops =' > $infile`;

# Read in the file we find(1) generated.
open VT, $infile;
foreach $l (<VT>) {
	chomp($l);

	# Attempt to find all the files holding vop_vector declarations
	
	##
	# Need to sort based on $fs[]->{fs}
	if ($l =~ m/^(\S+):.*vop_vector ((\S+)_vnodeops) =/) {
		# Eh, I suck at perl; forgot some syntax, so hack.
		$sz = $#fs;
		$#fs++;
		$fs[$sz]->{file} = $1;
		$fs[$sz]->{fs} = $3;	# file system 
		$fs[$sz]->{fsvar} = $2;	# fs vop vector variable
		push @fs_s, $3;
		next;
	}
}
close VT;
`rm -f $infile`;
@fs_s = sort  @fs_s;
$m = 0;
foreach $fss (@fs_s) {
	for ($l = 0; $l < $#fs; $l++) {
		if ($fs_s[$m] eq $fs[$l]->{fs}) {
			$fs[$l]->{fsi} = $m;
			$m++;
			last;
		}
	}
}

sub vop_fn_val {
	($vfn) = @_;
	$ret = "<TD BGCOLOR=\"#00ddd0\"> ";

	if ($vfn eq "VOP_EBADF") {
		$ret .= "b";
		return $ret;
	}	
	if ($vfn eq "VOP_PANIC") {
		$ret .= "!";
		return $ret;
	}	
	if ($vfn eq "VOP_NULL") {
		$ret .= "-";
		return $ret;
	}	
	if ($vfn =~ m/vop_std/) {
		$ret .= "S";
		return $ret;
	}
	if ($vfn eq "vop_cache_lookup") {
		$ret .= "C";
		return $ret;
	}
	if ($vfn =~ m/vop_no/) {
		$ret .= "N";
		return $ret;
	}
	if ($vfn =~ m/vop_/) {
		$ret .= "V";
		return $ret;
	}
	if ($vfn eq "VOP_EINVAL") {
		$ret .= "i";
		return $ret;
	}
	if ($vfn eq "VOP_ENOTTY") {
		$ret .= "t";
		return $ret;
	}
	if ($vfn eq "VOP_ENOTOPSUPP") {
		$ret .= "*";
		return $ret;
	}
	if ($vfn =~ m/_lookup/) {
		$ret .= "L";
		return $ret;
	}
	$ret .= "F";
	return $ret;
}

# Loop through files that define vop_vectors.
for ($j = 0; $j < $#fs; $j++) {

	# Open the file containing the vop_vector decl.
	open V, $fs[$j]->{file};
	@VF = <V>;
	close V;

	# Zero out array; is there an easier way to do this in perl?
	for ($z = 0; $z < $#fsvnops; $z++) {
		$fsvnops[$z] = "<td bgcolor=\"d0d0d0\">";
	}

	$opz = 0;
	foreach $v (@VF) {
		chomp($v);

		if ($v =~ m/vop_vector (\S+_vnodeops) =/) {
			$opz = 1;
			next;
		}

		next if ($opz == 0);

		# Ehhh...	hax
		if ($v =~ m/;/) {
			$opz = 0;
			next;
		}	

		# ... Generate a vop_vector under each @fs entry?
		#  vop_LABEL = <func>		
		if ($v =~ m/\.vop_(\S+)\s+=\s+(\S+),/) {
			$iz = get_vop_fn_index($1);
			if ($iz == -1) {
				print "Unknown vnop routine: $1:$2 \n";
				exit 1;
			}
			$fsvnops[$iz] = vop_fn_val($2);
		}		
	}
	# 
	$fs[$j]->{out} = " <TD> $fs[$j]->{fsvar}";

	# Print out based on fsvnops
	foreach $z (@fsvnops) {
		$fs[$j]->{out} .= $z;
	}
	$fs[$j]->{out} .= "</TR>\n";
}

foreach $s (@fs_s) {
	for ($b = 0; $b < $#fs; $b++) {
		if ($s eq $fs[$b]->{fs}) {
			print OF "<TD> $s ".$fs[$b]->{out};
		}
	}
}


print OF "</TR></TABLE>\n";
print OF "<PRE>\n";
print OF "
C  vfs_cache_lookup
*  vop_defaultop
b  vop_ebadf
i  vop_einval
t  vop_enotty
*  vop_eopnotsupp
-  vop_null
!  vop_panic
F  _
V  vop_
N  vop_no
S  vop_std
L  _lookup
</PRE>";
print OF "</BODY></HTML>\n";
close OF;
exit 0;

sub get_vop_fn_index {
	($vop_fn) = @_;
	$r = 0;

	foreach $y (@available_vn_ops) {
		if ($y eq $vop_fn) {
			return $r;
		}
		$r++;
	}
	return -1;
}