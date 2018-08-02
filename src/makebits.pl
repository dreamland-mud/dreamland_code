#!/usr/bin/perl
# ruffina, 2003

use Getopt::Std;
use File::stat;

#---------------------------------------------------------------------------
# init variables 
#---------------------------------------------------------------------------
getopts("rhf:s:", \%opts);
$source   = $opts{s};
$header   = $opts{h};
$file     = $opts{f};
$registry = $opts{r};

$source = 'bits.conf' unless $source;

exit if &files_not_modified($source, $file);

my @tabnames;

open (R, "> $file") || die "Cannot open output file.\n";

$file =~ s/^.*\///;

#---------------------------------------------------------------------------
# start output
#---------------------------------------------------------------------------
&print_comment();

if ($header) {
    &print_header_includes();
} else {
    &print_source_includes();
}

#---------------------------------------------------------------------------
# main TABLE routine 
#---------------------------------------------------------------------------
sub ENUMERATION {
    my $name = shift;
    &TABLE($name, 1, @_);
};

sub FLAGS {
    my $name = shift;
    &TABLE($name, 0, @_);
};

sub TABLE {
    $name = shift;
    $enum = shift;
    $classname = "FlagTable";
    $max = 0;
    $size = 0;
    @reverse = ();

    push @tabnames, $name if $registry;
    
    if ($header) {
	&print_header_table_start();
    } else {
	&print_source_table_start();
    }
    
    map {
	$array = shift;
	$value = 0;

	if ($enum) {
	    $value = @$array[1];
	}
	else {
	    $value = &flag_power(@$array[1]);
	    @$array[1] = "(@$array[1])";
	}
	
	if ($header) {
	    &print_header_table_body();
	} else {   
	    &print_source_table_body();
	}
	
	$max = $value if $value > $max;
	$reverse[$value] = $size if $value >= 0;
	$size++;
    } @_;

    unless ($header) {
	&print_source_table_end();
    }
}

#---------------------------------------------------------------------------
# suck-in the tables 
#---------------------------------------------------------------------------
require $source;

#---------------------------------------------------------------------------
# end printing
#---------------------------------------------------------------------------
if ($header) {
    &print_header_tail();
} elsif ($registry) {
    &print_source_tail();
}

close R;

#---------------------------------------------------------------------------
# Subroutines
#---------------------------------------------------------------------------
sub print_comment() 
{
    print R qq { 
/*
* Generated automatically from $source by $0.
* Do not modify directly this file.
*/
    };
}

sub print_header_includes()
{
    ($def = $file) =~s/(\w+)\.(\w+)$/__\U$1_$2__\E/;
    print R qq {
#ifndef $def
#define $def
    
#include "flagtable.h"
};
}

sub print_source_includes()
{
    ($hfile = $file) =~s/\.\w+/\.h/;
    print R qq {
#include "$hfile" 
#include "flagtableregistry.h"
#include "merc.h"
#include "def.h"

};    
}

sub print_header_tail()
{
    print R "\n\#endif\n";
}

sub print_source_tail()
{
    map {
	printf R "FlagTableRegistry::Entry reg_%-20s( %-20s, &%-20s );\n",
	         $_,
		 "\"$_\"",
		 $_;
    } @tabnames;
    
#    print R "const FlagTableRegistry::Field flagTableRegistry_fields [] = {\n";
#
#    map {
#	printf R "   { %-20s &%-20s }, \n", "\"$_\",", $_;
#    } @tabnames;
#
#    print R "    { 0, 0 }\n};\n";
#    print R "const FlagTableRegistry::Field * FlagTableRegistry::fields = flagTableRegistry_fields;\n";
}

sub print_header_table_start()
{
    print R qq {

/*
 * $name 
 */
extern const $classname $name;
};
}

sub print_source_table_start()
{
    print R "const $classname"."::Field $name\_fields [] = { \n";
}

sub print_header_table_body()
{
    printf R "\#define %-20s %-5s\n", @$array[0], @$array[1];
}

sub print_source_table_body()
{
    printf R " { %-5s",   $value.",";
    printf R " %-20s", "\"".@$array[2]."\","  if defined @$array[2];
    printf R " %-20s", "\"".@$array[3]."\","  if defined @$array[3];
    printf R " %-10s",      @$array[4]        if defined @$array[4];
    print  R " }, \n";
}

sub print_source_table_end()
{
    print R "};\n";
    print R "const int $name\_reverse [] = { \n  ";
    map {
	printf R "%s, ", (defined $_ ? $_ : "NO_FLAG");
    } @reverse;
    print R "\n};\n";
    print R "const $classname $name = { ", 
	     "\n\t\t$name\_fields, ",
	     "\n\t\t$name\_reverse, ",
	     "\n\t\t$size, $max, $enum };\n\n";
}

sub flag_power
{
    local $_ = shift;
    local $i;
    $power = 0;

    if (/^([A-Z]).*/) {
	$power = 0;
	for ($i = ord($1); $i > ord('A'); $i--) {
	    $power++;
	}
    }
    elsif (/^([a-z]).*/) {
	$power = 26;
	for ($i = ord($1); $i > ord('a'); $i--) {
	    $power++;
	}
    }
    
    return $power;
}

sub files_not_modified
{
    my ($src, $dst) = @_;

    return 0 unless -r $src;
    return 0 unless -w $dst;

    my $s_time = stat($src)->mtime;
    my $d_time = stat($dst)->mtime;
    
    return $s_time <= $d_time;
}
