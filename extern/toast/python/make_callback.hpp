#ifndef toast_python_make_callback_h
#define toast_python_make_callback_h

#include <iostream>
#include <stdexcept>

#include <boost/type_traits.hpp>
#include <boost/python.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_shifted_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

/**
 * \file
 * \ingroup pythongroup
 * \brief This file contains the make_callback function.
 */

/// @cond DONT_DOCUMENT_THIS

namespace toast {
namespace python {
namespace detail {

class GIL
{
    PyGILState_STATE state_;
public:
    GIL() : state_(PyGILState_Ensure()) {}
    ~GIL() { PyGILState_Release(state_); }
};

// wrapCallback and makeCallback stuff

#define WRAPCALLBACK_MAX_ARGS 6

template <int Arity, typename Signature>
struct Connector;

/* I need this object to not have its reference count modified
 * when outside of the GIL */
inline void make_callback_deleter(boost::python::object *o)
{
    PyGILState_STATE gil = PyGILState_Ensure();
    delete o;
    PyGILState_Release(gil);
}

#define WRAPCALLBACK_TRAIT(Z, N, _) , typename Traits::arg ## N ## _type 
// The arity in this is just to make the 0 argument callback wrapper a proper
// template function
#define WRAPCALLBACK(Z, N, _) \
template <typename ReturnType BOOST_PP_COMMA_IF(N) \
    BOOST_PP_ENUM_PARAMS(N, typename T) > \
struct WrapCallback ## N \
{ \
    static ReturnType exec(boost::shared_ptr<boost::python::object> &callback \
            BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS(N, T, arg) ) \
    { \
        GIL gil; \
        try \
        { \
            return boost::python::call<ReturnType>(callback->ptr() \
                    BOOST_PP_COMMA_IF(N) \
                    BOOST_PP_ENUM_PARAMS(N, arg)); \
        } \
        catch(boost::python::error_already_set const &e) \
        { \
		    PyErr_Print(); \
        } \
    } \
};

#define CONNECTOR(Z, N, _) \
template <typename Signature> \
struct Connector<N, Signature> \
{ \
    static boost::function<Signature> connect( \
            boost::python::object const &callback) \
    { \
        typedef boost::function_traits<Signature> Traits; \
        boost::shared_ptr<boost::python::object> cb(new \
                boost::python::object(callback), &make_callback_deleter); \
        return boost::bind(WrapCallback ## N <typename Traits::result_type \
                    BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_INC(N), \
                        WRAPCALLBACK_TRAIT, _)>::exec, cb \
                    BOOST_PP_COMMA_IF(N) \
                    BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(N), _)); \
    } \
};

BOOST_PP_REPEAT(WRAPCALLBACK_MAX_ARGS, WRAPCALLBACK, _)
BOOST_PP_REPEAT_FROM_TO(0, WRAPCALLBACK_MAX_ARGS, CONNECTOR, _)

#undef WRAPCALLBACK_MAX_ARGS
#undef WRAPCALLBACK_TRAIT
#undef WRAPCALLBACK
#undef CONNECTOR

} // namespace detail

/// @endcond

/** \addtogroup pythongroup
 *
 *  @{
 */

/**
 * \brief Makes a C++ boost::function<Signature> object from a callable python
 * object.
 *
 * The returned boost::function will print any exception thrown during the
 * execution of the 'callback' to stderr
 */
template <typename Signature>
boost::function<Signature> make_callback(boost::python::object const &callback)
{
    if(callback.ptr() == Py_None) 
        return boost::function<Signature>();
    if(!PyCallable_Check(callback.ptr()))
        throw std::runtime_error("object not callable");
    return detail::Connector<boost::function_traits<Signature>::arity,
                Signature>::connect(callback);
}

/** @} */

} // namespace python
} // namespace toast

#endif

