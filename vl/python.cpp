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
	"	game.quit()\n";

vl::PythonContext::PythonContext( vl::GameManager *game_man )
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
vl::PythonContext::executePythonScript( vl::TextResource const &script )
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
vl::PythonContext::executePythonCommand(const std::string& cmd)
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
