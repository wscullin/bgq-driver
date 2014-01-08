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
/* (C) Copyright IBM Corp.  2004, 2011                              */
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

/*!
 * \file MMCSCommand.h
 * \brief
 */

#ifndef _MMCSCommand_H
#define _MMCSCommand_H

#include <string>
#include <deque>
#include <boost/utility.hpp>
#include <hlcs/include/security/Enforcer.h>
#include "BlockControllerBase.h"
#include "ConsoleController.h"
#include <hlcs/include/security/Types.h>
#include "MMCSCommandProcessorStatus.h"
#include "MMCSProperties.h"

class ConsoleController;
class DBConsoleController;
class BlockControllerBase;
class BlockControllerTarget;
class MMCSCommandReply;

enum HELP_CAT {
    DEFAULT = 0,
    USER,
    ADMIN,
    SPECIAL
};

//typedef boost::shared_ptr<SecureBgDiagClient> BgDiagClientPtr;

/*!
 * \class MMCSCommandAttributes
 * \brief Attributes of an MMCS command:
 * requiresBlock -- does the command require a selected BlockController object?
 * requiresTarget -- does the command take a target prefix? (e.g., {*})
 * requiresConnection -- does the command require an active ido connection to the node cards?
 * internalCommand -- is the command for internal use only?
 * mmcsConsoleCommand -- can the command be executed by mmcs_console?
 * mmcsServerCommand -- can the command be executed by mmcs_server?
 * mmcsLiteCommand --  can the command be executed by mmcs_lite?
 * requiresObjNames -- special attribute to ensure that a block argument is sent to the command when a block is selected
 */
class MMCSCommandAttributes
{
public:
    typedef std::pair<hlcs::security::Object::Type, hlcs::security::Action::Type> AuthPair;

    MMCSCommandAttributes()
        : _requiresBlock(false),
          _requiresConnection(false),
          _requiresTarget(false),
          _internalCommand(false),
          _mmcsConsoleCommand(false),
          _mmcsServerCommand(false),
          _mmcsLiteCommand(false),
          _external(false),
          _helpCategory(DEFAULT),
          _bgadminAuth(false),
          _requiresObjNames(false),
          _internalAuth(false),
          _specialAuths("")
    {}
    bool  requiresBlock() { return _requiresBlock; }
    void  requiresBlock(bool tf) { _requiresBlock = tf; }
    bool  requiresTarget() { return _requiresTarget; }
    void  requiresTarget(bool tf) { _requiresTarget = tf; }
    bool  requiresConnection() { return _requiresConnection; }
    void  requiresConnection(bool tf) { _requiresConnection = tf; }
    bool  internalCommand() { return _internalCommand; }
    void  internalCommand(bool tf) { _internalCommand = tf; }
    bool  mmcsConsoleCommand() { return _mmcsConsoleCommand; }
    void  mmcsConsoleCommand(bool tf) { _mmcsConsoleCommand = tf; }
    bool  mmcsServerCommand() { return _mmcsServerCommand; }
    void  mmcsServerCommand(bool tf) { _mmcsServerCommand = tf; }
    bool  mmcsLiteCommand() { return _mmcsLiteCommand; }
    void  mmcsLiteCommand(bool tf) { _mmcsLiteCommand = tf; }
    bool  externalCommand() { return _external; }
    void  externalCommand(bool tf) { _external = tf; }
    void  addAuthPair(AuthPair& ap) { _authmap.push_back(ap); }
    bool  bgadminAuth(bool auth) { auth?_bgadminAuth = true:_bgadminAuth = false;return _bgadminAuth; }
    bool  getBgAdminAuth() { return _bgadminAuth; }
    void  requiresObjNames(bool tf) { _requiresObjNames = tf; }
    bool  requiresObjNames() { return _requiresObjNames; }
    bool  internalAuth(bool auth) { auth?_internalAuth = true:_internalAuth = false;return _internalAuth; }
    bool  getInternalAuth() { return _internalAuth; }
    std::string specialAuthString(std::string auths = "") { if(auths.length() != 0) _specialAuths = auths; return _specialAuths; }
    std::vector<AuthPair>* getAuthPairs() { return &_authmap; }
    HELP_CAT helpCategory() { return _helpCategory; }
    void helpCategory(HELP_CAT hc) { _helpCategory = hc; }
private:
    bool   _requiresBlock;                  //!< is the command valid before a block is allocated?
    bool   _requiresConnection;             //!< is the command valid before a block is connected?
    bool   _requiresTarget;                 //!< does the command use a BlockControllerTarget?
    bool   _internalCommand;                //!< Is this command for internal use only?
    bool   _mmcsConsoleCommand;             //!< can the command be executed by mmcs_console?
    bool   _mmcsServerCommand;              //!< can the command be executed by mmcs_server?
    bool   _mmcsLiteCommand;                //!< can the command be executed by mmcs_lite?
    bool   _external;                       //!< Is this the special "external" command
    HELP_CAT _helpCategory;                 //!< The command's help category
    bool _bgadminAuth;                      //!< Requires bgadmin authority
    bool _requiresObjNames;                 //!< All object names must be passed to this command.
    bool _internalAuth;                     //!< Command that acts on no security objects
    std::string _specialAuths;              //!< Description of special authorizations performed by command
    //! \brief vector of types to associated authorities
    std::vector<AuthPair> _authmap;
};


/*!
 * \brief Abstract base class for MMCS commands.
 *
 * To implement a specific command
 * -- Define a new class derived from MMCSCommand
 * -- Implement the build(), cmdname(), execute(), and help() methods
 * -- Invoke the build() method from mmcs
 * See MMCSCommand_echo.h and MMCSCommand_echo.cc for examples
 */
class MMCSCommand : private boost::noncopyable
{
protected:
public:
        /*!
         ** MMCSCommand constructor
         ** The constructor is normally invoked only by the build() method
         ** @param name the command name
         ** @param desc a brief, one line description
         ** @param attr describes the attributes and requirements of the command
         */
        MMCSCommand(const char* name, const char* description, const MMCSCommandAttributes& attributes)
            : _name(name),
            _description(description),
            _attributes(attributes)
    {}

        /*!
         ** build() - MMCSCommand factory
         ** This is invoked at MMCS startup when MMCS builds its list of commands
         ** @return an MMCSCommand object for this specific command
         */
        static  MMCSCommand* build() { return NULL; }    // factory method

        /*!
         ** cmdname() - Returns the command name
         ** This returns the name of the command. It is a static method so that it
         ** can be used before the object is built, in conjunction with build,
         ** for inserting the command objects into a map.
         ** @return command name
         */
        static  std::string cmdname() { return ""; }

        /*!
         ** MMCSCommand destructor
         */
        virtual ~MMCSCommand() {}

        /*!
         ** execute() - Perform specific MMCS command
         ** This variety of execute() is intended to be executed overridden by BlockController functions
         ** @param args the command arguments
         ** @param reply       the command output stream. Refer to class MMCSCommandReply
         ** @param pController the ConsoleController object that the command is to work on
         ** @param pTarget     the BlockControllerTarget list that the command is to work on (optional)
         */
        virtual void execute(
                std::deque<std::string> args,
                MMCSCommandReply& reply,
                ConsoleController* pController,
                BlockControllerTarget* pTarget,
                std::vector<std::string>* validnames
                )
        {
            reply << FAIL << "Internal error occurred: return code=1." << DONE;
            return;
        }

        /*!
         ** execute() - Perform specific MMCS command
         ** This variety of execute() is intended to be executed overridden by DBBlockController functions
         ** @param args the command arguments
         ** @param reply       the command output stream. Refer to class MMCSCommandReply
         ** @param pController the DBConsoleController object that the command is to work on
         ** @param pTarget     the BlockControllerTarget list that the command is to work on (optional)
         */
        virtual void execute(
                std::deque<std::string> args,
                MMCSCommandReply& reply,
                DBConsoleController* pController,
                BlockControllerTarget* pTarget,
                std::vector<std::string>* validnames
                )
        {
            return execute(
                    args,
                    reply,
                    (ConsoleController*)pController,
                    pTarget,
                    validnames
                    );
        }


        // These two versions are the same as the first two, except
        // that they do not take a list of object names that have
        // been authorized (validnames).
        virtual void execute(
                std::deque<std::string> args,
                MMCSCommandReply& reply,
                ConsoleController* pController,
                BlockControllerTarget* pTarget=NULL
                )
        {
            reply << FAIL << "Internal error occurred: return code=2." << DONE;
            return;
        }

        virtual void execute(
                std::deque<std::string> args,
                MMCSCommandReply& reply,
                DBConsoleController* pController,
                BlockControllerTarget* pTarget=NULL
                )
        {
            return execute(
                    args,
                    reply,
                    (ConsoleController*)pController,
                    pTarget
                    );
        }

        /*!
         ** help() - Print extended command help to the reply stream
         ** @param args  the help command arguments
         ** @param reply the command output stream. Refer to class MMCSCommandReply
         */
        virtual void help(std::deque<std::string> args, MMCSCommandReply& reply) = 0;

        /*!
         ** name() - Returns the command name
         ** @return command name
         */
        virtual std::string  name() { return _name; }

        /*!
         ** description() - Returns the brief description
         ** @return brief command description
         */
        virtual std::string  description() { return _description; }

        /*!
         ** attributes() - Return the command attributes
         ** @return MMCSCommandAttributes for this object
         */
        virtual MMCSCommandAttributes& attributes() { return _attributes; };
    std::string get_usage() { return usage; }

    // Return whether the number of args is within useful bounds for the command.
    // By default, all commands expect zero arguments.
    // Your command should override this if it expects anything other than exactly zero arguments.
    // Even if it doesn't care how many arguments it gets, it should return 'true'.
    virtual bool checkArgs(std::deque<std::string>& args) { if(0 != args.size()) return false; else return true; }

    virtual bool doSpecialAuths(std::vector<std::string>& blocks, boost::shared_ptr<hlcs::security::Enforcer>& enforcer,
                                MMCSCommandProcessorStatus::procstat& stat, bgq::utility::UserId& user) {
        return false;
    }

    // Takes the commmand arguments and returns the block objects the command would like to use.
    // Security validation in the command processor will determine whether the command can
    // actually use the objects.  By default, the first argument is returned.
    virtual std::vector<std::string> getBlockObjects(std::deque<std::string>& cmdString, DBConsoleController* pController) {
        std::vector<std::string> arg0_vec;
        if(cmdString.size() != 0) {
            arg0_vec.push_back(cmdString[0]);
        }
        return arg0_vec;
    }
    protected:
        std::string _name;                  // name of the command
        std::string _description;           // brief (one line) description -- use help() for extended help
        MMCSCommandAttributes _attributes;  // command attributes
        std::string usage;
        bool _stopping;
    private:
        MMCSCommand(const MMCSCommand&);
        MMCSCommand& operator=(const MMCSCommand&);
};

#endif