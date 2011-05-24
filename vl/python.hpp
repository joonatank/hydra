/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *	@file python.hpp
 *
 *	Python Manager.
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

#ifndef VL_PYTHON_HPP
#define VL_PYTHON_HPP

#include <boost/python.hpp>

#include "resource.hpp"

#include "typedefs.hpp"

namespace python = boost::python;

namespace vl
{

// class Config;

class PythonContext
{
public :
	PythonContext( vl::GameManagerPtr game );

	~PythonContext( void );

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

	/// @brief do there exist a script with that name
	/// @return true if there is a script with that nam,e
	/// @param name script name
	bool hasScript(std::string const &name) const;

	/// @return true if script with the name has been executed
	bool hasBeenExecuted(std::string const &name) const;

private :
	struct Script
	{
		Script(vl::TextResource const &res = vl::TextResource(), bool aut = false)
			: script(res), auto_run(aut), executed(false)
		{}

		std::string name;
		vl::TextResource script;
		bool auto_run;
		bool executed;
	};

	PythonContext( PythonContext const & );

	PythonContext & operator=( PythonContext const & );

	bool _auto_run;

	// Using vector to preserve the order they are added
	std::vector<Script> _scripts;

	// Python related
	python::object _global;
};

}

#endif // VL_PYTHON_HPP
