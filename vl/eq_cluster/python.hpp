/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
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

#ifndef EQ_OGRE_PYTHON_HPP
#define EQ_OGRE_PYTHON_HPP

#include <boost/python.hpp>

#include "resource.hpp"

namespace python = boost::python;

namespace vl
{
	class EventManager;
}

namespace eqOgre
{

class Config;

class PythonContext
{
public :
	PythonContext( eqOgre::Config *config, vl::EventManager *event_man );

	~PythonContext( void );

	void executePythonScript( vl::TextResource const &script );

private :
	PythonContext( PythonContext const & );

	PythonContext & operator=( PythonContext const & );

	// Python related
	python::object _global;
};

}

#endif // EQ_OGRE_PYTHON_HPP
