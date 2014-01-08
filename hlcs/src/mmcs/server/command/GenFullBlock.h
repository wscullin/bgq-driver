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


#ifndef MMCS_SERVER_COMMAND_GEN_FULL_BLOCK_H_
#define MMCS_SERVER_COMMAND_GEN_FULL_BLOCK_H_


#include "common/AbstractCommand.h"


namespace mmcs {
namespace server {
namespace command {


/*
** gen_full_block <blockid>
** Generate a block for the entire machine.
*/
class GenFullBlock : public common::AbstractCommand
{
public:
    GenFullBlock (const char* name, const char* description, const Attributes& attributes)
    : AbstractCommand(name,description,attributes) { usage = "gen_full_block <blockid>";}
    static  GenFullBlock* build();    // factory method
    static  std::string cmdname() { return "gen_full_block"; }
    void execute(std::deque<std::string> args,
             mmcs_client::CommandReply& reply,
             common::ConsoleController* pController,
             BlockControllerTarget* pTarget,
             std::vector<std::string>* validnames);
    void execute(std::deque<std::string> args,
             mmcs_client::CommandReply& reply,
             common::ConsoleController* pController,
             BlockControllerTarget* pTarget);
    bool checkArgs(std::deque<std::string>& args) { if(args.size() != 1) return false; else return true;  }
    void help(std::deque<std::string> args,
              mmcs_client::CommandReply& reply);
};


} } } // namespace mmcs::server::command


#endif
