#! /usr/bin/env perl



use strict;
use POSIX;
use Test::More;
use File::Basename;
use lib dirname($0);

 require "test-common.pl";

my $test_driver;
if(@ARGV == 1)
{
    $test_driver = $ARGV[0];
}
else
{
    die "provide a driver name please";
}

my @tests;
my @todo;
my $test_exec="./framework_test";
my $test_name="framework";

my $inline_finish;
my $buffer_size=2048;
my $c;

# setup different chunk sizes
my @chunk_sizes;
push(@chunk_sizes, "1024");
push(@chunk_sizes, "1924");
push(@chunk_sizes, "2048");

# setup different driver combinations
my @drivers;
push(@drivers, "-D $test_driver");
#push(@drivers, "-D debug -D $test_driver");
#push(@drivers, "-D debug -D $test_driver -D debug");
#push(@drivers, "-D bounce -D $test_driver");
#push(@drivers, "-D $test_driver -D bounce");

sub build_test_list
{
    my $inline_finish="-i";

    for(my $i = 0; $i < 2; $i++)
    {
        foreach(@chunk_sizes)
        {
            my $c = $_;

            foreach(@drivers)
            {
                my $d = $_;
                for(my $write_count = 1; $write_count <= 16; $write_count *= 4)
                {
                    for(my $read_count = 1; $read_count <= 16; $read_count *= 4)
                    {
                        push(@tests, "$test_name -w $write_count -r $read_count -c $c -b $buffer_size $inline_finish $d");
                    }
                }
            }
        }
        $inline_finish="";
    }
}

&build_test_list();
plan tests => scalar(@tests), todo => \@todo;
my $cnt=0;
foreach(@tests)
{
    my $test_str="$test_name.$cnt";
    &run_test("$test_exec $_", $test_str);
    $cnt++;
}
