/**	Joonatan Kuosa
 *	2010-12
 *
 *	Python
 */
#include "python.hpp"

#include "config_python.hpp"

#include "game_manager.hpp"

eqOgre::PythonContext::PythonContext( vl::GameManager *game_man )
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
		if (PyImport_AppendInittab("eqOgre", PyInit_eqOgre) == -1)
			throw std::runtime_error("Failed to add eqOgre to the interpreter's "
					"builtin modules");

		// Retrieve the main module
		python::object main = python::import("__main__");

		// Retrieve the main module's namespace
		_global = main.attr("__dict__");

		// Import eqOgre module
		python::handle<> ignored(( PyRun_String("from eqOgre import *\n"
										"print( 'eqOgre imported' )  \n",
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

eqOgre::PythonContext::~PythonContext(void )
{
}

void
eqOgre::PythonContext::executePythonScript( vl::TextResource const &script )
{
	// TODO Ogre LogManager not initialised yet
	std::string message = "Running python script file " + script.getName() + ".";
	std::cout << message << std::endl;
	//Ogre::LogManager::getSingleton().logMessage( message );

	EQASSERT( script.get() );
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

