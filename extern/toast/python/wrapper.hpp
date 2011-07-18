#ifndef toast_python_wrapper_h
#define toast_python_wrapper_h

#include <sstream>
#include <stdexcept>
#include <boost/python.hpp>

/**
 * \file
 * \ingroup pythongroup
 * \brief This file contains a utility template for wrapping pure virtual functions
 */

namespace toast {
  namespace python {

/** \addtogroup pythongroup
 *
 *  @{
 */

/**
 * \brief Base class for python bindings that helps wrapping
 * pure virtual functions.
 *
 * The boost::python::wrapper template class only supplies a get_override()
 * member function that returns an override object (which could be None).
 * However, if it is used as it is in the documentation, invoking the
 * returned value will just result in an attribute error with no real
 * indication of what the user was trying to do. This class allows users to
 * easily handle pure virtual functions and give a more descriptive error
 * when the pure virtual function was not implemented.
 */
template <typename T>
struct wrapper : boost::python::wrapper<T>
{
  /**
   * retrieve the override
   * \throw std::runtime_error if the function wasn't implemented
   */
  boost::python::override get_pure_override( char const *name )
  {
    boost::python::override o( this->get_override(name) );
    if( not o )
    {
      std::ostringstream os;
      os << name << " is not implemented";
      throw std::runtime_error( os.str().c_str() );
    }
    return o;
  }
};

/** @} */

  }
}


#endif
