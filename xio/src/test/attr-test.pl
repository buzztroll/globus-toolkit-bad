#! /usr/bin/env perl



use strict;
use POSIX;
use Test::More;
use File::Basename;
use lib dirname($0);

 require "test-common.pl";

my $type = 0;
if(@ARGV == 1)
{
    $type = 1;
}

my $test_exec="./framework_test";
my $test_name="attr";
my @tests;
my @todo;

push(@tests,  "$test_name");

if($type == 1)
{
    foreach(@tests)
    {
        print "$_\n";
    }
}
else
{
    my $cnt=0;
    plan tests => scalar(@tests), todo => \@todo;
    foreach(@tests)
    {
        my $test_str="$test_name.$cnt";
        &run_test("$test_exec $_", $test_str);
        $cnt++;
    }
}
