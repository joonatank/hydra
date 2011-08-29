/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file python.cpp
 *
 *	Python
 */

/// Interface
#include "python.hpp"

/// Module
#include "python_module.hpp"
/// Access to the Simulation data
#include "game_manager.hpp"

// Module
BOOST_PYTHON_MODULE(vl)
{
	export_math();

	export_animation();

	export_scene_graph();

	export_events();

	export_game();

	export_physics();

}	// endof hydra python module

/// Script always executed
/// @todo should this be broken down to multiple independent scripts?
/// @todo this definitely should not add the current working directory
/// to path, it should add the modules/exe directory where the pyogre module is
/// in. This needs to be passed as a parameter to the PythonContext.
/// On Linux if the path is not added, it fails to find pyogre module.
/// On Windows working directory is automatically in the path.
/// Both of these only work if the working directory is the exe directory.
char const *script =
	"import sys, os\n"
	"sys.path.append(os.getcwd())\n"
	"from pyogre import *\n"
	"from vl import *\n"
	"def quit():\n"
	"	game.quit()\n";

vl::PythonContext::PythonContext(vl::GameManagerPtr game_man)
	: _auto_run(false)
{
	try {
		Py_Initialize();

		// Add the module to the python interpreter
		// NOTE the name parameter does not rename the module
		// No idea why it's there
#if PY_MAJOR_VERSION > 2
		if(PyImport_AppendInittab("vl", PyInit_vl) == -1)
#else
		if(PyImport_AppendInittab("vl", initvl) == -1)
#endif
			throw std::runtime_error("Failed to add vl to the interpreter's "
					"builtin modules");

		// Retrieve the main module
		python::object main = python::import("__main__");

		// Retrieve the main module's namespace
		_global = main.attr("__dict__");

		// Import vl module
		python::handle<> ignored(( PyRun_String(script,
										Py_file_input,
										_global.ptr(),
										_global.ptr() ) ));

		// Add a global managers i.e. this and EventManager
		_global["game"] = python::ptr<>( game_man );
		vl::sink &python_sink_out = *(*game_man->getLogger()->getPythonOut());
		vl::sink &python_sink_err = *(*game_man->getLogger()->getPythonErr());
		boost::python::import("sys").attr("stdout") = python_sink_out;
		boost::python::import("sys").attr("stderr") = python_sink_err;
	}
	// Some error handling so that we can continue the application
	catch( ... ) {}
	if (PyErr_Occurred())
	{
		PyErr_Print();
	}
}

vl::PythonContext::~PythonContext( void )
{
}

void
vl::PythonContext::executeScript( vl::TextResource const &script )
{
	std::cout << vl::TRACE << "Running python script file " + script.getName() + "."
		<< std::endl;

	assert( script.get() );
	if( !script.get() )
	{ return; }

	try
	{
		// Run a python script.
		python::object result = python::exec(script.get(), _global, _global);
	}
	// Some error handling so that we can continue the application
	catch( ... ) {}
	if( PyErr_Occurred() )
	{
		PyErr_Print();
	}
}

void
vl::PythonContext::executeCommand(const std::string& cmd)
{
	try
	{
		// Run a python script.
		python::object result = python::exec(cmd.c_str(), _global, _global);
	}
	// Some error handling so that we can continue the application
	catch( ... ) {}
	if( PyErr_Occurred() )
	{
		PyErr_Print();
	}
}

void 
vl::PythonContext::addScript(std::string const &name, vl::TextResource const &script, bool auto_run)
{
	if( hasScript(name) )
	{ BOOST_THROW_EXCEPTION(vl::duplicate()); }

	_scripts.push_back(Script(script, auto_run));
	if(_auto_run && auto_run)
	{
		executeScript(script);
		_scripts.back().executed = true;
	}
}

void 
vl::PythonContext::autoRunScripts(void)
{
	_auto_run = true;
	std::vector<Script>::iterator iter;
	for( iter = _scripts.begin(); iter != _scripts.end(); ++iter )
	{
		if( iter->auto_run && !iter->executed )
		{
			executeScript(iter->script);
			iter->executed = true;
		}
	}
}

vl::TextResource const &
vl::PythonContext::getScript(std::string const &name) const
{
	for(std::vector<Script>::const_iterator iter = _scripts.begin(); iter != _scripts.end(); ++iter)
	{
		if(iter->name == name)
		{ return iter->script; }
	}
	
	// Not the best exception
	BOOST_THROW_EXCEPTION(vl::missing_script());
}
	
vl::TextResource &
vl::PythonContext::getScript(std::string const &name)
{	
	for(std::vector<Script>::iterator iter = _scripts.begin(); iter != _scripts.end(); ++iter)
	{
		if(iter->name == name)
		{ return iter->script; }
	}

	BOOST_THROW_EXCEPTION(vl::missing_script());
}

bool 
vl::PythonContext::hasScript(std::string const &name) const
{
	for(std::vector<Script>::const_iterator iter = _scripts.begin(); iter != _scripts.end(); ++iter)
	{
		if(iter->name == name)
		{ return true; }
	}

	return false;
}

bool 
vl::PythonContext::hasBeenExecuted(std::string const &name) const
{
	for(std::vector<Script>::const_iterator iter = _scripts.begin(); iter != _scripts.end(); ++iter)
	{
		if(iter->name == name)
		{ return iter->executed; }
	}

	BOOST_THROW_EXCEPTION(vl::missing_script());
}
