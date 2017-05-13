#----------------------------------------------------------------------------
# BEGIN - check the input arguments
#----------------------------------------------------------------------------
BEGIN { 
  ###system( "echo DEBUG Enter BEGIN > /dev/stderr" );
  if ( infile == 1 ) {
    copy=1;
    split( tests2, tests2a );
    ###system( "echo 'DEBUG Processing file #1' > /dev/stderr" );
    ###system( "echo 'DEBUG Input2=" input2 "' > /dev/stderr" );
    ###system( "echo 'DEBUG Tests2=" tests2 "' > /dev/stderr" );
    ###system( "echo 'DEBUG Tests2 as array - START' > /dev/stderr" );
    ###for ( t in tests2a ) system( "echo '" tests2a[t] "' > /dev/stderr" );
    ###system( "echo 'DEBUG Tests2 as array - END' > /dev/stderr" );
    if ( input2 == "" ) {
      system( "echo 'ERROR: wrong or missing input2' > /dev/stderr" );
      exit 1;
    }
  }
  else if ( infile == 2 ) {
    ###system( "echo 'DEBUG Processing file #2' > /dev/stderr" );
    ###system( "echo 'DEBUG Group=" group "' > /dev/stderr" );
    ###system( "echo 'DEBUG Tech=" tech "' > /dev/stderr" );
    ###system( "echo 'DEBUG Test=" test "' > /dev/stderr" );
    copy=0;
    if ( group == "" ) {
      system( "echo 'ERROR: wrong or missing group' > /dev/stderr" );
      exit 1;
    }
    if ( tech == "" ) {
      system( "echo 'ERROR: wrong or missing tech' > /dev/stderr" );
      exit 1;
    }
    if ( test == "" ) {
      system( "echo 'ERROR: wrong or missing test' > /dev/stderr" );
      exit 1;
    }
  }
  else {
    system( "echo 'ERROR: wrong or missing infile' > /dev/stderr" );
    exit 1;
  }
  ###system( "echo DEBUG Exit BEGIN > /dev/stderr" );
}
#----------------------------------------------------------------------------
# File #1: define the tests to skip
# File #2: define the tests to merge (one at a time)
#----------------------------------------------------------------------------
{ 
  if ( $1 == "<result" ) {
    nf = split( $2, array, "\"" );
    if ( nf != 3 ) {
      system( "echo 'ERROR: wrong #fields (exp 3)' > /dev/stderr" );
      exit 1;
    }
    this = array[2];
    nf = split( this, array, "." );
    if ( nf == 2 ) {
      thisGroup = array[1];
      thisTech = "no_db";
      thisTest = array[2];
    } 
    else if ( nf == 3 ) {
      thisGroup = array[1];
      thisTech = array[2];
      thisTest = array[3];
    } 
    else if ( nf == 4 ) {
      thisGroup = array[1];
      thisTech = array[2];
      thisTest = array[3]"."array[4];
      ###system( "echo DEBUG \"" thisGroup " " thisTech " " thisTest "\" > /dev/stderr" );
    } 
    else {
      system( "echo 'ERROR: wrong #fields (exp 2-4)' > /dev/stderr" );
      ###system( "echo DEBUG \"" this "\" > /dev/stderr" );
      exit 1;
    }
    ###system( "echo DEBUG \"" thisGroup " " thisTech " " thisTest "\" > /dev/stderr" );
    # Reinitialise at every new test output (default: copy file 1, skip file 2)
    if ( infile == 1 ) copy=1; else copy=0;
    # Define the tests to merge (copy file 2, skip file 1)
    if ( infile == 1 ) {
      ###system( "echo DEBUG this=" this " > /dev/stderr" );
      mergeThis=0;
      for ( t in tests2a ) { if ( this == tests2a[t] ) mergeThis=1; }
      if ( mergeThis == 1 )                                       # DEFAULT
      # if ( ( thisTest == "testX" ) && ( thisTech == "techX" ) ) # TEMPLATE
      # if ( thisTech == "sqlite" || thisTech == "frontier" )     # EXAMPLE
      # if ( thisTech == "sqlite" )                               # EXAMPLE
      # if ( thisGroup == "coolkernel" )                          # EXAMPLE
      # if ( thisTest == "evolution_130" )                        # EXAMPLE
      # if ( this == "relationalcool.sealplugindump" )            # EXAMPLE
      {
	copy=0; 
	cmd="awk -f mergeTestOutput.awk -v infile=2 -v group=" thisGroup " -v tech=" thisTech " -v test=" thisTest " " input2;
	###system( "echo DEBUG copy=" copy " > /dev/stderr" );
	###system( "echo DEBUG " cmd " > /dev/stderr" );
	system(cmd);
      }
      else {
	###system( "echo DEBUG copy=" copy " > /dev/stderr" );
      }
    }
    else {
      if ( thisGroup == group && thisTech == tech && thisTest == test ) {
	###system( "echo DEBUG OK > /dev/stderr" );
	copy=1;
      } 
      else {
	###system( "echo DEBUG NOT OK > /dev/stderr" );
      }
    }    
  }
}
#----------------------------------------------------------------------------
# File #1: do nothing
# File #2: avoid copying the last two lines
#----------------------------------------------------------------------------
{
  if ( infile == 2 && $1 == "</results>" ) copy=0;
}
#----------------------------------------------------------------------------
# File #1: copy data
# File #2: copy data
#----------------------------------------------------------------------------
{ 
  if ( copy == 1 ) print $0;
}
#----------------------------------------------------------------------------
# END - noop
#----------------------------------------------------------------------------
END { 
  ###system( "echo DEBUG Enter END > /dev/stderr" );
  ###system( "echo DEBUG Exit END > /dev/stderr" );
}
#----------------------------------------------------------------------------
