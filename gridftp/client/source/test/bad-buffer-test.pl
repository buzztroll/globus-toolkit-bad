#! /usr/bin/perl


#
# Try reading an url by passing in a bad data buffer

use strict;
use Test::More;
use File::Basename;
use lib dirname($0);
use FtpTestLib;

my $test_exec = './bad-buffer-test';

my @tests;
my @todo;

my ($proto) = setup_proto();
my ($source_host, $source_file, $local_copy) = setup_remote_source();

# Test 1: Bad buffer test
# Success if the transfer fails with exit code 2
sub bad_buffer
{
    my ($errors,$rc) = ("",0);
    my ($output);

    my $command = "$test_exec -s $proto$source_host$source_file";
    $errors = run_command($command, 2);

    ok($errors eq "", "bad_buffer $command");
}
push(@tests, "bad_buffer");

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
    plan tests => scalar(@tests), todo => \@todo;

    foreach (@tests)
    {
        eval "&$_";
    }
}
