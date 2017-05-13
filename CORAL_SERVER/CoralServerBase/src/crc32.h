// $Id: crc32.h,v 1.1.2.2 2011/12/10 22:51:26 avalassi Exp $
#ifndef CORALSERVERBASE_CRC32_H
#define CORALSERVERBASE_CRC32_H 1

// Include files
#ifndef _WIN32
#include <stdint.h> // For uint32_t on gcc46 (bug #89762)
#endif
#include <string>
#include <sys/times.h>


namespace coral {
  /*----------------------------------------------------------------------
   *
   *
   *
   *  CRC-32 version 2.0.0 by Craig Bruce, 2006-04-29.
   *
   *  This program generates the CRC-32 values for the files named in the
   *  command-line arguments.  These are the same CRC-32 values used by GZIP,
   *  PKZIP, and ZMODEM.  The Crc32_ComputeBuf() can also be detached and
   *  used independently.
   *
   *  THIS PROGRAM IS PUBLIC-DOMAIN SOFTWARE.
   *
   *  Based on the byte-oriented implementation "File Verification Using CRC"
   *  by Mark R. Nelson in Dr. Dobb's Journal, May 1992, pp. 64-67.
   *
   *  v1.0.0: original release.
   *  v1.0.1: fixed printf formats.
   *  v1.0.2: fixed something else.
   *  v1.0.3: replaced CRC constant table by generator function.
   *  v1.0.4: reformatted code, made ANSI C.  1994-12-05.
   *  v2.0.0: rewrote to use memory buffer & static table, 2006-04-29.
   \*-------------------------------------------------------------------- */


  /**
   *  NAME:
   *     Crc32_ComputeBuf() - computes the CRC-32 value of a memory buffer
   *  DESCRIPTION:
   *     Computes or accumulates the CRC-32 value for a memory buffer.
   *     The 'inCrc32' gives a previously accumulated CRC-32 value to allow
   *     a CRC to be generated for multiple sequential buffer-fuls of data.
   *     The 'inCrc32' for the first buffer must be zero.
   *  ARGUMENTS:
   *     inCrc32 - accumulated CRC-32 value, must be 0 on first call
   *     buf     - buffer to compute CRC-32 value for
   *     bufLen  - number of bytes in buffer
   *  RETURNS:
   *     crc32 - computed CRC-32 value
   *  ERRORS:
   *     (no errors are possible)
   */

  uint32_t Crc32_ComputeBuf( uint32_t inCrc32, const void *buf, size_t bufLen );

}
#endif // CORALSERVERBASE_CRC32_H
