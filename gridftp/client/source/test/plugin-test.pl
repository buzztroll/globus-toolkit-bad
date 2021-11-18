#! /usr/bin/perl



=head1 plugin-test

Tests to exercise the plugin management of the client library.

=cut

use strict;
use Test::More;
use File::Basename;
use lib dirname($0);
use FtpTestLib;

my $test_exec = './plugin-test';
my @tests;

sub go
{
    my $rc;
    my $errors="";
    $errors = run_command("$test_exec", 0);

    ok($errors eq "", "go $test_exec");
}

push(@tests, "go();");

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
