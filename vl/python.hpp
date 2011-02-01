/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
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

	void executePythonScript( vl::TextResource const &script );

private :
	PythonContext( PythonContext const & );

	PythonContext & operator=( PythonContext const & );

	// Python related
	python::object _global;
};

}

#endif // VL_PYTHON_HPP
