#!/usr/bin/env perl



=pod

=head1 Tests for the globus IO file code

    Tests to exercise the file IO functionality of the globus IO library.

=cut

use strict;
use Test::More;
use File::Temp;
use File::Compare;

srand();

my $test_prog = 'globus_io_file_test';
my @tests;
my $valgrind="";

if (exists $ENV{VALGRIND})
{
    $valgrind = "valgrind --log-file=VALGRIND-$test_prog.log";
    if (exists $ENV{VALGRIND_OPTIONS})
    {
        $valgrind .= ' ' . $ENV{VALGRIND_OPTIONS};
    }
}

sub sig_handler
{
    if( -e "$test_prog.log.stdout" )
    {
        unlink("$test_prog.log.stdout");
    }

    if( -e "$test_prog.log.stderr" )
    {
        unlink("$test_prog.log.stderr");
    }
}

$SIG{'INT'}  = 'sig_handler';
$SIG{'QUIT'} = 'sig_handler';
$SIG{'KILL'} = 'sig_handler';

plan tests => 3;

my $testfile = mktemp("file_test_random_XXXXXXXX");
my $testfh;
my @chars = ("A".."Z", "a".."z", "0".."9");
my $rc;

open($testfh, ">$testfile");
print $testfh $chars[rand @chars] for 1..256;
close($testfh);

$rc = system("$valgrind ./$test_prog $testfile 1>$test_prog.log.stdout 2>$test_prog.log.stderr") / 256;

ok($rc == 0, "io file test exits with 0");

ok(File::Compare::compare("$test_prog.log.stdout", "$testfile") == 0, "io file test stdout matches");

ok(! -s "$test_prog.stderr", "io file test stderr empty");

if( -e "$test_prog.log.stdout" )
{
    unlink("$test_prog.log.stdout");
}

if( -e "$test_prog.log.stderr" )
{
    unlink("$test_prog.log.stderr");
}
unlink($testfile);
