/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file python.cpp
 *
 *	Python
 */

#include "python.hpp"

#include "python_module.hpp"

#include "game_manager.hpp"

/// Script always executed
char const *script =
	"from pyogre import *\n"
	"from vl import *\n"
	"def quit():\n"
	"	game.quit()\n"
	"print( 'vl imported' )\n";

vl::PythonContext::PythonContext( vl::GameManager *game_man )
{
	// TODO Ogre LogManager not initialised yet
	std::string message = "Initing python context.";
	std::cout << message << std::endl;
	//Ogre::LogManager::getSingleton().logMessage( message );

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
	}
	// Some error handling so that we can continue the application
	catch( ... )
	{
		// TODO Ogre LogManager not initialised yet
		message = "Exception occured when initing python context.";
		std::cerr << message << std::endl;
		//Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
	}
	if (PyErr_Occurred())
	{
		PyErr_Print();
	}
}

vl::PythonContext::~PythonContext( void )
{
}

void
vl::PythonContext::executePythonScript( vl::TextResource const &script )
{
	// TODO Ogre LogManager not initialised yet
	std::string message = "Running python script file " + script.getName() + ".";
	std::cout << message << std::endl;
	//Ogre::LogManager::getSingleton().logMessage( message );

	assert( script.get() );
	if( !script.get() )
	{ return; }

	try
	{
		// Run a python script.
		python::object result = python::exec(script.get(), _global, _global);
	}
	// Some error handling so that we can continue the application
	catch( ... )
	{
		// TODO these should be moved to ingame console and log file
		// TODO Ogre LogManager not initialised yet
		message = "Exception occured in python script: " + script.getName();
		std::cerr << message << std::endl;
		//Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
	}
	if( PyErr_Occurred() )
	{
		PyErr_Print();
	}
}

void
vl::PythonContext::executePythonCommand(const std::string& cmd)
{
	try
	{
		// Run a python script.
		python::object result = python::exec(cmd.c_str(), _global, _global);
	}
	// Some error handling so that we can continue the application
	catch( ... )
	{
		// TODO these should be moved to ingame console and log file
		// TODO Ogre LogManager not initialised yet
		std::string message = "Exception occured in : " + cmd;
		std::cerr << message << std::endl;
		//Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
	}
	if( PyErr_Occurred() )
	{
		PyErr_Print();
	}
}
