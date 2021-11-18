use Globus::Core::Paths;

package Globus::Core::Config;

sub new
{
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self = {};
    my $path = shift;
    my $fh;
    my $line;

    $path = Globus::Core::Paths::eval_path($path);

    if (! -f $path)
    {
        return undef;
    }

    open($fh, "<$path");

    # Odd parsing algorithm lifted from C code. See globus_common_paths.c
    while ($line = <$fh>)
    {
        # Remove leading whitespace
        $line =~ s/^[ \t]*//;

        # Process anything that's an attr=.* line
        if ($line =~ m/([^=]*)=(.*)/)
        {
            my $attr = $1;
            my $value = $2;

            # Remove single leading double quote if present
            $value =~ s/^"//;
            # Remove all trailing space, tab, newline and quotes
            $value =~ s/[ \t"]*$//;
            $self->{$attr} = $value;
        }
    }
    bless $self, $class;

    return $self;
}

sub get_attribute
{
    my $self = shift;
    my $attribute = shift;

    if (exists $self->{$attribute})
    {
        return $self->{$attribute};
    }
    else
    {
        return undef;
    }
}

1;
