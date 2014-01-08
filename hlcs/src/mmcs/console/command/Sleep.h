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
/* (C) Copyright IBM Corp.  2012, 2012                              */
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


#ifndef MMCS_CONSOLE_COMMAND_SLEEP_H_
#define MMCS_CONSOLE_COMMAND_SLEEP_H_


#include "common/AbstractCommand.h"


namespace mmcs {
namespace console {
namespace command {


/*!
** sleep
** sleep <seconds>
** Pause for the specified number of seconds
*/
class Sleep : public common::AbstractCommand
{
public:
    Sleep(const char* name, const char* description, const Attributes& attributes)
      : AbstractCommand(name,description,attributes) { usage = "sleep <seconds>" ;}
    static  Sleep* build();    // factory method
    static  std::string cmdname() { return "sleep"; }
    void execute(std::deque<std::string> args,
             mmcs_client::CommandReply& reply,
             common::ConsoleController* pController,
             server::BlockControllerTarget* pTarget=NULL);
    bool checkArgs(std::deque<std::string>& args) { if(args.size() != 1) return false; return true; }
    void help(std::deque<std::string> args,
              mmcs_client::CommandReply& reply);
};


} } } // namespace mmcs::console::command


#endif
