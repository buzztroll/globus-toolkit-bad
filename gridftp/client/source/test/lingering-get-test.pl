#! /usr/bin/perl



=head1 globus-ftp-client-lingering-get-test

Tests to exercise the deactivation of the client and control libraries while
an operation is left in progress.

=cut

use strict;
use Test::More;
use File::Basename;
use lib dirname($0);
use FtpTestLib;

my $test_exec = './lingering-get-test';
my @tests;

=head1 I<lingering_get> (Test 1)

Do a get of $test_url. But don't deal with data block before deactivating
the client library.

=cut

my ($proto) = setup_proto();
my ($source_host, $source_file, $local_copy) = setup_remote_source();

sub lingering_get
{
    my ($errors,$rc) = ("",0);

    my $command = "$test_exec -s $proto$source_host$source_file";
    $errors = run_command($command, 1);
    
    ok($errors eq "", "lingering_get $command");
}
push(@tests, "lingering_get();");

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
