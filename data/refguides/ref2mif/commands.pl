
sub commands { 
    chomp ;
    s/\t/ /g ; 
    xf_output ( "\n#= '$_'\n" ) if $opt_v ; 
    my $ignored ; 
    if ( /^\s*$/ ) { 
	# ignore
	$ignored++ ;
	if ( $ignored >= 2 ) { 
	    $_ = &paragraph("Spacer", "" ) ; 
	    $ignored = 0 ; 
	}
    } elsif ( /^\s*(private|deprecated)/ ) { 
	# just drop these for now
	while ( $_ !~ /^\s*$/ ) { 
	    $_ = xf_input() ; 
	}
	$ignored = 1 ;
    } elsif ( /^\s/ ) { # 
	$ignored = 0 ; 
	s/^\s+// ;
	$_ = &paragraph ( "description", "#\n" . &emphasize(\%Parameters, "ParameterName", $_) ) ;
    } else {
	$ignored = 0 ; 
	undef %Parameters ;  # parse_cdeclarations creates the array %Parameters as a side effect.
	$_ = &paragraph ( "cdeclaration", "#\n" . parse_command($_) ) ;
    }
    return $_ ;
}

sub parse_command { 
    my ($in ) = @_ ; 
    my $result, $name, $instance ; 

    if ( $in =~ /^\s*(\w+)\s+/ ) { 
	$name = $1 ; 
	$in = $' ; 
	$result = &format_type ( "FunctionName", $name  ) ;
	$result .= &parse_command_arguments($in) ; 

    } elsif ( $in =~ /^\s*\(\s*(\w+)\s*\)\s+(\w+)/ ) { 
	$instance = $1 ; 
	$name = $2 ;
	$in = $' ; 
	$result = &string ( "($instance) ") ;
	$result .= &format_type ( "FunctionName", $name  ) ;
	$result .= &parse_command_arguments($in) ; 
    } else { 
	xf_warn ( "parse_command doesn't understand '$in'" ) ; 
	$result = &string ( $in ) ; 
    }
    return $result ; 
}

sub parse_command_arguments { 
    my ($in) = @_ ; 
    my $result, $name ; 
    $result = "" ;
    my $left, $match ; 
    while ( $in =~ /(-?\w+)/ ) { 
	$in = $' ; 
	$match = $1 ;
	$left = $` ;
	if ( $match =~ /^-/ ) { 
	    $result .= &string(" $left $match") ;
	} else { 
	    $result .= &string(" $left") . &format_type("ParameterName", $match) ; 
	}
    }
    $result .= &string($in) if $in !~ /^\s*$/ ; 
    return $result ;
}

1;
