
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
# We try first to get the version number from a local binary, in case we're
# running an installed version of Perl to produce documentation from an
# uninstalled newer version's pod files.if ($^O ne 'plan9' and $^O ne 'dos' and $^O ne 'os2' and $^O ne 'MSWin32') {
  my $perl = (-x './perl' && -f './perl' ) ?
                 './perl' :
                 ((-x '../perl' && -f '../perl') ?
                      '../perl' :
                      '');
  ($version,$patch) = `$perl -e 'print $]'` =~ /^(\d\.\d{3})(\d{2})?/ if $perl;
}
# No luck; we'll just go with the running Perl's version
($version,$patch) = $] =~ /^(.{5})(\d{2})?/ unless $version;
$DEF_RELEASE  = "perl $version";
$DEF_RELEASE .= ", patch $patch" if $patch;


sub makedate {
    my $secs = shift;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($secs);
    my $mname = (qw{Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec})[$mon];
    $year += 1900;
    return "$mday/$mname/$year";
}

use Getopt::Long;

$DEF_SECTION = 1;
$DEF_CENTER = "User Contributed Perl Documentation";
$STD_CENTER = "Perl Programmers Reference Guide";
$DEF_FIXED = 'CW';
$DEF_LAX = 0;

sub usage {
    warn "$0: @_\n" if @_;
    die <<EOF;
usage: $0 [options] podpage
Options are:
	--section=manext      (default "$DEF_SECTION")
	--release=relpatch    (default "$DEF_RELEASE")
	--center=string       (default "$DEF_CENTER")
	--date=string         (default "$DEF_DATE")
	--fixed=font	      (default "$DEF_FIXED")
	--official	      (default NOT)
	--lax                 (default NOT)
EOF
}

$uok = GetOptions( qw(
	section=s
	release=s
	center=s
	date=s
	fixed=s
	official
	lax
	help));

$DEF_DATE = makedate((stat($ARGV[0]))[9] || time());

usage("Usage error!") unless $uok;
usage() if $opt_help;
usage("Need one and only one podpage argument") unless @ARGV == 1;

$section = $opt_section || ($ARGV[0] =~ /\.pm$/
				? $DEF_PM_SECTION : $DEF_SECTION);
$RP = $opt_release || $DEF_RELEASE;
$center = $opt_center || ($opt_official ? $STD_CENTER : $DEF_CENTER);
$lax = $opt_lax || $DEF_LAX;

$CFont = $opt_fixed || $DEF_FIXED;

if (length($CFont) == 2) {
    $CFont_embed = "\\f($CFont";
}
elsif (length($CFont) == 1) {
    $CFont_embed = "\\f$CFont";
}
else {
    die "roff font should be 1 or 2 chars, not `$CFont_embed'";
}

$date = $opt_date || $DEF_DATE;

for (qw{NAME DESCRIPTION}) {
# for (qw{NAME DESCRIPTION AUTHOR}) {
    $wanna_see{$_}++;
}
$wanna_see{SYNOPSIS}++ if $section =~ /^3/;


$name = @ARGV ? $ARGV[0] : "<STDIN>";
$Filename = $name;
if ($section =~ /^1/) {
    require File::Basename;
    $name = uc File::Basename::basename($name);
}
$name =~ s/\.(pod|p[lm])$//i;

# Lose everything up to the first of
#     */lib/*perl*	standard or site_perl module
#     */*perl*/lib	from -D prefix=/opt/perl
#     */*perl*/		random module hierarchy
# which works.
$name =~ s-//+-/-g;
if ($name =~ s-^.*?/lib/[^/]*perl[^/]*/--i
	or $name =~ s-^.*?/[^/]*perl[^/]*/lib/--i
	or $name =~ s-^.*?/[^/]*perl[^/]*/--i) {
    # Lose ^site(_perl)?/.
    $name =~ s-^site(_perl)?/--;
    # Lose ^arch/.	(XXX should we use Config? Just for archname?)
    $name =~ s~^(.*-$^O|$^O-.*)/~~o;
    # Lose ^version/.
    $name =~ s-^\d+\.\d+/--;
}

# Translate Getopt/Long to Getopt::Long, etc.
$name =~ s(/)(::)g;

if ($name ne 'something') {
    FCHECK: {
	open(F, "< $ARGV[0]") || die "can't open $ARGV[0]: $!";
	while (<F>) {
	    next unless /^=\b/;
	    if (/^=head1\s+NAME\s*$/) {  # an /m would forgive mistakes
		$_ = <F>;
		unless (/\s*-+\s+/) {
		    $oops++;
		    warn "$0: Improper man page - no dash in NAME header in paragraph $. of $ARGV[0]\n"
                } else {
		    my @n = split /\s+-+\s+/;
		    if (@n != 2) {
			$oops++;
			warn "$0: Improper man page - malformed NAME header in paragraph $. of $ARGV[0]\n"
		    }
		    else {
			$n[0] =~ s/\n/ /g;
			$n[1] =~ s/\n/ /g;
			%namedesc = @n;
		    }
		}
		last FCHECK;
	    }
	    next if /^=cut\b/;	# DB_File and Net::Ping have =cut before NAME
	    next if /^=pod\b/;  # It is OK to have =pod before NAME
	    next if /^=(for|begin|end)\s+comment\b/;  # It is OK to have =for =begin or =end comment before NAME
	    die "$0: Invalid man page - 1st pod line is not NAME in $ARGV[0]\n" unless $lax;
	}
	die "$0: Invalid man page - no documentation in $ARGV[0]\n" unless $lax;
    }
    close F;
}

print <<"END";
.rn '' }`
''' \$RCSfile\$\$Revision\$\$Date\$
'''
''' \$Log\$
'''
.de Sh
.br
.if t .Sp
.ne 5
.PP
\\fB\\\\\$1\\fR
.PP
..
.de Sp
.if t .sp .5v
.if n .sp
..
.de Ip
.br
.ie \\\\n(.\$>=3 .ne \\\\\$3
.el .ne 3
.IP "\\\\\$1" \\\\\$2
..
.de Vb
.ft $CFont
.nf
.ne \\\\\$1
..
.de Ve
.ft R

.fi
..
'''
'''
'''     Set up \\*(-- to give an unbreakable dash;
'''     string Tr holds user defined translation string.
'''     Bell System Logo is used as a dummy character.
'''
.tr \\(*W-|\\(bv\\*(Tr
.ie n \\{\\
.ds -- \\(*W-
.ds PI pi
.if (\\n(.H=4u)&(1m=24u) .ds -- \\(*W\\h'-12u'\\(*W\\h'-12u'-\\" diablo 10 pitch
.if (\\n(.H=4u)&(1m=20u) .ds -- \\(*W\\h'-12u'\\(*W\\h'-8u'-\\" diablo 12 pitch
.ds L" ""
.ds R" ""
'''   \\*(M", \\*(S", \\*(N" and \\*(T" are the equivalent of
'''   \\*(L" and \\*(R", except that they are used on ".xx" lines,
'''   such as .IP and .SH, which do another additional levels of
'''   double-quote interpretation
.ds M" """
.ds S" """
.ds N" """""
.ds T" """""
.ds L' '
.ds R' '
.ds M' '
.ds S' '
.ds N' '
.ds T' '
'br\\}
.el\\{\\
.ds -- \\(em\\|
.tr \\*(Tr
.ds L" ``
.ds R" ''
.ds M" ``
.ds S" ''
.ds N" ``
.ds T" ''
.ds L' `
.ds R' '
.ds M' `
.ds S' '
.ds N' `
.ds T' '
.ds PI \\(*p
'br\\}
END

print <<'END';
.\"	If the F register is turned on, we'll generate
.\"	index entries out stderr for the following things:
.\"		TH	Title 
.\"		SH	Header
.\"		Sh	Subsection 
.\"		Ip	Item
.\"		X<>	Xref  (embedded
.\"	Of course, you have to process the output yourself
.\"	in some meaninful fashion.
.if \nF \{
.de IX
.tm Index:\\$1\t\\n%\t"\\$2"
..
.nr % 0
.rr F
.\}
END

print <<"END";
.TH $name $section "$RP" "$date" "$center"
.UC
END

push(@Indices, qq{.IX Title "$name $section"});

while (($name, $desc) = each %namedesc) {
    for ($name, $desc) { s/^\s+//; s/\s+$//; }
    push(@Indices, qq(.IX Name "$name - $desc"\n));
}

print <<'END';
.if n .hy 0
.if n .na
.ds C+ C\v'-.1v'\h'-1p'\s-2+\h'-1p'+\s0\v'.1v'\h'-1p'
.de CQ          \" put $1 in typewriter font
END
print ".ft $CFont\n";
print <<'END';
'if n "\c
'if t \\&\\$1\c
'if n \\&\\$1\c
'if n \&"
\\&\\$2 \\$3 \\$4 \\$5 \\$6 \\$7
'.ft R
..
.\" @(#)ms.acc 1.5 88/02/08 SMI; from UCB 4.2
.	\" AM - accent mark definitions
.bd B 3
.	\" fudge factors for nroff and troff
.if n \{\
.	ds #H 0
.	ds #V .8m
.	ds #F .3m
.	ds #[ \f1
.	ds #] \fP
.\}
.if t \{\
.	ds #H ((1u-(\\\\n(.fu%2u))*.13m)
.	ds #V .6m
.	ds #F 0
.	ds #[ \&
.	ds #] \&
.\}
.	\" simple accents for nroff and troff
.if n \{\
.	ds ' \&
.	ds ` \&
.	ds ^ \&
.	ds , \&
.	ds ~ ~
.	ds ? ?
.	ds ! !
.	ds /
.	ds q
.\}
.if t \{\
.	ds ' \\k:\h'-(\\n(.wu*8/10-\*(#H)'\'\h"|\\n:u"
.	ds ` \\k:\h'-(\\n(.wu*8/10-\*(#H)'\`\h'|\\n:u'
.	ds ^ \\k:\h'-(\\n(.wu*10/11-\*(#H)'^\h'|\\n:u'
.	ds , \\k:\h'-(\\n(.wu*8/10)',\h'|\\n:u'
.	ds ~ \\k:\h'-(\\n(.wu-\*(#H-.1m)'~\h'|\\n:u'
.	ds ? \s-2c\h'-\w'c'u*7/10'\u\h'\*(#H'\zi\d\s+2\h'\w'c'u*8/10'
.	ds ! \s-2\(or\s+2\h'-\w'\(or'u'\v'-.8m'.\v'.8m'
.	ds / \\k:\h'-(\\n(.wu*8/10-\*(#H)'\z\(sl\h'|\\n:u'
.	ds q o\h'-\w'o'u*8/10'\s-4\v'.4m'\z\(*i\v'-.4m'\s+4\h'\w'o'u*8/10'
.\}
.	\" troff and (daisy-wheel) nroff accents
.ds : \\k:\h'-(\\n(.wu*8/10-\*(#H+.1m+\*(#F)'\v'-\*(#V'\z.\h'.2m+\*(#F'.\h'|\\n:u'\v'\*(#V'
.ds 8 \h'\*(#H'\(*b\h'-\*(#H'
.ds v \\k:\h'-(\\n(.wu*9/10-\*(#H)'\v'-\*(#V'\*(#[\s-4v\s0\v'\*(#V'\h'|\\n:u'\*(#]
.ds _ \\k:\h'-(\\n(.wu*9/10-\*(#H+(\*(#F*2/3))'\v'-.4m'\z\(hy\v'.4m'\h'|\\n:u'
.ds . \\k:\h'-(\\n(.wu*8/10)'\v'\*(#V*4/10'\z.\v'-\*(#V*4/10'\h'|\\n:u'
.ds 3 \*(#[\v'.2m'\s-2\&3\s0\v'-.2m'\*(#]
.ds o \\k:\h'-(\\n(.wu+\w'\(de'u-\*(#H)/2u'\v'-.3n'\*(#[\z\(de\v'.3n'\h'|\\n:u'\*(#]
.ds d- \h'\*(#H'\(pd\h'-\w'~'u'\v'-.25m'\f2\(hy\fP\v'.25m'\h'-\*(#H'
.ds D- D\\k:\h'-\w'D'u'\v'-.11m'\z\(hy\v'.11m'\h'|\\n:u'
.ds th \*(#[\v'.3m'\s+1I\s-1\v'-.3m'\h'-(\w'I'u*2/3)'\s-1o\s+1\*(#]
.ds Th \*(#[\s+2I\s-2\h'-\w'I'u*3/5'\v'-.3m'o\v'.3m'\*(#]
.ds ae a\h'-(\w'a'u*4/10)'e
.ds Ae A\h'-(\w'A'u*4/10)'E
.ds oe o\h'-(\w'o'u*4/10)'e
.ds Oe O\h'-(\w'O'u*4/10)'E
.	\" corrections for vroff
.if v .ds ~ \\k:\h'-(\\n(.wu*9/10-\*(#H)'\s-2\u~\d\s+2\h'|\\n:u'
.if v .ds ^ \\k:\h'-(\\n(.wu*10/11-\*(#H)'\v'-.4m'^\v'.4m'\h'|\\n:u'
.	\" for low resolution devices (crt and lpr)
.if \n(.H>23 .if \n(.V>19 \
\{\
.	ds : e
.	ds 8 ss
.	ds v \h'-1'\o'\(aa\(ga'
.	ds _ \h'-1'^
.	ds . \h'-1'.
.	ds 3 3
.	ds o a
.	ds d- d\h'-1'\(ga
.	ds D- D\h'-1'\(hy
.	ds th \o'bp'
.	ds Th \o'LP'
.	ds ae ae
.	ds Ae AE
.	ds oe oe
.	ds Oe OE
.\}
.rm #[ #] #H #V #F C
END

$indent = 0;

$begun = "";

# Unrolling [^A-Z>]|[A-Z](?!<) gives:    // MRE pp 165.
my $nonest = '(?:[^A-Z>]*(?:[A-Z](?!<)[^A-Z>]*)*)';

while (<>) {
    if ($cutting) {
	next unless /^=/;
	$cutting = 0;
    }
    if ($begun) {
	if (/^=end\s+$begun/) {
            $begun = "";
	}
	elsif ($begun =~ /^(roff|man)$/) {
	    print STDOUT $_;
        }
	next;
    }
    chomp;

    # Translate verbatim paragraph

    if (/^\s/) {
	@lines = split(/\n/);
	for (@lines) {
	    1 while s
		{^( [^\t]* ) \t ( \t* ) }
		{ $1 . ' ' x (8 - (length($1)%8) + 8 * (length($2))) }ex;
	    s/\\/\\e/g;
	    s/\A/\\&/s;
	}
	$lines = @lines;
	makespace() unless $verbatim++;
	print ".Vb $lines\n";
	print join("\n", @lines), "\n";
	print ".Ve\n";
	$needspace = 0;
	next;
    }

    $verbatim = 0;

    if (/^=for\s+(\S+)\s*/s) {
	if ($1 eq "man" or $1 eq "roff") {
	    print STDOUT $',"\n\n";
	} else {
	    # ignore unknown for
	}
	next;
    }
    elsif (/^=begin\s+(\S+)\s*/s) {
	$begun = $1;
	if ($1 eq "man" or $1 eq "roff") {
	    print STDOUT $'."\n\n";
	}
	next;
    }

    # check for things that'll hosed our noremap scheme; affects $_
    init_noremap();

    if (!/^=item/) {

	# trofficate backslashes; must do it before what happens below
	s/\\/noremap('\\e')/ge;

	# protect leading periods and quotes against *roff
	# mistaking them for directives
	s/^(?:[A-Z]<)?[.']/\\&$&/gm;

	# first hide the escapes in case we need to
	# intuit something and get it wrong due to fmting

	1 while s/([A-Z]<$nonest>)/noremap($1)/ge;

	# func() is a reference to a perl function
	s{
	    \b
	    (
		[:\w]+ \(\)
	    )
	} {I<$1>}gx;

	# func(n) is a reference to a perl function or a man page
	s{
	    ([:\w]+)
	    (
		\( [^\051]+ \)
	    )
	} {I<$1>\\|$2}gx;

	# convert simple variable references
	s/(\s+)([\$\@%][\w:]+)(?!\()/${1}C<$2>/g;

	if (m{ (
		    [\-\w]+
		    \(
			[^\051]*?
			[\@\$,]
			[^\051]*?
		    \)
		)
	    }x && $` !~ /([LCI]<[^<>]*|-)$/ && !/^=\w/)
	{
	    warn "$0: bad option in paragraph $. of $ARGV: ``$1'' should be [LCI]<$1>\n";
	    $oops++;
	}

	while (/(-[a-zA-Z])\b/g && $` !~ /[\w\-]$/) {
	    warn "$0: bad option in paragraph $. of $ARGV: ``$1'' should be [CB]<$1>\n";
	    $oops++;
	}

	# put it back so we get the <> processed again;
	clear_noremap(0); # 0 means leave the E's

    } else {
	# trofficate backslashes
	s/\\/noremap('\\e')/ge;

    }

    # need to hide E<> first; they're processed in clear_noremap
    s/(E<[^<>]+>)/noremap($1)/ge;


    $maxnest = 10;
    while ($maxnest-- && /[A-Z]</) {

	# can't do C font here
	s/([BI])<($nonest)>/font($1) . $2 . font('R')/eg;

	# files and filelike refs in italics
	s/F<($nonest)>/I<$1>/g;

	# no break -- usually we want C<> for this
	s/S<($nonest)>/nobreak($1)/eg;

	# LREF: a la HREF L<show this text|man/section>
	s:L<([^|>]+)\|[^>]+>:$1:g;

	# LREF: a manpage(3f)
	s:L<([a-zA-Z][^\s\/]+)(\([^\)]+\))?>:the I<$1>$2 manpage:g;

	# LREF: an =item on another manpage
	s{
	    L<
		([^/]+)
		/
		(
		    [:\w]+
		    (\(\))?
		)
	    >
	} {the C<$2> entry in the I<$1> manpage}gx;

	# LREF: an =item on this manpage
	s{
	   ((?:
	    L<
		/
		(
		    [:\w]+
		    (\(\))?
		)
	    >
	    (,?\s+(and\s+)?)?
	  )+)
	} { internal_lrefs($1) }gex;

	# LREF: a =head2 (head1?), maybe on a manpage, maybe right here
	# the "func" can disambiguate
	s{
	    L<
		(?:
		    ([a-zA-Z]\S+?) /
		)?
		"?(.*?)"?
	    >
	}{
	    do {
		$1 	# if no $1, assume it means on this page.
		    ?  "the section on I<$2> in the I<$1> manpage"
		    :  "the section on I<$2>"
	    }
	}gesx; # s in case it goes over multiple lines, so . matches \n

	s/Z<>/\\&/g;

	# comes last because not subject to reprocessing
	s/C<($nonest)>/noremap("${CFont_embed}${1}\\fR")/eg;
    }

    if (s/^=//) {
	$needspace = 0;		# Assume this.

	s/\n/ /g;

	($Cmd, $_) = split(' ', $_, 2);

	$dotlevel = 1;
	if ($Cmd eq 'head1') {
	   $dotlevel = 1;
	}
	elsif ($Cmd eq 'head2') {
	   $dotlevel = 1;
	}
	elsif ($Cmd eq 'item') {
	   $dotlevel = 2;
	}

	if (defined $_) {
	    &escapes($dotlevel);
	    s/"/""/g;
	}

	clear_noremap(1);

	if ($Cmd eq 'cut') {
	    $cutting = 1;
	}
	elsif ($Cmd eq 'head1') {
	    s/\s+$//;
	    delete $wanna_see{$_} if exists $wanna_see{$_};
	    print qq{.SH "$_"\n};
      push(@Indices, qq{.IX Header "$_"\n});
	}
	elsif ($Cmd eq 'head2') {
	    print qq{.Sh "$_"\n};
      push(@Indices, qq{.IX Subsection "$_"\n});
	}
	elsif ($Cmd eq 'over') {
	    push(@indent,$indent);
	    $indent += ($_ + 0) || 5;
	}
	elsif ($Cmd eq 'back') {
	    $indent = pop(@indent);
	    warn "$0: Unmatched =back in paragraph $. of $ARGV\n" unless defined $indent;
	    $needspace = 1;
	}
	elsif ($Cmd eq 'item') {
	    s/^\*( |$)/\\(bu$1/g;
	    # if you know how to get ":s please do
	    s/\\\*\(L"([^"]+?)\\\*\(R"/'$1'/g;
	    s/\\\*\(L"([^"]+?)""/'$1'/g;
	    s/[^"]""([^"]+?)""[^"]/'$1'/g;
	    # here do something about the $" in perlvar?
	    print STDOUT qq{.Ip "$_" $indent\n};
      push(@Indices, qq{.IX Item "$_"\n});
	}
	elsif ($Cmd eq 'pod') {
	    # this is just a comment
	} 
	else {
	    warn "$0: Unrecognized pod directive in paragraph $. of $ARGV: $Cmd\n";
	}
    }
    else {
	if ($needspace) {
	    &makespace;
	}
	&escapes(0);
	clear_noremap(1);
	print $_, "\n";
	$needspace = 1;
    }
}

print <<"END";

.rn }` ''
END

if (%wanna_see && !$lax) {
    @missing = keys %wanna_see;
    warn "$0: $Filename is missing required section"
	.  (@missing > 1 && "s")
	.  ": @missing\n";
    $oops++;
}

foreach (@Indices) { print "$_\n"; }

exit;
#exit ($oops != 0);

#########################################################################

sub nobreak {
    my $string = shift;
    $string =~ s/ /\\ /g;
    $string;
}

sub escapes {
    my $indot = shift;

    s/X<(.*?)>/mkindex($1)/ge;

    # translate the minus in foo-bar into foo\-bar for roff
    s/([^0-9a-z-])-([^-])/$1\\-$2/g;

    # make -- into the string version \*(-- (defined above)
    s/\b--\b/\\*(--/g;
    s/"--([^"])/"\\*(--$1/g;  # should be a better way
    s/([^"])--"/$1\\*(--"/g;

    # fix up quotes; this is somewhat tricky
    my $dotmacroL = 'L';
    my $dotmacroR = 'R';
    if ( $indot == 1 ) {
	$dotmacroL = 'M';
	$dotmacroR = 'S';
    }  
    elsif ( $indot >= 2 ) {
	$dotmacroL = 'N';
	$dotmacroR = 'T';
    }  
    if (!/""/) {
	s/(^|\s)(['"])/noremap("$1\\*($dotmacroL$2")/ge;
	s/(['"])($|[\-\s,;\\!?.])/noremap("\\*($dotmacroR$1$2")/ge;
    }

    #s/(?!")(?:.)--(?!")(?:.)/\\*(--/g;
    #s/(?:(?!")(?:.)--(?:"))|(?:(?:")--(?!")(?:.))/\\*(--/g;


    # make sure that func() keeps a bit a space tween the parens
    ### s/\b\(\)/\\|()/g;
    ### s/\b\(\)/(\\|)/g;

    # make C++ into \*C+, which is a squinched version (defined above)
    s/\bC\+\+/\\*(C+/g;

    # make double underbars have a little tiny space between them
    s/__/_\\|_/g;

    # PI goes to \*(PI (defined above)
    s/\bPI\b/noremap('\\*(PI')/ge;

    # make all caps a teeny bit smaller, but don't muck with embedded code literals
    my $hidCFont = font('C');
    if ($Cmd !~ /^head1/) { # SH already makes smaller
	# /g isn't enough; 1 while or we'll be off

#	1 while s{
#	    (?!$hidCFont)(..|^.|^)
#	    \b
#	    (
#		[A-Z][\/A-Z+:\-\d_$.]+
#	    )
#	    (s?) 		
#	    \b
#	} {$1\\s-1$2\\s0}gmox;

	1 while s{
	    (?!$hidCFont)(..|^.|^)
	    (
		\b[A-Z]{2,}[\/A-Z+:\-\d_\$]*\b
	    )
	} {
	    $1 . noremap( '\\s-1' .  $2 . '\\s0' )
	}egmox;

    }
}

# make troff just be normal, but make small nroff get quoted
# decided to just put the quotes in the text; sigh;
sub ccvt {
    local($_,$prev) = @_;
    noremap(qq{.CQ "$_" \n\\&});
}

sub makespace {
    if ($indent) {
	print ".Sp\n";
    }
    else {
	print ".PP\n";
    }
}

sub mkindex {
    my ($entry) = @_;
    my @entries = split m:\s*/\s*:, $entry;
    push @Indices, ".IX Xref " . join ' ', map {qq("$_")} @entries;
    return '';
}

sub font {
    local($font) = shift;
    return '\\f' . noremap($font);
}

sub noremap {
    local($thing_to_hide) = shift;
    $thing_to_hide =~ tr/\000-\177/\200-\377/;
    return $thing_to_hide;
}

sub init_noremap {
	# escape high bit characters in input stream
	s/([\200-\377])/"E<".ord($1).">"/ge;
}

sub clear_noremap {
    my $ready_to_print = $_[0];

    tr/\200-\377/\000-\177/;

    # trofficate backslashes
    # s/(?!\\e)(?:..|^.|^)\\/\\e/g;

    # now for the E<>s, which have been hidden until now
    # otherwise the interative \w<> processing would have
    # been hosed by the E<gt>
    s {
	    E<
	    (
	        ( \d + ) 
	        | ( [A-Za-z]+ )	
	    )
	    >	
    } {
	 do {
	     defined $2
		? chr($2)
		:	
	     exists $HTML_Escapes{$3}
		? do { $HTML_Escapes{$3} }
		: do {
		    warn "$0: Unknown escape in paragraph $. of $ARGV: ``$&''\n";
		    "E<$1>";
		}
	 }
    }egx if $ready_to_print;
}

sub internal_lrefs {
    local($_) = shift;
    local $trailing_and = s/and\s+$// ? "and " : "";

    s{L</([^>]+)>}{$1}g;
    my(@items) = split( /(?:,?\s+(?:and\s+)?)/ );
    my $retstr = "the ";
    my $i;
    for ($i = 0; $i <= $#items; $i++) {
	$retstr .= "C<$items[$i]>";
	$retstr .= ", " if @items > 2 && $i != $#items;
	$retstr .= " and " if $i+2 == @items;
    }

    $retstr .= " entr" . ( @items > 1  ? "ies" : "y" )
	    .  " elsewhere in this document";
    # terminal space to avoid words running together (pattern used
    # strips terminal spaces)
    $retstr .= " " if length $trailing_and;
    $retstr .=  $trailing_and;

    return $retstr;

}

BEGIN {
%HTML_Escapes = (
    'amp'	=>	'&',	#   ampersand
    'lt'	=>	'<',	#   left chevron, less-than
    'gt'	=>	'>',	#   right chevron, greater-than
    'quot'	=>	'"',	#   double quote

    "Aacute"	=>	"A\\*'",	#   capital A, acute accent
    "aacute"	=>	"a\\*'",	#   small a, acute accent
    "Acirc"	=>	"A\\*^",	#   capital A, circumflex accent
    "acirc"	=>	"a\\*^",	#   small a, circumflex accent
    "AElig"	=>	'\*(AE',	#   capital AE diphthong (ligature)
    "aelig"	=>	'\*(ae',	#   small ae diphthong (ligature)
    "Agrave"	=>	"A\\*`",	#   capital A, grave accent
    "agrave"	=>	"A\\*`",	#   small a, grave accent
    "Aring"	=>	'A\\*o',	#   capital A, ring
    "aring"	=>	'a\\*o',	#   small a, ring
    "Atilde"	=>	'A\\*~',	#   capital A, tilde
    "atilde"	=>	'a\\*~',	#   small a, tilde
    "Auml"	=>	'A\\*:',	#   capital A, dieresis or umlaut mark
    "auml"	=>	'a\\*:',	#   small a, dieresis or umlaut mark
    "Ccedil"	=>	'C\\*,',	#   capital C, cedilla
    "ccedil"	=>	'c\\*,',	#   small c, cedilla
    "Eacute"	=>	"E\\*'",	#   capital E, acute accent
    "eacute"	=>	"e\\*'",	#   small e, acute accent
    "Ecirc"	=>	"E\\*^",	#   capital E, circumflex accent
    "ecirc"	=>	"e\\*^",	#   small e, circumflex accent
    "Egrave"	=>	"E\\*`",	#   capital E, grave accent
    "egrave"	=>	"e\\*`",	#   small e, grave accent
    "ETH"	=>	'\\*(D-',	#   capital Eth, Icelandic
    "eth"	=>	'\\*(d-',	#   small eth, Icelandic
    "Euml"	=>	"E\\*:",	#   capital E, dieresis or umlaut mark
    "euml"	=>	"e\\*:",	#   small e, dieresis or umlaut mark
    "Iacute"	=>	"I\\*'",	#   capital I, acute accent
    "iacute"	=>	"i\\*'",	#   small i, acute accent
    "Icirc"	=>	"I\\*^",	#   capital I, circumflex accent
    "icirc"	=>	"i\\*^",	#   small i, circumflex accent
    "Igrave"	=>	"I\\*`",	#   capital I, grave accent
    "igrave"	=>	"i\\*`",	#   small i, grave accent
    "Iuml"	=>	"I\\*:",	#   capital I, dieresis or umlaut mark
    "iuml"	=>	"i\\*:",	#   small i, dieresis or umlaut mark
    "Ntilde"	=>	'N\*~',		#   capital N, tilde
    "ntilde"	=>	'n\*~',		#   small n, tilde
    "Oacute"	=>	"O\\*'",	#   capital O, acute accent
    "oacute"	=>	"o\\*'",	#   small o, acute accent
    "Ocirc"	=>	"O\\*^",	#   capital O, circumflex accent
    "ocirc"	=>	"o\\*^",	#   small o, circumflex accent
    "Ograve"	=>	"O\\*`",	#   capital O, grave accent
    "ograve"	=>	"o\\*`",	#   small o, grave accent
    "Oslash"	=>	"O\\*/",	#   capital O, slash
    "oslash"	=>	"o\\*/",	#   small o, slash
    "Otilde"	=>	"O\\*~",	#   capital O, tilde
    "otilde"	=>	"o\\*~",	#   small o, tilde
    "Ouml"	=>	"O\\*:",	#   capital O, dieresis or umlaut mark
    "ouml"	=>	"o\\*:",	#   small o, dieresis or umlaut mark
    "szlig"	=>	'\*8',		#   small sharp s, German (sz ligature)
    "THORN"	=>	'\\*(Th',	#   capital THORN, Icelandic
    "thorn"	=>	'\\*(th',,	#   small thorn, Icelandic
    "Uacute"	=>	"U\\*'",	#   capital U, acute accent
    "uacute"	=>	"u\\*'",	#   small u, acute accent
    "Ucirc"	=>	"U\\*^",	#   capital U, circumflex accent
    "ucirc"	=>	"u\\*^",	#   small u, circumflex accent
    "Ugrave"	=>	"U\\*`",	#   capital U, grave accent
    "ugrave"	=>	"u\\*`",	#   small u, grave accent
    "Uuml"	=>	"U\\*:",	#   capital U, dieresis or umlaut mark
    "uuml"	=>	"u\\*:",	#   small u, dieresis or umlaut mark
    "Yacute"	=>	"Y\\*'",	#   capital Y, acute accent
    "yacute"	=>	"y\\*'",	#   small y, acute accent
    "yuml"	=>	"y\\*:",	#   small y, dieresis or umlaut mark
);
}