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

#include "common/ArgParse.h"

#include "lib/BGMasterClientApi.h"
#include "lib/exceptions.h"
#include "lib/ListAgents.h"

#include <utility/include/Log.h>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include <csignal>

LOG_DECLARE_FILE( "master" );

BGMasterClient client;
Args* pargs;

void
doStat(
        bool fancy
        )
{
    int mpid = 0;
    std::string start_time = "";
    std::string version = "";

    if (fancy) {
        std::vector<std::string> idles;
        try {
            client.idle_aliases(idles);
        } catch (exceptions::BGMasterError& e) {
            std::cerr << "Failed to find aliases, error is: " << e.what() << std::endl;
        } catch (std::runtime_error& e) {
            std::cerr << "Failed to find aliases, error is: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        if (!idles.empty()) {
            std::cout << "Aliases not currently running" << std::endl;
            std::cout << "-----------------------------" << std::endl;
            BOOST_FOREACH(std::string& al, idles) {
                if (al != "bgmaster_server") {
                    std::cout << al << std::endl;
                }
            }
        } else {
            std::cout << "All configured aliases running." << std::endl;
        }
    }

    try {
        mpid = client.master_status(start_time, version);
    } catch (exceptions::BGMasterError& e) {
        std::cerr << "master_status failed, error is: " << e.what() << std::endl;
    } catch (std::runtime_error& e) {
        std::cerr << "master_status failed, error is: " << e.what() << std::endl;
    }
    if (mpid > 0) {
        ListAgents::doListAgents(client, false, fancy);
        std::cout << std::endl << version << " running under pid " << mpid
                  << " since " << start_time << std::endl;
    } else {
        std::cerr << "bgmaster_server process id unavailable" << std::endl;
    }

}

void
help()
{
    std::cerr << "Return the process id of bgmaster_server and information about managed processes." << std::endl;
}

void
usage()
{
    std::cerr << "master_status [ --properties filename ] [ --help ] [ --host host:port ] [ --verbose verbosity ]" << std::endl;
}

int main(int argc, const char** argv)
{
    std::vector<std::string> validargs;
    std::vector<std::string> singles;
    std::string fancyarg = "--fancy"; // hidden for backwards compatibility
    std::string uglyarg = "--normal";
    singles.push_back(fancyarg);
    singles.push_back(uglyarg);
    Args largs(argc, argv, &usage, &help, validargs, singles);
    pargs = &largs;
    client.initProperties(pargs->get_props());

    // assume fancy by default
    bool fancy = true;
    if (largs.find_arg(uglyarg)) {
        fancy = false;
    }

    try {
        client.connectMaster(pargs->get_portpairs());
    } catch (exceptions::CommunicationError& e) {
        std::cerr << "Unable to contact bgmaster_server, server may be down." << std::endl;
        exit(1);
    }
    doStat(fancy);
}
