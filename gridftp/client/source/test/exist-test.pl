#! /usr/bin/perl



=head1 exist-test

    Tests to exercise the existence checking of the client library.

=cut

use strict;
use File::Temp qw/:POSIX/;
use Test::More;
use File::Basename;
use lib dirname($0);
use FtpTestLib;

my $test_exec = './exist-test';
my @tests;

my ($proto) = setup_proto();
my ($source_host, $source_file, $local_copy) = setup_remote_source();

sub check_existence
{
    my ($errors,$rc) = ("",0);
    my $src_url = shift;
    my $existence_rc = shift;

    my $command = "$test_exec -s $src_url";
    $errors = run_command($command, $existence_rc);
    ok($errors eq "", "check_existence $command");
}

if(source_is_remote())
{
    print "using remote source, skipping check_existence()\n";
}
else
{
    my $emptydir = File::Temp::tmpnam();
    my @test_dirs;

    if(!defined($ENV{'FTP_TEST_BACKEND'}))
    {
        mkdir $emptydir, 0755;
    }

    push(@test_dirs, $emptydir);
    push(@test_dirs, '/etc/no-such-file');
    push(@test_dirs, '/etc');
    push(@test_dirs, '/');
    push(@test_dirs, '/etc/group');

    foreach('/etc/group', '/', '/etc', '/etc/no-such-file', $emptydir)
    {
        my $exists_rc = stat($_) ? 0 : 1;
        
        push(@tests, "check_existence('$proto$source_host$_', $exists_rc);");
    }

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

    rmdir $emptydir;
}
