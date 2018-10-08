#!/usr/bin/perl

use BerkeleyDB;
use XML::XPath;
use XML::XPath::XMLParser;

my %objs, $css;

$env = new BerkeleyDB::Env (
    -Flags => DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_INIT_LOCK,
    -Home => "."
);

tie %css, "BerkeleyDB::Btree",
    -Env => $env,
    -Filename => "fenia",
    -Subname => "codesouces",
    -Flags => DB_RDONLY or die$!;

tie %objs, "BerkeleyDB::Btree",
    -Env => $env,
    -Filename => "fenia",
    -Subname => "objects",
    -Flags => DB_RDONLY or die $!;

my $parser = XML::XPath::XMLParser->new( );
my $xp = XML::XPath->new( );

my $root = XML::XPath::Node::Element->new("root", "");

my $key;

$|=1;

my $i;

print "Parsing World: ";

foreach $key (keys %objs) {
    my ($id, $xml) = ( unpack ("L", $key), $objs{$key} );
    print "." unless $i++ % 10;

    my $nodeset = $parser->parse($xml)->getChildNode(1);

    $nodeset->appendAttribute(XML::XPath::Node::Attribute->new("id", $id, ""));

    $root->appendChild($nodeset);
}

foreach $key (keys %css) {
    my ($id, $subj, $owner, $text) = 
	( unpack ("L", $key), unpack("Z* Z* Z*", $css{$key}) );

    print ",";

    my $node = XML::XPath::Node::Element->new("codesource", "");

    sub addatr {
	my $node = shift;
	$node->appendAttribute(XML::XPath::Node::Attribute->new(@_));
    }
    addatr($node, "id", $id);
    addatr($node, "owner", $owner);
    addatr($node, "subj", $subj);

    $node->appendChild(XML::XPath::Node::Text->new($text));

    $root->appendChild($node);
}

print "Done\n";

my $rc = $xp->find($ARGV[0], $root);

if($rc->isa(XML::XPath::NodeSet)) {
    foreach ($rc->get_nodelist) {
	print $_->toString . "\n";
    }
} else {
    print "$rc\n";
}
