#!/usr/bin/perl


require 5.005;

use warnings;
use strict;
use Test::Harness;
use vars qw(@tests);

@tests = qw( globus-io-file-test.pl
             globus-io-authorization-test.pl
             globus-io-tcp-test.pl
             );

runtests(@tests);

