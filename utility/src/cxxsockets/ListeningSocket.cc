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

#include "cxxsockets/ListeningSocket.h"

#include "cxxsockets/error.h"
#include "cxxsockets/exception.h"
#include "cxxsockets/FileLocker.h"
#include "cxxsockets/SockAddr.h"
#include "cxxsockets/TCPSocket.h"

#include "Log.h"
       
#include <netinet/tcp.h>

LOG_DECLARE_FILE( "utility.cxxsockets" );

namespace CxxSockets {

ListeningSocket::ListeningSocket(
        const SockAddr& addr, 
        int backlog
        )
{
    CXXSOCKET_LOGGING_DECLARE_FD_MDC;
    FileLocker locker;
    LockFile(locker);
    #ifdef SOCK_CLOEXEC
    _fileDescriptor = socket(addr.family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    #else
    _fileDescriptor = socket(addr.family(), SOCK_STREAM , 0);
    #endif
    if(_fileDescriptor < 0) {
        std::ostringstream errmsg;
        errmsg << "socket() error: " << strerror(errno);
        LOG_ERROR_MSG(errmsg.str());
        throw HardError(errno, errmsg.str());
    }

    LOG_DEBUG_MSG("constructing listener");
        
    pBind(addr);
    if(listen(_fileDescriptor, backlog) == -1) {
        std::ostringstream msg;
        msg << "listen error: " << strerror(errno);
        LOG_ERROR_MSG(msg.str());
        throw HardError(errno, msg.str());
    }
}

int
ListeningSocket::Accept()
{
    CXXSOCKET_LOGGING_DECLARE_FD_MDC;
    SockAddr addr;
    socklen_t addrlen = sizeof(sockaddr_storage);
    #ifdef SOCK_CLOEXEC
    const int newFd = accept4(_fileDescriptor, (sockaddr*)(&addr), &addrlen, SOCK_CLOEXEC);
    #else
    const int newFd = accept(_fileDescriptor, (sockaddr*)(&addr), &addrlen);
    #endif
    if(newFd < 0) {
        std::ostringstream msg;
        msg << "accept error: " << strerror(errno);
        LOG_ERROR_MSG(msg.str());
        if(errno == EINTR || errno == EWOULDBLOCK 
           || errno == EAGAIN || errno == ERESTART)
            throw SoftError(errno, msg.str());
        else
            throw HardError(errno, msg.str());
    }

    const int flag = 1;
    LOG_DEBUG_MSG("Disabling nagle");
    const int ret = setsockopt( newFd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );
    if(ret < 0) {
        throw HardError(errno, "Disabling nagle algorithm (TCP_NODELAY) failed");
    }

    SockAddr my_addr;
    std::string my_host;
    std::string my_sport;
    std::string peer_addr;
    std::string peer_port;

    socklen_t sasz = sizeof(sockaddr_storage);

    if(getsockname(newFd, (sockaddr*)(&my_addr), &sasz) < 0)
        LOG_WARN_MSG("cannot get sock name " << strerror(errno));

    try {
        my_host = my_addr.getHostAddr();
        my_sport = boost::lexical_cast<std::string>(my_addr.getServicePort());
        peer_addr = addr.getHostAddr();
        peer_port = boost::lexical_cast<std::string>(addr.getServicePort());
    } catch (const Error& e) {
        LOG_DEBUG_MSG(e.what());
    }

    LOG_INFO_MSG("Accepted socket descriptor " << newFd << " " 
                 << my_host << ":" << my_sport 
                 << "::" << peer_addr << ":" << peer_port);

    return newFd;
}

bool
ListeningSocket::AcceptNew(
        const TCPSocketPtr& sock
        )
{
    BOOST_ASSERT( sock );
    CXXSOCKET_LOGGING_DECLARE_FD_MDC;
    FileLocker locker;
    LockFile(locker);
    const int fd = Accept();
    sock->replaceFd(fd);

    LOG_INFO_MSG("Accepted socket file descriptor " << fd);
    return true;
}

}
