/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file python/python.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**	Python Manager.
 *	Manages the updates to the python context.
 *	Also the only execution point for the scripts python scripts.
 *
 *	Later will keep copies of the scripts it executes, so that they can easily
 *	be re-executed. Also keeps copies of the failed scripts and why they failed.
 *
 *	Links: This needs game manager and event manager at least probably more
 *	when we expand it.
 *	This receives data in TextResource format.
 */

#ifndef HYDRA_PYTHON_CONTEXT_IMPL_HPP
#define HYDRA_PYTHON_CONTEXT_IMPL_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

// Interface
#include "python_context.hpp"

#include <boost/python.hpp>

#include "resource.hpp"

#include "typedefs.hpp"

namespace python = boost::python;

namespace vl
{

class HYDRA_API PythonContextImpl : public vl::PythonContext
{
public :
	PythonContextImpl(vl::GameManagerPtr game);

	~PythonContextImpl(void);

	/// @todo depricated
	void executeScript( vl::TextResource const &script );

	void executeCommand( std::string const &cmd );

	/// @brief add a script resource to the stack
	/// @param name
	/// @param 
	/// @param auto_run the script is run automatically
	/// scripts can be run and retrieved using the name
	/// auto run runs the script as soon as autoRunScripts is called
	/// if a script is added after that they are run instantly
	void addScript(std::string const &name, vl::TextResource const &script, bool auto_run = false);

	/// @brief run all scripts marked for auto run which have not been ran yet
	void autoRunScripts(void);

	/// @brief get the whole script resource
	/// throws vl::missing_resource if no such script has been added
	vl::TextResource const &getScript(std::string const &name) const;
	vl::TextResource &getScript(std::string const &name);

	vl::Script const &getScript(size_t index) const;
	vl::Script &getScript(size_t index);
	
	size_t getNScripts(void) const;

	/// @brief do there exist a script with that name
	/// @return true if there is a script with that nam,e
	/// @param name script name
	bool hasScript(std::string const &name) const;

	/// @return true if script with the name has been executed
	bool hasBeenExecuted(std::string const &name) const;

	template<typename T>
	void addVariable(std::string variable_name, T &var);

private :

	PythonContextImpl(PythonContextImpl const &);

	PythonContextImpl &operator=(PythonContextImpl const &);

	bool _auto_run;

	// Using vector to preserve the order they are added
	std::vector<Script> _scripts;

	// Python related
	python::object _global;
};

template<typename T>
void
PythonContextImpl::addVariable(std::string variable_name, T &var)
{
	_global[variable_name] = python::ptr<>(&var);
}

}	// namespace vl


#endif // HYDRA_PYTHON_CONTEXT_IMPL_HPP
