#! /usr/bin/perl


my $rc;
my $count = 1;

if(!defined($ENV{FTP_TEST_NO_GSI}))
{
    $count = 2;
}

for(my $i = 0; $i < $count; $i++)
{
    $rc = system('./run-tests.pl -runserver -junit');
    if($rc != 0)
    {
        exit $rc;
    }
    
    if(defined($ENV{GLOBUS_TEST_EXTENDED}))
    {
        $rc = system('./many-server.pl');
        if($rc != 0)
        {
            exit $rc;
        }
    }
    $ENV{FTP_TEST_NO_GSI} = 1;
}
exit $rc;
