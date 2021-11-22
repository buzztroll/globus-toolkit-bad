#! /usr/bin/env perl



use strict;
use Test::Harness;
use Cwd;
use Getopt::Long;
require 5.005;
use vars qw(@tests);

#$Test::Harness::verbose = 1;

unlink("test_results.txt");

@tests = qw(
            basic-test.pl
            close-barrier-test.pl
            close-cancel-test.pl
            failure-test.pl
            read-barrier-test.pl
            timeout-test.pl
            cancel-test.pl
            random-test.pl
            server-test.pl
            verify-test.pl
            attr-test.pl
            space-test.pl
            server2-test.pl
            block-barrier-test.pl
            stack-test.pl
            unload-test.pl
            );

my $runserver;
my $server_pid;

$ENV{'XIO_TEST_OUPUT_DIR'}="test_output/$$";

my $test_dir=$ENV{'XIO_TEST_OUPUT_DIR'};

system("rm -rf $test_dir");

eval runtests(@tests);

$@ && print "$@";

exit 0;
