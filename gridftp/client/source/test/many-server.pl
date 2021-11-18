# !/usr/bin/perl

use File::Basename;
use lib dirname($0);

use strict;
require 5.005;

require "gfs_common.pl";

my $subject = gfs_setup_security_env();
my $test_ndx = 0;
my $pid;
my $cs;
my $cnt=0;
($pid,$cs,$test_ndx) = gfs_next_test($test_ndx);
while($test_ndx >= 0)
{
    my $rc = system('./globus-ftp-client-run-tests.pl');
    if($rc != 0)
    {
        exit $rc;
    }
    ($pid,$cs,$test_ndx) = gfs_next_test($test_ndx);
}

exit 0;
