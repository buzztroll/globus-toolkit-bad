#! /usr/bin/perl 



=head1 dir-test

Tests to exercise the size checking of the client library.

=cut

use strict;
use Test::More;
use File::Basename;
use lib dirname($0);
use FtpTestLib;
use File::Spec;

my @tests;

my ($proto) = setup_proto();
my ($source_host, $source_file, $local_copy) = setup_remote_source();

my $source_url="$proto$source_host$source_file";

# remove the file if it is there
run_command("./delete-test -s $source_url", -2);
push(@tests, "run_check('./mkdir-test', '-s', '');");
push(@tests, "run_check('./rmdir-test', '-s', '');");
push(@tests, "run_check('./put-test', '-d', '< $local_copy');");
push(@tests, "run_check('./delete-test', '-s', '');");

sub run_check
{
    my ($errors,$rc) = ("",0);
    my $test_exec = shift;
    my $s_or_d = shift;
    my $input = shift;
    my $checked_size;

    my $command = "$test_exec $s_or_d $source_url $input >".File::Spec::->devnull();
    `$command`;
    $rc = $?;
    if($rc / 256 != 0)
    {
        $errors .= "\n# Test exited with " . $rc / 256;
    }

    ok($errors eq "", "run_check $command");
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
