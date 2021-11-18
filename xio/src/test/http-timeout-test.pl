#! /usr/bin/env perl


use strict;
use Test::More;

my $type = 0;
if(@ARGV == 1)
{
    $type = 1;
}

my @todo;
my $test_exec="./http_timeout_test";
my $data_dir=$ENV{srcdir};

my @arg_sets = (
    "-t client -T 500 -a",
    "-t server -T 500 -r",
    "-t client -T 500 -r");

plan tests => scalar(@arg_sets);

foreach (@arg_sets)
{
    my $result;
    my $case=$_;
    my $clientorserver = $case;
    my $which = $case;

    $clientorserver =~ m/(client|server)/;
    $clientorserver = $1;
    $which =~ m/-(a|r)/;
    $which = $1;

    $result = system("$test_exec $case");

    ok($result == 0, "http_timeout_${clientorserver}_${which}");
}
