#ifndef toast_python_callback_hpp_INCLUDED
#define toast_python_callback_hpp_INCLUDED

#include <iostream>
#include <boost/python.hpp>
#include <boost/type_traits.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/mpl/bool.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>

/**
 * \file
 * \ingroup pythongroup
 * \brief This file contains the toast::python::signal_connect function.
 */

namespace toast { namespace python {

namespace detail {

template <typename ArgType>
inline boost::python::object
getobjhelper(ArgType a, boost::mpl::bool_<false> const & /* isref */,
  boost::mpl::bool_<false> const & /* isptr */)
{
  return boost::python::object(a);
}
    
template <typename ArgType>
inline boost::python::object
getobjhelper(ArgType a, boost::mpl::bool_<true> const & /* isref */,
  boost::mpl::bool_<false> const & /* isptr */)
{
  return boost::python::object(boost::python::ptr(&a));
}
    
template <typename ArgType>
inline boost::python::object
getobjhelper(ArgType a, boost::mpl::bool_<false> const & /* isref */,
  boost::mpl::bool_<true> const & /* isptr */)
{
  return boost::python::object(boost::python::ptr(a));
}

template <typename ArgType>
inline boost::python::object getobject( ArgType a )
{
  boost::python::object ret = getobjhelper<ArgType>(a,
      typename boost::is_reference<ArgType>::type(),
      typename boost::is_pointer<ArgType>::type());
  return ret;

}

inline PyObject* pyobject_call(boost::python::object& o,
  boost::python::tuple& args, boost::python::dict& kw)
{
  PyObject *r = PyObject_Call(o.ptr(), args.ptr(), kw.ptr());
  if( !r )
    PyErr_Print();
  return r;
}

inline PyObject* pyobject_call(boost::python::object& o,
  boost::python::list& args, boost::python::dict& kw)
{
  boost::python::tuple newargs(args);
  return pyobject_call(o, newargs, kw);
}

template <typename T_return>
struct callpyobj0
{
  static T_return
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::object ret(pyobject_call(o, args, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <>
struct callpyobj0<void>
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    pyobject_call(o, args, kw);
    PyGILState_Release(state);
  }
};

template <typename T_return, typename T_arg1>
struct callpyobj1
{
  static T_return
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.extend( boost::python::list(args) );
    boost::python::object ret(pyobject_call(o, newargs, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <typename T_arg1>
struct callpyobj1<void, T_arg1>
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.extend( boost::python::list(args) );
    pyobject_call(o, newargs, kw);
    PyGILState_Release(state);
  }
};

template <typename T_return, typename T_arg1, typename T_arg2>
struct callpyobj2
{
  static T_return
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.extend( boost::python::list(args) );
    boost::python::object ret(pyobject_call(o, newargs, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <typename T_arg1, typename T_arg2>
struct callpyobj2<void, T_arg1, T_arg2>
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.extend( boost::python::list(args) );
    pyobject_call(o, newargs, kw);
    PyGILState_Release(state);
  }
};

template <typename T_return, typename T_arg1, typename T_arg2,
          typename T_arg3>
struct callpyobj3
{
  static T_return
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.extend( boost::python::list(args) );
    boost::python::object ret(pyobject_call(o, newargs, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <typename T_arg1, typename T_arg2, typename T_arg3>
struct callpyobj3<void, T_arg1, T_arg2, T_arg3>
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.extend( boost::python::list(args) );
    pyobject_call(o, newargs, kw);
    PyGILState_Release(state);
  }
};

template <typename T_return, typename T_arg1, typename T_arg2,
          typename T_arg3, typename T_arg4>
struct callpyobj4
{
  static T_return
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.extend( boost::python::list(args) );
    boost::python::object ret(pyobject_call(o, newargs, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <typename T_arg1, typename T_arg2, typename T_arg3,
          typename T_arg4>
struct callpyobj4<void, T_arg1, T_arg2, T_arg3, T_arg4 >
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.extend( boost::python::list(args) );
    pyobject_call(o, newargs, kw);
    PyGILState_Release(state);
  }
};

template <typename T_return, typename T_arg1, typename T_arg2,
          typename T_arg3, typename T_arg4, typename T_arg5>
struct callpyobj5
{
  static T_return 
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4, T_arg5 arg5 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.append( getobject<T_arg5>(arg5) );
    newargs.extend( boost::python::list(args) );
    boost::python::object ret(pyobject_call(o, newargs, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <typename T_arg1, typename T_arg2, typename T_arg3,
          typename T_arg4, typename T_arg5>
struct callpyobj5<void, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4, T_arg5 arg5 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.append( getobject<T_arg5>(arg5) );
    newargs.extend( boost::python::list(args) );
    pyobject_call(o, newargs, kw);
    PyGILState_Release(state);
  }
};

template <typename T_return, typename T_arg1, typename T_arg2,
          typename T_arg3, typename T_arg4, typename T_arg5,
          typename T_arg6>
struct callpyobj6
{
  static T_return
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4, T_arg5 arg5, T_arg6 arg6 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.append( getobject<T_arg5>(arg5) );
    newargs.append( getobject<T_arg6>(arg6) );
    newargs.extend( boost::python::list(args) );
    boost::python::object ret(pyobject_call(o, newargs, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <typename T_arg1, typename T_arg2, typename T_arg3,
          typename T_arg4, typename T_arg5, typename T_arg6>
struct callpyobj6<void, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4, T_arg5 arg5, T_arg6 arg6 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.append( getobject<T_arg5>(arg5) );
    newargs.append( getobject<T_arg6>(arg6) );
    newargs.extend( boost::python::list(args) );
    pyobject_call(o, newargs, kw);
    PyGILState_Release(state);
  }
};

template <typename T_return, typename T_arg1, typename T_arg2,
          typename T_arg3, typename T_arg4, typename T_arg5,
          typename T_arg6, typename T_arg7>
struct callpyobj7
{
  static T_return
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4, T_arg5 arg5, T_arg6 arg6, T_arg7 arg7 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.append( getobject<T_arg5>(arg5) );
    newargs.append( getobject<T_arg6>(arg6) );
    newargs.append( getobject<T_arg7>(arg7) );
    newargs.extend( boost::python::list(args) );
    boost::python::object ret(pyobject_call(o, newargs, kw));
    T_return &x = boost::python::extract<T_return&>(ret);
    PyGILState_Release(state);
    return x;
  }
};

template <typename T_arg1, typename T_arg2, typename T_arg3,
          typename T_arg4, typename T_arg5, typename T_arg6,
          typename T_arg7>
struct callpyobj7<void, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6,
                  T_arg7>
{
  static void
  call( boost::python::object o, boost::python::tuple args,
    boost::python::dict kw, T_arg1 arg1, T_arg2 arg2, T_arg3 arg3,
    T_arg4 arg4, T_arg5 arg5, T_arg6 arg6, T_arg7 arg7 )
  {
    PyGILState_STATE state(PyGILState_Ensure());
    boost::python::list newargs;
    newargs.append( getobject<T_arg1>(arg1) );
    newargs.append( getobject<T_arg2>(arg2) );
    newargs.append( getobject<T_arg3>(arg3) );
    newargs.append( getobject<T_arg4>(arg4) );
    newargs.append( getobject<T_arg5>(arg5) );
    newargs.append( getobject<T_arg6>(arg6) );
    newargs.append( getobject<T_arg7>(arg7) );
    newargs.extend( boost::python::list(args) );
    pyobject_call(o, newargs, kw);
    PyGILState_Release(state);
  }
};

template <int, typename Signature, typename Connection, typename T,
          typename Slot>
struct connector;
    
template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<0, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj0<typename traits::result_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw));
  }
};

template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<1, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj1<typename traits::result_type,
        typename traits::arg1_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw, _1) );
  }
};

template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<2, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj2<typename traits::result_type,
        typename traits::arg1_type,
        typename traits::arg2_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw, _1, _2));
  }
};

template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<3, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj3<typename traits::result_type,
        typename traits::arg1_type,
        typename traits::arg2_type,
        typename traits::arg3_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw, _1, _2, _3) );
  }
};

template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<4, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj4<typename traits::result_type,
        typename traits::arg1_type,
        typename traits::arg2_type,
        typename traits::arg3_type,
        typename traits::arg4_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw, _1, _2, _3, _4) );
  }
};

template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<5, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj5<typename traits::result_type,
        typename traits::arg1_type,
        typename traits::arg2_type,
        typename traits::arg3_type,
        typename traits::arg4_type,
        typename traits::arg5_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw, _1, _2, _3, _4, _5) );
  }
};

template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<6, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj6<typename traits::result_type,
        typename traits::arg1_type,
        typename traits::arg2_type,
        typename traits::arg3_type,
        typename traits::arg4_type,
        typename traits::arg5_type,
        typename traits::arg6_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw, _1, _2, _3, _4, _5, _6) );
  }
};

template <typename Signature, typename Connection, typename T,
          typename Slot>
struct connector<7, Signature, Connection, T, Slot>
{
  static Connection
  connect( boost::python::tuple args, boost::python::dict kw,
    Connection (T::*f)(Slot) )
  {
    using namespace boost::python;
    typedef boost::function_traits<Signature> traits;

    object xobj = args[0];
    T& x = extract<T&>(xobj);
    object callable = args[1];
    return (x.*f)(boost::bind(callpyobj7<typename traits::result_type,
        typename traits::arg1_type,
        typename traits::arg2_type,
        typename traits::arg3_type,
        typename traits::arg4_type,
        typename traits::arg5_type,
        typename traits::arg6_type,
        typename traits::arg7_type>::call,
        callable, boost::python::tuple(args.slice(2, _)),
        kw, _1, _2, _3, _4, _5, _6, _7) );
  }
};

} // detail namespace

/**
 * \addtogroup pythongroup
 * @{
 */

/**
 * \brief Wraps a signal/slot style connect function to be used with
 * Boost.Python
 *
 * Used something like
 * '.def("onUpdate", signal_connect<void ()>(&Class::onUpdate)'
 * the resulting python method will take a python callable and bind any
 * following arguments to the callback.  It will also return a connection which
 * if you need to be able to use it needs to be wrapped with Boost.Python.
 * This has worked well for use with Boost signals and Sigc++ signals as well
 * as another variant.
 *
 * It is important to note that arguments will be passed to python as the
 * signature describes.  If you use 'signal_connect<void (T &)>' then a
 * reference will be passed to python.  You will be able to modify T in python
 * and the effect will be visible in C++.  Also, if the object goes away in C++
 * you will have a dangling reference in python!!  You probably don't want to
 * store these objects on the python side.  If you want to simplify your life
 * you can copy the object by using 'signal_connect<void (T)>' instead, but
 * this could be a large copy, and you won't effect the object on the C++ side.
 */
template <typename Signature, typename Connection, typename T,
          typename Slot>
boost::python::object signal_connect( Connection (T::*f) (Slot) )
{
//  using namespace boost;
//  using namespace boost::python;

  return
    boost::python::raw_function(boost::function<Connection (boost::python::tuple, boost::python::dict)>
      (boost::bind(&detail::connector<boost::function_traits<Signature>::arity,
        Signature, Connection, T, Slot>::connect,
        _1, _2, f)) );
}

/** @} */

} // namespace python

} // namespace toast

/**
 * \defgroup pythongroup toast python library
 *
 * We love Boost.Python!  But it doesn't suit our needs for callbacks and
 * there isn't great support for wrapping containers.  So this module contains
 * some extensions that make our lives easier in those respects.  Take a look
 * at toast::python::signal_connect and toast::python::register_container.
 */

#endif // toast_python_callback_hpp_INCLUDED
