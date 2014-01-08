/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/* ================================================================ */
/*                                                                  */
/* Licensed Materials - Property of IBM                             */
/*                                                                  */
/* Blue Gene/Q                                                      */
/*                                                                  */
/* (C) Copyright IBM Corp.  2007, 2012                              */
/*                                                                  */
/* US Government Users Restricted Rights -                          */
/* Use, duplication or disclosure restricted                        */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/* This software is available to you under the                      */
/* Eclipse Public License (EPL).                                    */
/*                                                                  */
/* ================================================================ */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

// Includes
#include "Kernel.h"
#include "fs/virtFS.h"
#include <limits.h>

//! \brief  Run the readlink system call.
//! \param  r3,r4,...,r8 Syscall parameters.
//! \return CNK_RC_SUCCESS(0) or CNK_RC_FAILURE(errno).

uint64_t sc_readlink(SYSCALL_FCN_ARGS)
{
   char *pathname = (char *)r3;
   char *buf = (char *)r4;
   size_t bufsiz = r5;

   // Check for error conditions.
   if ((bufsiz > SSIZE_MAX) || (bufsiz < 0)) {
      return CNK_RC_FAILURE(EINVAL);
   }

   uint64_t pathlen = validatePathname(pathname);
   if (CNK_RC_IS_FAILURE(pathlen)) {
      return pathlen;
   }

   // Set the length to the maximum so a valid message can be sent to the io node.
   if (bufsiz > PATH_MAX) {
      bufsiz = PATH_MAX;
   }

   TRACESYSCALL(("(I) %s%s: path=\"%s\", buf=0x%p, bufsiz=%lu\n", __func__, whoami(), pathname, buf, bufsiz));

   // Run file system's readlink() method.
   return File_GetFSPtrFromPath(pathname)->readlink(pathname, buf, bufsiz);
}
