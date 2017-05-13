#----------------------------------------------------------------------------
# BEGIN - check the input arguments
#----------------------------------------------------------------------------
BEGIN {
  ###system( "echo DEBUG Enter BEGIN > /dev/stderr" );
  follows_timeout_no  = "";
  follows_timeout_yes = "TIMED OUT!";
  follows_timeout = follows_timeout_no;
  curr_opened=0;
  n_opened[0]=0;
  open_user[0]="UNKNOWN_USER";
  ###open_reqs[0]="UNKNOWN_REQS";
  open_args[0]="UNKNOWN_ARGS";
  open_date[0]="yyyy/mm/dd_hh:MM:ss";
  curr_date=0; 
  n_curr_date=0; # Add fake ms to sort entries at the same second
  printReq=1; # QUIET (0) vs DEBUG (1)
  ###system( "echo DEBUG Exit BEGIN > /dev/stderr" );
}
#----------------------------------------------------------------------------
# Function - time difference in seconds (date format: 'yyyy/mm/dd_hh:MM:ss')
#----------------------------------------------------------------------------
function delta( date2, date1 )
{
  # Decode date1
  year1  = substr( date1, 0, 4 );
  month1 = substr( date1, 6, 2 );
  day1   = substr( date1, 9, 2 );
  hour1  = substr( date1, 12, 2 );
  min1   = substr( date1, 15, 2 );
  sec1   = substr( date1, 18, 2 );
  secs1  = hour1*3600 + min1*60 + sec1;
  # Decode date2
  year2  = substr( date2, 0, 4 );
  month2 = substr( date2, 6, 2 );
  day2   = substr( date2, 9, 2 );
  hour2  = substr( date2, 12, 2 );
  min2   = substr( date2, 15, 2 );
  sec2   = substr( date2, 18, 2 );
  secs2  = hour2*3600 + min2*60 + sec2;
  if ( year2!=year1 || month2!=month1 || day2!=day1 ) secs2+=86400;
  ###print date1, year1, month1, day1, hour1, min1, sec1;
  ###print date2, year2, month2, day2, hour2, min2, sec2;
  secs = secs2-secs1;
  secs = substr( 100000+secs, 2 );
  return secs;
}
#----------------------------------------------------------------------------
# Function - update the #entries with this date
#----------------------------------------------------------------------------
function checkdate( date )
{
  if ( date != curr_date ) 
  {
    curr_date=date;
    n_curr_date=0;
  }    
  else 
  {
    n_curr_date++;
  }
}
#----------------------------------------------------------------------------
# Analyse opendir and closedir requests
# Analyse starttrans and endtrans requests
# Analyse startsess and endsess requests
#----------------------------------------------------------------------------
{
  if ( $0 != "" )
  {
    # Identify the thread
    nf = split( $3, pid_thread, "," );
    if ( nf != 2 )
    {
      print "ERROR '"$0"'";
      system( "echo 'ERROR: wrong #fields (exp 2)' > /dev/stderr" );
      #exit 1;
    }
    thread = pid_thread[1] "," substr( 10000+pid_thread[2], 2 );
    # Identify the date/time
    if ( year == 0 ) year=2010;
    date = year "/" $1 "_" $2;
    # Select opendir and closedir requests
    # Select starttrans and endtrans requests
    # Select startsess and endsess requests
    # Dump closing requests (closedir, endtrans, endsess)
    if ( $8 == "request" &&
         ( $7 == "opendir" || $7 == "closedir" ||
           $7 == "starttrans" || $7 == "endtrans" ||
           $7 == "startsess" || $7 == "endsess" ) )
    {
      iword=10;
      user = $iword;
      while ( iword++ < NF-3 ) user=user"_"$iword;
      cn = index( user, "CN=" );
      if ( cn > 0 ) user = substr( user, cn );
      # --- CLOSING REQUESTS ---
      if ( $7 == "closedir" || $7 == "endtrans" || $7 == "endsess" )
      {
        ###print "OpenArgs[" thread "]='" open_args[thread] "'";
        if ( open_args[thread] == open_args[0] ) # THREAD ALREADY FREED???
        {
          elapsed = "error";
        }
        else if ( open_args[thread] == "" ) # THREAD NOT YET SEEN IN LOGFILE!
        {
          open_args[thread]=open_args[0];
          elapsed = "?????";
        }
        ###else if ( open_reqs[thread] == "startsess" &&
        ###          $7 == "endtrans" ) # Transaction within a session
        ###{
        ###  elapsed = "(...)";
        ###}
        else 
        {
          curr_opened--;
          elapsed = delta( date, open_date[thread] );
        }
        request = $7"  ";        
        if ( $7 == "endsess" ) request=request" ";
        if ( printReq == 1 )
        {
          checkdate( date );
          print thread, date "." substr( 1000+n_curr_date, 2 ), 
            elapsed, substr( 1000+curr_opened, 2 ),
            request, user, open_args[thread], follows_timeout;
        }
        # Mark the thread as free
        open_user[thread]=open_user[0];
        ###open_reqs[thread]=open_reqs[0];
        open_args[thread]=open_args[0];
        open_date[thread]=open_date[0];
      }
      # --- INITIAL REQUESTS (1st call, user logged, arguments not logged) ---
      else 
      {
        open_user[thread] = user;
        if ( open_args[thread] == "" ) # THREAD NOT YET SEEN IN LOGFILE!
        {
          ###open_reqs[thread]=open_reqs[0];
          open_args[thread]=open_args[0];
          open_date[thread]=open_date[0];
        }
      }
    }
    # Dump initial requests (opendir, starttrans, startsess)
    # --- INITIAL REQUESTS (2nd call, user not logged, arguments logged) ---
    if ( $5 == "NS098" && 
         ( $7 == "opendir" || $7 == "starttrans" || $7 == "startsess" ) )
    {
      request = $7;
      if ( $7 == "opendir" ) request=request"   ";
      if ( $7 == "startsess" ) request=request" ";
      if ( open_user[thread] == "" ) # THREAD NOT YET SEEN IN LOGFILE?
      {
        open_user[thread]=open_user[0];
        elapsed = "-----";
        n_opened[thread]++;
        curr_opened++;
      }
      ###else if ( open_reqs[thread] == "startsess" &&
      ###          $7 == "starttrans" ) # Transaction within a session
      ###{
      ###  elapsed = "(...)";
      ###}
      else if ( open_args[thread] != open_args[0] ) # THREAD NEVER FREED???
      {
        elapsed = "CHECK";
      }
      else # Normal situation (thread seen in logfile on 1st initial call)
      {
        elapsed = "-----";
        n_opened[thread]++;
        curr_opened++;
      }
      iword=8;
      args = $iword;
      while ( iword++ < NF ) args=args"_"$iword;
      if ( printReq == 1 )
      {
        checkdate( date );
        print thread, date "." substr( 1000+n_curr_date, 2 ), 
          elapsed, substr( 1000+curr_opened, 2 ),
          request, open_user[thread], args;
      }
      open_date[thread]=date;
      open_args[thread]=args;
      ###if ( $7 != "starttrans" || open_reqs[thread] != "startsess" )
      ###  open_reqs[thread]=$7;
    }
    # Select time out errors
    if ( $10 == "Timed" && $11 == "out" )
    {
      ###print thread, date, $7, $10, $11;
      follows_timeout = "(" $7 " " follows_timeout_yes ")";
    }
    else
    {
      follows_timeout = follows_timeout_no;
    }
  }
}
#----------------------------------------------------------------------------
# END - noop
#----------------------------------------------------------------------------
END {
  ###system( "echo DEBUG Enter END > /dev/stderr" );
  for ( thread in n_opened ) 
  {
    if ( thread != "0" ) print "#loopsOpened", thread, n_opened[thread];
  }
  ###system( "echo DEBUG Exit END > /dev/stderr" );
}
#----------------------------------------------------------------------------
