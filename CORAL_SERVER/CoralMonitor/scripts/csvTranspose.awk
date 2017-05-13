#----------------------------------------------------------------------------
# BEGIN
#----------------------------------------------------------------------------
BEGIN { 
  if ( verb ) system( "echo DEBUG Enter BEGIN > /dev/stderr" );
  status = 0;
  nRow = 0;
  nCol = 0;
  csv[0,0] = "";
  ###unknownHeader = 0;
  if ( verb ) system( "echo DEBUG Exit BEGIN > /dev/stderr" );
}
#----------------------------------------------------------------------------
# Load file contents into the matrix
#----------------------------------------------------------------------------
{ 
  n = split( $0, v, "," );  
  if ( v[n] == "" ) n--; # IGNORE TRAILING COMMA
  if ( verb ) system( "echo DEBUG Row has "n" columns > /dev/stderr" );
  if ( n == 0 )
  {
    system( "echo 'ERROR: could not split csv line \""$0"\"' > /dev/stderr" );
    status = 1;
    exit status;
  }
  if ( nCol == 0 ) # FIRST ROW
  {
    nCol = n;
    if ( verb ) system( "echo DEBUG New nCol="nCol" > /dev/stderr" );
  }
  else 
  {
    if ( strict && n != nCol ) 
    {
      system( "echo 'ERROR: found "n" rows (expected "nCol")' >/dev/stderr" );
      status = 1;
      exit status;
    } 
    if ( !strict && n > nCol ) 
    {
      nCol = n;
      if ( verb ) system( "echo DEBUG New nCol="nCol" > /dev/stderr" );
    }
  }
  nRow++;
  for ( iCol=1; iCol <= n; iCol++ )
  {
    csv[nRow,iCol] = v[iCol];
  }  
}
#----------------------------------------------------------------------------
# END
#----------------------------------------------------------------------------
END { 
  if ( verb ) system( "echo DEBUG Enter END > /dev/stderr" );
  if ( status == 0 )
  {
    for ( iCol=1; iCol <= nCol; iCol++ )
    {
      row = "";
      row2 = ""; # Only used for timestamps (split into date and time)
      isTime = 0;
      for ( iRow=1; iRow <= nRow; iRow++ )
      {
        if ( ( iRow, iCol ) in csv )
        {
          cell = csv[iRow,iCol];
        }
        else 
        {
          if ( iRow == 1 ) 
          {
            cell = "UNKNOWN"; # HEADER
            ###if ( unknownHeader == 0 ) system( "echo ERROR! Header "iRow" "iCol" not found>/dev/stderr" );
            ###unknownHeader = 1;
            system( "echo ERROR! Header "iRow" "iCol" not found>/dev/stderr" );
          }
          else
          {
            cell = "N/A";
            ###system( "echo DEBUG Cell "iRow" "iCol" not found>/dev/stderr" );
          }
        }
        if ( iRow == 1 )
        {
          head = cell; # HEADER
          if ( cell == "timestamp" ) 
          {
            isTime=1;
            row = row "date";
            row2 = row2 "time";
          }
          else
          {
            row = row cell;
          }          
        }
        else 
        {
          if ( isTime ) 
          {
            if ( cell == "timestamp" ) 
            {
              cell = "date";
              cell2 = "time";
            }
            else if ( split( cell, v, " " ) == 2 )  
            {
              if ( split( v[1], v2, "-" ) == 3 ) cell = v2[1] v2[2] v2[3];
              else cell = "'"v[1]"'";
              cell2 = v[2];
            }
            else
            {
              cell = "'"cell"'";
              cell2 = "N/A"; # Split of "date time" failed
            }
            row = row "," cell;
            row2 = row2 "," cell2;
          }
          else
          {
            row = row "," cell;
          }
        }
      }
      if ( noHeader!=1 ) 
      {
        if ( isTime ) 
        {
          row = row ",date";
          row2 = row2 ",time";
        }
        else
        {
          row = row "," head; # ADD HEADER AT THE END
        }
      }      
      print row;
      if ( isTime ) print row2;
    }  
  }  
  if ( verb ) system( "echo DEBUG Exit END > /dev/stderr" );
}
#----------------------------------------------------------------------------
