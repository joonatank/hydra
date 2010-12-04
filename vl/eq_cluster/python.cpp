
#include "python.hpp"

#include "config.hpp"

#include "config_python.hpp"

#include "event_manager.hpp"

eqOgre::PythonContext::PythonContext( eqOgre::Config *config,
									  vl::EventManager *event_man )
{
	EQINFO << "Initing python context." << std::endl;

	try {
		Py_Initialize();

		// Add the module to the python interpreter
		// NOTE the name parameter does not rename the module
		// No idea why it's there
		if (PyImport_AppendInittab("eqOgre_python", initeqOgre_python) == -1)
			throw std::runtime_error("Failed to add eqOgre to the interpreter's "
					"builtin modules");

		// Retrieve the main module
		python::object main = python::import("__main__");

		// Retrieve the main module's namespace
		_global = main.attr("__dict__");

		// Import eqOgre module
		python::handle<> ignored(( PyRun_String("from eqOgre_python import *\n"
										"print 'eqOgre imported'       \n",
										Py_file_input,
										_global.ptr(),
										_global.ptr() ) ));

		// Add a global managers i.e. this and EventManager
		_global["config"] = python::ptr<>( config );
		_global["event_manager"] = python::ptr<>( event_man );
	}
	// Some error handling so that we can continue the application
	catch( ... )
	{
		std::cout << "Exception occured when initing python context." << std::endl;

	}
	if (PyErr_Occurred())
	{
		PyErr_Print();
	}
}



void
eqOgre::PythonContext::executePythonScript( vl::TextResource const &script )
{
	EQINFO << "Running python script file " << script.getName() << "." << std::endl;

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
		std::cout << "Exception occured in python script: " << script.getName()
			<< std::endl;
	}
	if (PyErr_Occurred())
	{
		PyErr_Print();
	}
}