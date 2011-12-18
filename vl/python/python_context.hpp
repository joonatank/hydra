/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-12
 *	@file python/python_context.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_PYTHON_CONTEXT_HPP
#define HYDRA_PYTHON_CONTEXT_HPP

#include "resource.hpp"

namespace vl
{

struct Script
{
	Script(std::string const &nam, vl::TextResource const &res = vl::TextResource(), bool aut = false)
		: name(nam), script(res), auto_run(aut), executed(false)
	{}

	std::string name;
	vl::TextResource script;
	bool auto_run;
	bool executed;
};

class PythonContext
{
public :
	/// @todo depricated
	virtual void executeScript( vl::TextResource const &script ) = 0;

	virtual void executeCommand( std::string const &cmd ) = 0;

	/// @brief add a script resource to the stack
	/// @param name
	/// @param 
	/// @param auto_run the script is run automatically
	/// scripts can be run and retrieved using the name
	/// auto run runs the script as soon as autoRunScripts is called
	/// if a script is added after that they are run instantly
	virtual void addScript(std::string const &name, vl::TextResource const &script, bool auto_run = false) = 0;

	/// @brief run all scripts marked for auto run which have not been ran yet
	virtual void autoRunScripts(void) = 0;

	/// @brief get the whole script resource
	/// throws vl::missing_resource if no such script has been added
	virtual vl::TextResource const &getScript(std::string const &name) const = 0;
	virtual vl::TextResource &getScript(std::string const &name) = 0;

	virtual vl::Script const &getScript(size_t index) const = 0;
	virtual vl::Script &getScript(size_t index) = 0;
	
	virtual size_t getNScripts(void) const = 0;

	/// @brief do there exist a script with that name
	/// @return true if there is a script with that nam,e
	/// @param name script name
	virtual bool hasScript(std::string const &name) const = 0;

	/// @return true if script with the name has been executed
	virtual bool hasBeenExecuted(std::string const &name) const = 0;

	virtual void reset(void) = 0;

};	// class PythonContex

}	// namespace vl

#endif // HYDRA_PYTHON_CONTEXT_HPP
