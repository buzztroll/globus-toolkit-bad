#! /usr/bin/perl



use strict;
use File::Basename;
use lib dirname($0);
use TAP::Harness;

use vars qw(@tests);

my $args = {'lib' => [ dirname($0) ]};
my $harness = TAP::Harness->new($args);
my $testdir = dirname($0);
$ENV{PATH} = ".:".dirname($0).":$ENV{PATH}";
print "ENV{PATH} = $ENV{PATH}\n";

@tests = (
            "$testdir/basic-test.pl",
            "$testdir/close-barrier-test.pl",
            "$testdir/close-cancel-test.pl",
            "$testdir/failure-test.pl",
            "$testdir/read-barrier-test.pl",
            "$testdir/timeout-test.pl",
            "$testdir/cancel-test.pl",
            "$testdir/random-test.pl",
            "$testdir/server-test.pl",
            "$testdir/verify-test.pl",
            "$testdir/attr-test.pl",
            "$testdir/space-test.pl",
            "$testdir/server2-test.pl",
            "$testdir/block-barrier-test.pl",
            "$testdir/stack-test.pl",
            "$testdir/unload-test.pl",
            "$testdir/http-header-test.pl",
            "$testdir/http-post-test.pl",
            "$testdir/http-put-test.pl",
            "$testdir/http-get-test.pl"
            );

my $runserver;
my $server_pid;

$ENV{'XIO_TEST_OUPUT_DIR'}="test_output/$$";

my $test_dir=$ENV{'XIO_TEST_OUPUT_DIR'};

system("rm -rf $test_dir");

$harness->runtests(@tests);
