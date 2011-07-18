#ifndef toast_python_optional_h
#define toast_python_optional_h

#include <boost/python.hpp>
#include <boost/optional.hpp>
#include <toast/assert.hpp>

/**
 * \file
 * \ingroup pythongroup
 * \brief This file contains utility functions for writing boost::python wrappers
 * that make use of boost::optional.
 */


namespace toast {
	namespace python {

/** \addtogroup pythongroup
 *
 *  @{
 */

template <typename T, typename TfromPy>
struct object_from_python
{
	object_from_python() {
		boost::python::converter::registry::push_back
			(&TfromPy::convertible, &TfromPy::construct,
			 boost::python::type_id<T>());
	}
};

template <typename T, typename TtoPy, typename TfromPy>
struct register_python_conversion
{
	register_python_conversion() {
		boost::python::to_python_converter<T, TtoPy>();
		object_from_python<T, TfromPy>();
	}
};

	
/**
 * \brief Registers a boost::optional<Type> for in Python, making empty optionals into 'None' in Python
 *
 * Instantiate the type register_optional<T> to do the registration.
 *
 * When going from C++ to Python, empty optional's will become the 'None' object, and non-empty optionals of type boost::optional<T> will behave like T.\
 * From Python to C++, None instances will become empty boost::optional<T>'s, and non-none objects will instantiated boost::optional<T>'s.
**/
template <typename T>
struct register_optional : public boost::noncopyable
{
	struct optional_to_python
	{
		static PyObject * convert(const boost::optional<T>& value)
		{
			return (value ? boost::python::to_python_value<T>()(*value) :
				boost::python::detail::none());
		}
	};

	struct optional_from_python
	{
		static void * convertible(PyObject * source)
		{
			using namespace boost::python::converter;

			if (source == Py_None)
				return source;

			const registration& converters(registered<T>::converters);

			if (implicit_rvalue_convertible_from_python(source,  
					converters)) {
				rvalue_from_python_stage1_data data =
					rvalue_from_python_stage1(source, converters);
				return rvalue_from_python_stage2(source, data, converters);
			}
			return NULL;
		}

		static void construct(PyObject * source,
			boost::python::converter::rvalue_from_python_stage1_data * data)
		{
			using namespace boost::python::converter;

			void * const storage = ((rvalue_from_python_storage<boost::optional<T> > *)  
				data)->storage.bytes;

			if (data->convertible == source)	    // == None
				new (storage) boost::optional<T>(); // A Boost uninitialized value
			else
				new (storage) boost::optional<T>(*static_cast<T *>(data->convertible));

			data->convertible = storage;
		}
	};

	explicit register_optional() {
		register_python_conversion<boost::optional<T>,
			optional_to_python, optional_from_python>();
	}
};

template <>
struct register_optional<double> : public boost::noncopyable
{
  struct optional_to_python
  {
    static PyObject * convert(const boost::optional<double>& value)
    {
      return (value ? PyFloat_FromDouble(*value) : 
        boost::python::detail::none());
    }
  };
  
  struct optional_from_python
  {
    static void * convertible(PyObject * source)
    {
      using namespace boost::python::converter;
      
      if (source == Py_None || PyFloat_Check(source))
        return source;
      
      return 0;
    }
    
    static void construct(PyObject * source,
      boost::python::converter::rvalue_from_python_stage1_data * data)
    {
      TOAST_ASSERT(data->convertible == source);
      
      using namespace boost::python::converter;
      
      void * const storage = ((rvalue_from_python_storage<boost::optional<double> > *)  
        data)->storage.bytes;
      
      if (source == Py_None)	    // == None
        new (storage) boost::optional<double>(); // A Boost uninitialized value
      else
        new (storage) boost::optional<double>(PyFloat_AsDouble(source));
      
      data->convertible = storage;
    }
  };
  
  explicit register_optional() {
    register_python_conversion<boost::optional<double>,
      optional_to_python, optional_from_python>();
  }
};

/** @} */


}
}


#endif
