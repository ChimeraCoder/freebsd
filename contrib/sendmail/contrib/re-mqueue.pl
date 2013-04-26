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

use Sys::Syslog;

$LOCK_EX = 2;
$LOCK_NB = 4;
$LOCK_UN = 8;

# Count arguments, exit if wrong in any way.
die "Usage: $0 [-d] queueA queueB seconds\n" if ($#ARGV < 2);

while ($_ = $ARGV[0], /^-/) {
    shift;
    last if /^--$/;
    /^-d/ && $debug++;
}

$queueA = shift;
$queueB = shift;
$age = shift;

die "$0: $queueA not a directory\n" if (! -d $queueA);
die "$0: $queueB not a directory\n" if (! -d $queueB);
die "$0: $age isn't a valid number of seconds for age\n" if ($age =~ /\D/);

# chdir to $queueA and read the directory.  When a df* file is found, stat it.
# If it's older than $age, lock the corresponding qf* file.  If the lock
# fails, give up and move on.  Once the lock is obtained, verify that files
# of the same name *don't* already exist in $queueB and move on if they do.
# Otherwise re-link the qf* and df* files into $queueB then release the lock.

chdir "$queueA" || die "$0: can't cd to $queueA: $!\n";
opendir (QA, ".") || die "$0: can't open directory $queueA for reading: $!\n";
@dfiles = grep(/^df/, readdir(QA));
$now = time();
($program = $0) =~ s,.*/,,;
&openlog($program, 'pid', 'mail');

# Loop through the dfiles
while ($dfile = pop(@dfiles)) {
    print "Checking $dfile\n" if ($debug);
    ($qfile = $dfile) =~ s/^d/q/;
    ($xfile = $dfile) =~ s/^d/x/;
    ($mfile = $dfile) =~ s/^df//;
    if (! -e $qfile || -z $qfile) {
	print "$qfile is gone or zero bytes - skipping\n" if ($debug);
	next;
    }

    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
     $atime,$mtime,$ctime,$blksize,$blocks) = stat($dfile);
    if (! defined $mtime) {
	print "$dfile is gone - skipping\n" if ($debug);
	next;
    }

    # Compare timestamps
    if (($mtime + $age) > $now) {
	printf ("%s is %d seconds old - skipping\n", $dfile, $now-$mtime) if ($debug);
	next;
    }

    # See if files of the same name already exist in $queueB
    if (-e "$queueB/$dfile") {
	print "$queueb/$dfile already exists - skipping\n" if ($debug);
	next;
    }
    if (-e "$queueB/$qfile") {
	print "$queueb/$qfile already exists - skipping\n" if ($debug);
	next;
    }
    if (-e "$queueB/$xfile") {
	print "$queueb/$xfile already exists - skipping\n" if ($debug);
	next;
    }

    # Try and lock qf* file
    unless (open(QF, ">>$qfile")) {
	print "$qfile: $!\n" if ($debug);
	next;
    }
    $retval = flock(QF, $LOCK_EX|$LOCK_NB) || ($retval = -1);
    if ($retval == -1) {
	print "$qfile already flock()ed - skipping\n" if ($debug);
	close(QF);
	next;
    }
    print "$qfile now flock()ed\n" if ($debug);

    # Check df* file again in case sendmail got in
    if (! -e $dfile) {
	print "$mfile sent - skipping\n" if ($debug);
	# qf* file created by ourselves at open? (Almost certainly)
	if (-z $qfile) {
	   unlink($qfile);
	}
	close(QF);
	next;
    }

    # Show time!  Do the link()s
    if (link("$dfile", "$queueB/$dfile") == 0) {
	$bang = $!;
	&syslog('err', 'link(%s, %s/%s): %s', $dfile, $queueB, $dfile, $bang);
	print STDERR "$0: link($dfile, $queueB/$dfile): $bang\n";
	exit (1);
    }
    if (link("$qfile", "$queueB/$qfile") == 0) {
	$bang = $!;
	&syslog('err', 'link(%s, %s/%s): %s', $qfile, $queueB, $qfile, $bang);
	print STDERR "$0: link($qfile, $queueB/$qfile): $bang\n";
	unlink("$queueB/$dfile");
	exit (1);
    }
    if (-e "$xfile") {
	if (link("$xfile", "$queueB/$xfile") == 0) {
	    $bang = $!;
	    &syslog('err', 'link(%s, %s/%s): %s', $xfile, $queueB, $xfile, $bang);
	    print STDERR "$0: link($xfile, $queueB/$xfile): $bang\n";
	    unlink("$queueB/$dfile");
	    unlink("$queueB/$qfile");
	    exit (1);
	}
    }

    # Links created successfully.  Unlink the original files, release the
    # lock, and close the file.
    print "links ok\n" if ($debug);
    if (unlink($qfile) == 0) {
	$bang = $!;
	&syslog('err', 'unlink(%s): %s', $qfile, $bang);
	print STDERR "$0: unlink($qfile): $bang\n";
	exit (1);
    }
    if (unlink($dfile) == 0) {
	$bang = $!;
	&syslog('err', 'unlink(%s): %s', $dfile, $bang);
	print STDERR "$0: unlink($dfile): $bang\n";
	exit (1);
    }
    if (-e "$xfile") {
	if (unlink($xfile) == 0) {
	    $bang = $!;
	    &syslog('err', 'unlink(%s): %s', $xfile, $bang);
	    print STDERR "$0: unlink($xfile): $bang\n";
	    exit (1);
	}
    }
    flock(QF, $LOCK_UN);
    close(QF);
    &syslog('info', '%s moved to %s', $mfile, $queueB);
    print "Done with $dfile $qfile\n\n" if ($debug);
}