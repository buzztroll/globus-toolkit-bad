#! /usr/bin/perl


use strict;
use warnings;
use Test::More;
use IPC::Open2 qw(open2);

my @tests;
my $test_exec="./http_header_test";
my $data_dir=$ENV{TEST_DATA_DIR};

if (! -d $data_dir && -r 'headers')
{
    $data_dir = '';
}
elsif (! -r "${data_dir}headers")
{
    print STDERR "Can't find data.\n";
    exit(99);
}

push(@tests, "${data_dir}headers");
push(@tests, "${data_dir}long-headers");
push(@tests, "${data_dir}multi-line-header");
push(@tests, "${data_dir}multi-headers");

plan tests => scalar(@tests);
foreach my $test_name (@tests)
{
    my $result;
    my ($client_in, $client_out, $server_in, $server_out);
    my ($client_pid, $server_pid);
    $client_pid = open2($client_out, $client_in, $test_exec, '-c', '-f',
            $test_name);
    $server_pid = open2($server_out, $server_in, $test_exec, '-s', '-f',
            $test_name);
    my $input = <$server_out>;
    close($server_in);
    close($server_out);
    if (!$input) {
        ok(0, $_->[2]);
        close($client_in);
        close($client_out);
        waitpid($server_pid, 0);
        waitpid($client_pid, 0);
        next;
    }
    print $client_in $input;
    close($client_in);
    local ($/);
    waitpid($server_pid, 0);
    waitpid($client_pid, 0);
    $result = <$client_out>;
    $result =~ s/\s*$//;
    close($client_out);

    ok($result eq 'Success', $_);
}
