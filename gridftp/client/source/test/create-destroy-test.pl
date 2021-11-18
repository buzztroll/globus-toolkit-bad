#! /usr/bin/perl


#
# Extremely basic data structure creation/destroy test. No transfers
# are done here. Sanity check activation/deactivation code, and make
# sure handle con/destructors work.

use strict;
use Test::More;
use File::Basename;
use lib dirname($0);
use FtpTestLib;

my $test_exec = './create-destroy-test';
my @tests;

sub create_destroy
{
    my ($errors,$rc) = ("",0);

    my $command = "$test_exec";
    $errors = run_command($command, 0);

    ok($errors eq "", "create_destroy $command");
}
push(@tests, "create_destroy");

if(defined($ENV{FTP_TEST_RANDOMIZE}))
{
    shuffle(\@tests);
}

if(@ARGV)
{
    plan tests => scalar(@ARGV);

    foreach (@ARGV)
    {
        eval "&$tests[$_-1]";
    }
}
else
{
    plan tests => scalar(@tests);

    foreach (@tests)
    {
        eval "&$_";
    }
}
