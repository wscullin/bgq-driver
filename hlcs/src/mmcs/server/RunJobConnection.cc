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


#include "RunJobConnection.h"

#include "common/Properties.h"

#include <utility/include/cxxsockets/PollingSocketSet.h>
#include <utility/include/cxxsockets/SecureTCPSocket.h>
#include <utility/include/cxxsockets/SockAddrList.h>
#include <utility/include/Log.h>

#include <boost/foreach.hpp>

#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>


using mmcs::common::Properties;


LOG_DECLARE_FILE("mmcs.server");


namespace {

const int RJ_POLL_TIME = 10; // Ten ms.

}


namespace mmcs {
namespace server {


RunJobConnection::RunJobConnection() :
    _sock(),
    _valid( false ),
    _pending_kills(),
    _lock()
{
    LOG_TRACE_MSG( __FUNCTION__ );
}

bool
RunJobConnection::Connect()
{
    LOG_DEBUG_MSG(__FUNCTION__);
    boost::mutex::scoped_lock scoped_lock(_lock);

    _valid = false;
    // connect to runjob_server
    const std::string servname = "24510";
    bgq::utility::ClientPortConfiguration port_config(servname);
    port_config.setProperties(Properties::getProperties(), "runjob.server.commands");
    port_config.notifyComplete();
    bgq::utility::PortConfiguration::Pairs portpairs = port_config.getPairs();
    CxxSockets::SockAddrList masterlist;
    BOOST_FOREACH(const bgq::utility::PortConfiguration::Pair& curr_pair, portpairs) {
        CxxSockets::SockAddrList salist(AF_UNSPEC, curr_pair.first, curr_pair.second);
        LOG_TRACE_MSG("Adding " << curr_pair.first << ":" << curr_pair.second << " to sockaddr list.");
        // Now copy every SockAddr in to the master list
        BOOST_FOREACH(const CxxSockets::SockAddr& curr_sockaddr, salist) {
            masterlist.push_back(curr_sockaddr);
        }
    }

    bool success = false;
    BOOST_FOREACH(const CxxSockets::SockAddr& addr, masterlist) {
        CxxSockets::SecureTCPSocketPtr sock(new CxxSockets::SecureTCPSocket(addr.family(), 0));
        try {
            bgq::utility::ClientPortConfiguration port_config(0, bgq::utility::ClientPortConfiguration::ConnectionType::Administrative);
            port_config.setProperties(Properties::getProperties(), "");
            port_config.notifyComplete();
            sock->Connect(addr, port_config);
            // Set keepalive parameters.
            sock->setProbe(true, 1, 1, 3);
        } catch (const CxxSockets::Error& e) {
            LOG_DEBUG_MSG("Connect failed to runjob_server on one port. " << e.what());
            sock.reset();
            continue;
        }
        success = true;
        LOG_INFO_MSG("Connected to runjob_server");
        _sock = sock;
        break;
    }

    if(!success) {
        LOG_WARN_MSG( "runjob_server not available" );
        _valid = false;
        return false;
    }

    _valid = true;
    this->sendPending();
    return _valid;
}

int
RunJobConnection::Send(
        const CxxSockets::Message& request,
        CxxSockets::Message& response,
        const uint32_t tag,
        const unsigned int message_length
        )
{
    LOG_DEBUG_MSG(__FUNCTION__);
    try {
        const unsigned int bytes = _sock->SendUnManaged(request);
        if(bytes < message_length) {
            LOG_ERROR_MSG(
                    "Failed sending request to runjob_server. Only " << bytes << " of " <<
                    message_length << " bytes sent."
                    );
            _valid = false;
            return -1;
        }
    } catch (const CxxSockets::Error& e) {
        LOG_ERROR_MSG("Failed sending request to runjob_server: " << e.what());
        return -1;
    }

    runjob::commands::Header respheader;
    CxxSockets::Message headermsg;
    try {
        const unsigned int resp_length = _sock->ReceiveUnManaged(headermsg, sizeof(respheader));
        LOG_TRACE_MSG("Received header of " << resp_length << " bytes.");
        memcpy(&respheader, headermsg.str().c_str(), sizeof(respheader));
        if(resp_length < sizeof(respheader)) {
            LOG_ERROR_MSG(
                    "Received incomplete response header of " << resp_length <<
                    " bytes from runjob_server.  Expected " << sizeof(respheader)
                    );
            _valid = false;
            return -1;
        }
    } catch (const CxxSockets::Error& e) {
        LOG_ERROR_MSG("Failed to receive response from runjob_server: " << e.what());
        _valid = false;
        return -1;
    }
    LOG_TRACE_MSG("Received header message " << respheader);

    if(respheader._type != runjob::commands::Message::Header::Response) {
        LOG_ERROR_MSG("Received incorrectly formatted response from runjob_server.");
        _valid = false;
        return -1;
    }
    if(respheader._tag != tag) {
        LOG_ERROR_MSG("runjob reply is " << respheader._tag << ". Should be " << tag);
        _valid = false;
        return -1;
    }

    try {
        const unsigned int length = _sock->ReceiveUnManaged(response, respheader._length);
        LOG_TRACE_MSG("Received response of " << length << " bytes");
        if(length < respheader._length) {
            LOG_ERROR_MSG(
                    "Failed receiving response from runjob_server.  Received " <<
                    length << " bytes of " << respheader._length << " expected bytes."
                    );
            _valid = false;
            return -1;
        }
    }
    catch (const CxxSockets::Error& e) {
        LOG_ERROR_MSG("Failed to receive response from runjob_server: " << e.what());
        _valid = false;
        return -1;
    }
    return 0;
}

void
RunJobConnection::Kill(
        const BGQDB::job::Id jobid,
        const int signal,
        const int recid
        )
{
    LOG_INFO_MSG("killing job " << jobid << " with signal " << signal << " due to RAS event " << recid );

    boost::mutex::scoped_lock scoped_lock(_lock);
    if(!_valid) {
        LOG_ERROR_MSG("Invalid runjob connection object.  Socket not connected.");
        _pending_kills[jobid] = signal;
        return;
    }

    // Generate the request and empty response.
    runjob::commands::request::KillJob kill_request;
    kill_request._job = jobid;
    kill_request._signal = signal;
    kill_request._controlActionRecordId = recid;
    std::ostringstream os;
    kill_request.doSerialize(os);

    // Create the header
    runjob::commands::Header header;
    bzero( &header, sizeof(header) );
    header._type = kill_request.getType();
    header._tag = kill_request.getTag();
    header._length = os.str().length();

    CxxSockets::Message msg;
    char headerbuff[sizeof(header)];
    bzero(headerbuff, sizeof(header));
    memcpy(&headerbuff, &header, sizeof(header));

    // Add the header to the message.
    msg.write(headerbuff, sizeof(header));

    // Now append the message to the header
    msg.write(os.str().c_str(), os.str().length());

    LOG_TRACE_MSG(header);

    CxxSockets::Message responsemsg;
    const int retval = Send(msg, responsemsg, runjob::commands::Message::Tag::KillJob, os.str().length() + sizeof(header));
    if (retval) {
        LOG_WARN_MSG( "send failed: " << retval );
        _pending_kills[jobid] = signal;
        return;
    }

    runjob::commands::response::KillJob kill_response;

    // Deserialize
    std::istringstream is(responsemsg.str());
    boost::archive::text_iarchive ar(is);
    ar & kill_response;

    switch ( kill_response.getError() ) {
        case runjob::commands::error::success:
            LOG_DEBUG_MSG("kill job " << jobid << " successful" );
            break;
        case runjob::commands::error::job_already_dying:
            LOG_DEBUG_MSG("job " << jobid << " already dying" );
            break;
        default:
            LOG_ERROR_MSG(
                    "could not kill job " << jobid << ": " <<
                    kill_response.getMessage() << " (" <<
                    runjob::commands::error::toString( kill_response.getError() ) << ")"
                    );
            break;
    }

    return;
}

bool
RunJobConnection::CheckViability()
{
    LOG_TRACE_MSG("Polling runjob_server connection for errors");
    boost::mutex::scoped_lock scoped_lock(_lock);
    const CxxSockets::PollingSocketSetPtr pset(new CxxSockets::PollingSocketSet);
    try {
        pset->AddSock(_sock, CxxSockets::ERROR);
        if(pset->Poll(RJ_POLL_TIME) != 0) {
            _valid = false;
        }
    } catch (const CxxSockets::SoftError& e) {
        // We should only get interrupted when we are getting shut down.
        // The connection isn't bad so we won't retry.
        _valid = true;
        LOG_DEBUG_MSG("Polling runjob_server connection soft error: " << e.what());
    } catch (const CxxSockets::HardError& e) {
        _valid = false;
        LOG_ERROR_MSG("Failed runjob_server connection polling: " << e.what());
    }
    LOG_TRACE_MSG("runjob_server connection polling complete");
    return _valid;
}

void
RunJobConnection::sendPending()
{
    LOG_DEBUG_MSG(__FUNCTION__);

    // we assume the mutex remains locked from Connect here

    if ( _pending_kills.empty() ) return;
    LOG_DEBUG_MSG( "sending " << _pending_kills.size() << " pending signals" );

    const Container kills( _pending_kills );
    _pending_kills.clear();
    for( Container::const_iterator i = kills.begin(); i != kills.end(); ++i ) {
        this->Kill(i->first, i->second);
    }
}

void*
RunJobConnectionMonitor::threadStart()
{
    LOG_TRACE_MSG(__FUNCTION__);
    // Timeouts in usecs.  1 usec is 10^-6 seconds.
    const int init_wait = 250; // This is where we start.
    const int max_wait = 3000000; // Three second max wait time.
    int curr_wait = init_wait; // When backing off, this is the current timeout.
    while(isThreadStopping() == false) {
        // As long as we think we're connected, we'll keep checking to see if we stay that way.
        // We make reconnection attempts until we get connected or we are killed.  To be
        // sensible about it, we start fast and slow down to a reasonable periodicity.
        // Our normal connection polling time is reasonably fast, but not a CPU killer.
        // The total normal wait is init_wait + RJ_POLL_TIME
        bool reconnect = false;
        if(RunJobConnection::instance().isConnected()) {  // We think we're connected.
            if(RunJobConnection::instance().CheckViability() == false) {  // So we check it.
                curr_wait = init_wait; // Not connected, start retrying right away.
                reconnect = true;  // But it's broken so we need to reconnect.
            } else {
                curr_wait = max_wait;  // Or, we have a good connection, don't need to repoll so soon.
                LOG_TRACE_MSG("Connection to runjob_server validated.");
            }
        } else {  // We know we're not connected!
            // Don't adjust the wait time because we're doing the backoff here.
            reconnect = true;
        }

        if(reconnect) {
            LOG_WARN_MSG("The runjob_server appears to be down.  Will attempt reconnection.");
            if(RunJobConnection::instance().Connect() == true) {  // Try to connect.
                curr_wait = max_wait;  // Connection is now good, poll slow.
            } else {  // Connection failed, try again in a bit.
                curr_wait *= 3;
                if(curr_wait > max_wait)
                    curr_wait = max_wait;
            }
        }

        usleep(curr_wait);
    }
    return 0;
}

RunJobConnectionMonitor::RunJobConnectionMonitor()
{
    LOG_INFO_MSG( "starting" );
}

} } // namespace mmcs::server