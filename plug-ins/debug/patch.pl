
my $cfile="dreamlandcore.feb";
$dynamic=0x08049e4c;

my $slurp=$/;
open CR, "<$cfile";
undef $/;
$core = <CR>;
close CR;
$/=$slurp;

open RDELF, "readelf -l $cfile|";

my @bodyparts = ();

while(<RDELF>) {
    chomp;
    next unless /LOAD/;

    @_ = split;

    push @bodyparts, {
	Type	=> shift @_,
	Offset	=> eval shift @_,
	VirtAddr=> eval shift @_,
	PhysAddr=> eval shift @_,
	FileSiz	=> eval shift @_,
	MemSiz	=> eval shift @_,
	Flg	=> shift @_,
	Align	=> eval shift @_
    };
}

close RDELF;



sub v2o {
    my $v = shift;

    foreach (@bodyparts) {
	my ($start, $end) = (${%$_}{VirtAddr}, ${%$_}{MemSiz});
	$end += $start;
	return ${%$_}{Offset}+$v-$start if $v >= $start and $v < $end;
    }

    die "sigsegv! no $v in core";
}

sub vcore {
    my ($vptr, $len) = @_;
    return substr $core, v2o($vptr), $len;
}

sub vcorez {
    my $optr = v2o shift;
    return unpack "x$optr Z*", $core;
}

sub vcorel {
    my $optr = v2o shift;
    return unpack "x$optr L", $core;
}

for(;;$dynamic+=8) {
    ($type, $val) = unpack "LL", vcore $dynamic, 8;
    die "no debug entry in .dynamic section" if $type == 0;
    next unless $type == 0x15;
    $debug = $val;
    last;
}

my ($sos) = vcorel $debug+4;

my $basetosize = 0x20-0xb8;

printf "obj_list: 0x%08x\n", $sos-0xb8;

open PC, ">patch.c";

while($sos) {
    my ($base, $name, $next, $prev) = unpack "LL x4 LL", vcore $sos, 20;
    my $size = vcorel $sos+$basetosize;
    $name = vcorez $name;

    if($size == 0 && $name =~ /\/ld-elf.so/) {
	my @stat = stat $name;

	$size = ($stat[7] + 0x1000) & ~(0x1000 - 1);
    }
    
    printf PC "map(\"%s\", 0, 0x%08x, 0x%08x, 0);\n", $name, $base, $size;

    $sos=$next;
}

foreach $part (@bodyparts) {
    printf PC "map(\"$cfile\", 0x%08x, 0x%08x, 0x%08x, 1);\n",
	    map { ${%$part}{$_} } ("Offset", "VirtAddr", "MemSiz");
}

close PC;
0;
