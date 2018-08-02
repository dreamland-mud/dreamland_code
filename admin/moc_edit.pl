#!/usr/bin/perl

use Cwd;
use File::Find;
use File::Basename;


my %directory;
my $thisRealName = basename( "$0" );
my $thisProg = "$0";

while( defined( $ARGV[0] ) )
{
	my $root = shift;

	if( $root eq "--root" )
	{
		if( defined( $ARGV[0] ) )
		{
			$RootDir = shift;
			$RootDir = "$topdir/" . basename( $RootDir );
			$thisProg .= " --root $RootDir";
		}
	}
	else
	{
		$_ = $root;

		$_ = cwd( ) . "/" . $_ if( ! /^\// );
		$directory{$_} = 1;
		$localDir = 1;
	}
}

if( !defined( $localDir ) )
{
	find( \&add_file, cwd( ) );
	foreach $file ( @files )
	{
		processFile( $file );
		last if( $errorflag );
	}
}
else
{
	foreach $dir ( keys %directory )
	{
		processFile( $dir . "/Makefile.in" );
		last if( $errorflag );
	}
}

processFiles( );

exit $errorflag;


# -----------------------*** functions ***--------------------

if( !defined( $localDir ) )
{
	find( \&add_file, cwd( ) );
}



sub add_file( )
{
	push( @files, $File::Find::name ) if( /Makefile.in$/ );
}

sub processFile( $ )
{
	my( $file ) = @_;
	$fileDir = dirname( $file );

#	$file = basename( $file );

	$filesInDirectory{$fileDir} = " $file"
		if( ( defined( $localdir )
			&& defined( $directory{$fileDir} ) )
			|| !defined( $localdir ) );
}

sub processFiles( )
{
	foreach $printname ( keys %filesInDirectory )
	{
		my $moc_files = "";
		my $file = $filesInDirectory{$printname};
		modifyMakefile( $file );
	}
}

sub modifyMakefile( $ )
{
	my( $file ) = @_;

	my @libraries = ();

	if( $RootDir eq $printname )
	{
		$localDir = " -r ";
	}
	else
	{
		$localDir = "";
	}

	$makefileData = "";

	open( FILEIN, $file ) || die "Can't open '$file': $!";
	while( <FILEIN> )
	{
		$makefileData .= $_;
	}
	close( FILEIN );

	$makefileData =~ s/\\\s*\n/\034/g;

	while( $makefileData =~ /\n(\S*)_OBJECTS\s*=[ \t\034]*([^\n]*)\n/g )
	{
		my $library = $1;
		my $objs = $2;

		$library =~ s/^am_// if( $library =~ /^am_/ );
#		$library =~ s/_la$// if( $library =~ /_la$/ );
		push( @libraries, $library );
	}

	tag_AUTOMAKE( );

	my $depfiles = "";
	
	foreach $library ( @libraries )
	{
		my $files = "";
		my $lookup = "($library\_OBJECTS.*=)([^\n]*)";
		my $lookupH = "$library\_MOC.*=([^\n]*)";
			
		if( $makefileData =~ m/\n$lookupH/ )
		{
			$_ = $1;
			s/^[ \t\034]*//;
			foreach my $file (split /[ \t\034]+/) {
			    $files .= "\$(srcdir)/$file ";
			}
		}

		next if $files eq "";
		
		if( $makefileData =~ m/\n$lookup/ )
		{
			my $newLine = "$1 $library\_moc_xml.lo $2";
			substituteLine( $lookup, $newLine );
		}

		$depfiles .= "./\$(DEPDIR)/$library\_moc_xml.Plo ";
		
#XXX: ..._moc_xml.cpp: ... Makefile 

		appendLines (qq{
		    
$library\_moc_xml.cpp:	\$(top_builddir)/src/moc/moc $files
	\$(top_builddir)/src/moc/moc $localDir -o \$\@ -i '$files' -I\$(srcdir) \$(INCLUDES) 

MOC_CLEANFILES += $library\_moc_xml.cpp

\@AMDEP_TRUE\@\@am__include\@ \@am__quote\@./\$(DEPDIR)/$library\_moc_xml.Plo\@am__quote\@

});
	}

	my $lookupD = "(\@AMDEP_TRUE\@DEP_FILES.*=)([^\n]*)";
	if( $makefileData =~ m/\n$lookupD/ )
	{
		my $newLine = "$1 $depfiles $2";
		substituteLine( $lookupD, $newLine );
	}
	
	appendLines (qq{
	    
clean-generic: clean-moc

clean-moc:
	-rm -f \$(MOC_CLEANFILES)
});

	updateMakefile( $file );

        #chdir( $topdir );
}

sub tag_AUTOMAKE( )
{
	my $lookup = '.*cd \$\(top_srcdir\)[ \t\034]+&&[ \t\034]+\$\(AUTOMAKE\)(.*)';
	return 1 if( $makefileData !~ /($lookup)/ );
	my $newLine = $1 . "\n\tcd \$(top_srcdir) && /usr/bin/perl admin/$thisRealName $printname";
	substituteLine( $lookup, $newLine );
}

sub substituteLine( $$ )
{
	my( $lookup, $new ) = @_;

	if( $makefileData =~ /\n($lookup)/ )
	{
		$old = $1;
		$old =~ s/\034/\\\n#>\- /g;
		$new =~ s/\034/\\\n/g;
		my $newCount = 1;
		$newCount++ while( $new =~ /\n/g );

		$makefileData =~ s/\n$lookup/\n#>- $old\n#>\+ $newCount\n$new/;
	}
	else
	{
		die "!!!!!!!\n";
	}
}

sub appendLines( $ )
{
	my( $new ) = @_;

	$new =~ s/\034/\\\n/g;
	my $newCount = 1;
	$newCount++ while( $new =~ /\n/g );

	$makefileData .= "\n#>\+ $newCount\n$new";
}

sub updateMakefile( $ )
{
	my( $file ) = @_;
	open( FILEOUT, "> $file" ) || die "Could not create '$file': $!\n";
	$makefileData =~ s/\034/\\\n/g;
	print FILEOUT $makefileData;
	close( FILEOUT );
}
