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
/* (C) Copyright IBM Corp.  2010, 2011                              */
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

#ifndef TCPLISTEN_H
#define TCPLISTEN_H

// Simple TCP communications - listen() side.
// 09 Feb 2001 Derek Lieber.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

// Create a "listener" socket.
// Taken:    port number to listen on
// Returned: socket (-1 = error)
//
static inline int
TCPcreateListener(unsigned short localPort, unsigned int interface=INADDR_ANY, int backlog=1)
   {
   // create a socket
   //
   int listenerFd = socket(AF_INET, SOCK_STREAM, 0);
   if (listenerFd == -1)
      {
      perror("TCPlisten: socket");
      return -1;
      }

   // allow bind to work even if other connections are still active on same port
   // ( this prevents spurious "bind: Address already in use" errors if previous incarnations
   // of this process are still alive )
   //
   int enableReuseAddr = 1;
   if (setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &enableReuseAddr, sizeof(enableReuseAddr)) == -1)
      {
      perror("TCPlisten: SO_REUSEADDR");
      close(listenerFd);
      return -1;
      }

   // publish it on network
   //
   sockaddr_in localAddress;
   localAddress.sin_family      = AF_INET;
   localAddress.sin_addr.s_addr = htonl(interface);
   localAddress.sin_port        = htons(localPort);
   if (bind(listenerFd, (sockaddr *)&localAddress, sizeof(localAddress)) == -1)
      {
      perror("TCPlisten: bind");
      close(listenerFd);
      return -1;
      }

   // make it a "listener"
   //
   if (listen(listenerFd, backlog) == -1)
      {
      perror("TCPlisten: listen");
      close(listenerFd);
      return -1;
      }

   return listenerFd;
   }



// Newer constructor: Will pass back the ip addr and port of the
// machine making the connection.

static inline int
TCPaccept(int listenerFd, char *desc, int descBufferLen)
   {
   sockaddr_in remoteAddress;
   socklen_t len = sizeof(remoteAddress);
   int connectionFd = -1;

   for (;;)
      {
      connectionFd = accept(listenerFd, (sockaddr *)&remoteAddress, &len);
      if (connectionFd != -1)
         break;

      if (errno == EINTR)
         continue;

      perror("TCPaccept: accept");
      return -1;
      }

   assert(len == sizeof(remoteAddress));
   assert(remoteAddress.sin_family == AF_INET);

   // force data written to socket to be sent immediately, rather than delaying in order to coalesce packets
   //
   int enableNoDelay = 1;
   if (setsockopt(connectionFd, IPPROTO_TCP, TCP_NODELAY, &enableNoDelay, sizeof(enableNoDelay)) == -1)
      {
      perror("TCPlisten: TCP_NODELAY");
      close(connectionFd);
      return -1;
      }

   if ( desc != NULL && descBufferLen > 0 ) {
     snprintf( desc, descBufferLen, "%s:%d", inet_ntoa( remoteAddress.sin_addr ), ntohs( remoteAddress.sin_port ) );
     desc[descBufferLen-1] = '\0';
   }

   return connectionFd;
   }



// Accept a connection on a listener socket.
// Taken:    socket to listen on
// Returned: socket connected to remote client (-1 = error)
//

// Original constructor.  Does not pass back the ip addr and port of
// the machine making the connection.  New code and upgraded code
// should use the other constructor so that we can create better error
// messages

static inline int
TCPaccept(int listenerFd)
   {
   return TCPaccept( listenerFd, NULL, 0 );
   }




// Wait for multiple remote processes to connect to this one.
// Taken:    port number to listen on
//           number of connections to wait for
//           place to put connection file descriptors
// Returned: 0=success, -1=error
// See also: TCPconnect()
//
static inline int
TCPlisten(unsigned short localPort, unsigned numConnectionFds, int connectionFds[])
   {
   // create listener socket
   //
   int listenerFd = TCPcreateListener(localPort);
   if (listenerFd == -1)
      return -1;

   // wait for remote processes to connect to it
   //
   for (unsigned connectionIndex = 0; connectionIndex < numConnectionFds; ++connectionIndex)
      {
      connectionFds[connectionIndex] = TCPaccept(listenerFd);
      if (connectionFds[connectionIndex] == -1)
         {
         for (unsigned i = 0; i < connectionIndex; ++i)
            close(connectionFds[i]);
         close(listenerFd);
         return -1;
         }
      }

   // we won't be accepting any more connections, so abandon listener socket
   //
   if (close(listenerFd))
      {
      perror("TCPlisten: close");
      for (unsigned i = 0; i < numConnectionFds; ++i)
         close(connectionFds[i]);
      return -1;
      }
   
   return 0;
   }

// Wait for a single remote process to connect to this one.
// Taken:    port number to wait on
// Returned: connection file descriptor (-1=error)
// See also: TCPconnect()
//
static inline int
TCPlisten(unsigned short localPort)
   {
   int connectionFd;
   if (TCPlisten(localPort, 1, &connectionFd))
      return -1;
   return connectionFd;
   }

#endif /* TCPLISTEN_H */
