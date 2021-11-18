#! /usr/bin/perl


use strict;
use warnings;
use Test::More;
use IPC::Open2 qw(open2);

my @tests;
my $test_exec="./http_throughput_test";

plan tests => 1;

my $result;
my ($client_in, $client_out, $server_in, $server_out);
my ($client_pid, $server_pid);

$client_pid = open2($client_out, $client_in, $test_exec, '-c');
$server_pid = open2($server_out, $server_in, $test_exec, '-s');

my $input = <$server_out>;
close($server_in);
close($server_out);
if (!$input) {
    ok(0, $_->[2]);
    close($client_in);
    close($client_out);
    waitpid($server_pid, 0);
    waitpid($client_pid, 0);
    exit(1);
}

print $client_in $input;
close($client_in);
local ($/);
waitpid($server_pid, 0);
waitpid($client_pid, 0);
$result = <$client_out>;
$result =~ s/\s*$//;
close($client_out);

ok($result eq 'Success', 'http_throughput_test');
