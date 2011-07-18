#ifndef toast_python_container_hpp_INCLUDED
#define toast_python_container_hpp_INCLUDED

#include <exception>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <memory>
#include <bitset>
#include <string>
#include <boost/python.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/make_constructor.hpp>
#include <toast/typeinfo.hpp>
#include <toast/scope_guard.hpp>
#include <boost/mpl/bool.hpp>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
/**
 * \file
 * \ingroup pythongroup
 * \brief This file contains the toast::python::register_container helper.
 */

namespace toast { namespace python {

/**
 * \addtogroup pythongroup
 * @{
 */
 
/**
 * \brief Register a container to be automatically wrapped in python with
 * Boost.Python.
 *
 * It's like magic!  Just call register_container<std::vector<int> >(); in your
 * Boost.Python module and vectors of int's are available within python.  This
 * will work for vector, list, deque, map, set, and bitset.

 * Other
 * containers that meet the standard interface can be added easily and even
 * non-standard containers can be added with some extra work.
 * In both cases you just specialize register_container for the container
 * you want to wrap.  In the standard container case you just need to derive
 * from detail::register_std_container or detail::register_std_ra_container if
 * you want RandomAccess.  In the case of a non-standard container you'll have
 * to peek at the source for some examples of how we dealt with the standard
 * containers.
 */
template <typename T>
struct register_container
{
  register_container() { boost::STATIC_ASSERTION_FAILURE<false>(); }
};

/**
 * @}
 */
  
namespace detail {
    
template <typename Container>
struct implicit
{
  typedef register_container<Container> reg;
  static void * convertible(PyObject *obj)
  {
    return reg::convertible(obj);
  }
      
  typedef boost::python::converter::rvalue_from_python_stage1_data
  rvalue_data;
  typedef boost::python::converter::rvalue_from_python_storage<Container>
  rvalue_storage;

  static void destruct(Container *c) { c->~Container(); }

  static void
  construct( PyObject *obj, rvalue_data *data )
  {
    using namespace boost::python;
    void *storage = ((rvalue_storage*)data)->storage.bytes;
        
    new (storage) Container;
    toast::scope_guard guard(boost::bind(&implicit<Container>::destruct,
        static_cast<Container *>(storage)
      ));
    reg::copyContents(extract<object>(obj),
      *static_cast<Container *>(storage));

    data->convertible = storage;
    guard.dismiss();
  }
};

template <typename T, typename MakePtr>
struct refwrapper
{
  refwrapper( T & ) { ::boost::STATIC_ASSERTION_FAILURE<false>(); }
};

template <typename T>
struct refwrapper<T, boost::mpl::bool_<true> >
{
  boost::python::object obj;
  refwrapper( T &t ) : obj(boost::python::ptr(&t)) {}
};
    
template <typename T>
struct refwrapper<T, boost::mpl::bool_<false> >
{
  boost::python::object obj;
  refwrapper( T &t ) : obj(t) {}
};
    
template <typename T>
struct can_take_reference
  : boost::mpl::and_<boost::is_class<T>,
                     boost::mpl::not_<boost::is_same<T,
                                                     std::string> > > {};
template <typename T>
boost::python::object getrefobj( T &val )
{
  return refwrapper<T, typename can_take_reference<T>::type>(val).obj;
}

template <typename Container>
struct register_std_container
{
  typedef register_container<Container> reg;
  typedef typename Container::value_type value_type;
  static void copyContents(boost::python::object o, Container &container )
  {
    using namespace boost::python;
    handle<PyObject> iter(PyObject_GetIter(o.ptr()));
    while(handle<PyObject> item =
      handle<PyObject>(allow_null(PyIter_Next(iter.get())))) {
      reg::add(container, extract<value_type>(item.get()) );
    }
  }
      
  static Container * make( boost::python::object o )
  {
    using namespace boost::python;
    if( !converter::implicit<object, Container>::convertible(o.ptr()) ) {
      PyErr_SetObject(PyExc_TypeError, o.ptr());
      throw_error_already_set();
    }
    std::auto_ptr<Container> c(new Container);
    reg::copyContents(o, *c);
    return c.release();
  }

  static boost::python::list list( Container &container )
  {
    boost::python::list l;
    for( typename Container::iterator iter = container.begin();
         iter != container.end(); ++iter ) {
      l.append( getrefobj(*iter) );
    }
    return l;
  }
      
  register_std_container()
  {
    using namespace boost::python;
    // XXX - the static below might or might not be thread safe
    static bool registered = false;
    if( registered )
      return;
    class_.reset(new boost::python::class_<Container>(toast::type_id<Container>().name().c_str(), no_init) );
        
    class_->def("__init__", make_constructor(&reg::make) )
      .def( "__len__", &Container::size )
      ;
        
    implicitly_convertible<object, Container>();
    registered = true;
  }

protected:
  boost::shared_ptr<boost::python::class_<Container> > class_;
};

template <typename Container>
struct register_std_sequence : register_std_container<Container>
{
  typedef register_std_sequence<Container> reg;
  typedef typename Container::value_type value_type;

  static void * convertible(PyObject *obj)
  {
    return PySequence_Check(obj) ? obj : 0;
  }
      
  static void add( Container &c , value_type value )
  {
    c.push_back(value);
  }
      
  void def_iter(boost::mpl::bool_<true>)
  {
    this->class_->def("__iter__", 
      boost::python::iterator<Container,
      boost::python::return_internal_reference<> >() );
  }
      
  void def_iter(boost::mpl::bool_<false>)
  {
    this->class_->def( "__iter__",
      boost::python::iterator<Container>() ) ;
  }

  register_std_sequence()
  {
    using namespace boost::python;
    if( this->class_ ) {
      this->class_->def( "list", &reg::list )
        .def( "append", &reg::add )
        ;

      def_iter(typename can_take_reference<value_type>::type());
          
    }
  }
};
  
template <typename Container>
struct register_std_ra_sequence : register_std_sequence<Container>
{
  typedef typename Container::value_type value_type;
  typedef register_std_ra_sequence<Container> reg;
      
  static typename boost::mpl::if_<
    can_take_reference<value_type>,
    value_type &, value_type>::type
  getitem( Container &container, int index )
  {
    if( index < 0 )
      index += container.size();
    return container.at(index);
  }
      
  static boost::python::list
  getslice( Container &container, boost::python::slice &s )
  {
    boost::python::list l;
    int begin = 0;
    if( s.start() ) {
      begin = boost::python::extract<int>(s.start());
      if( begin < 0 )
        begin = container.size() + begin;
      if( begin < 0 )
        begin = 0;
      else if( static_cast<unsigned>(begin) >= container.size() )
        begin = container.size() - 1;
    }
    int incr = 1;
    if( s.step() ) {
      incr = boost::python::extract<int>(s.step());
    }
    int end = 0;
    if( incr > 0 )
      end = container.size();
    if( s.stop() ) {
      end = boost::python::extract<int>(s.stop());
      if( end < 0 )
        end = container.size() + end;
      if( end < 0 )
        end = 0;
      else if( static_cast<unsigned>(end) >= container.size() )
        end = container.size();
    }
    if( incr > 0 )
      for( int i = begin; i < end; i += incr)
        l.append(getrefobj(container[i]));
    else
      for( int i = begin; i > end; i += incr)
        l.append(getrefobj(container[i]));
    return l;
  }
      
  static void setitem( Container &container, int index, value_type value )
  {
    if( index < 0 )
      index += container.size();
    if( index < 0 || static_cast<unsigned>(index) >= container.size() ) {
      PyErr_SetString(PyExc_IndexError, "assignment index out of range");
      ::boost::python::throw_error_already_set();
    }
    container[index] = value;
  }

  static void delitem( Container &container, int index )
  {
    if( index < 0 )
      index += container.size();
    if( index < 0 || static_cast<unsigned>(index) >= container.size() ) {
      PyErr_SetString(PyExc_IndexError, "assignment index out of range");
      ::boost::python::throw_error_already_set();
    }
    typename Container::iterator iter = container.begin() + index;
    container.erase(iter);
  }
      
  void def_getitem(boost::mpl::bool_<true>)
  {
    this->class_->def("__getitem__", &reg::getitem,
      boost::python::return_internal_reference<>() );
  }
    
  void def_getitem(boost::mpl::bool_<false>)
  {
    this->class_->def( "__getitem__", &reg::getitem);
  }

  register_std_ra_sequence()
  {
    if( this->class_ ) {
      def_getitem(typename can_take_reference<value_type>::type());

      this->class_->def( "__getitem__",
        &reg::getslice)
        .def( "__setitem__",
          &reg::setitem)
        .def( "__delitem__",
          &reg::delitem)
        ;
    }
  }
};
    
} //detail namespace
  
template <typename KeyType, typename ValueType>
struct register_container< std::map<KeyType, ValueType> > 
  : detail::register_std_container< std::map<KeyType, ValueType> >
{
  typedef std::map<KeyType, ValueType> container_type;
  typedef register_container<container_type> reg;
  static void * convertible(PyObject *obj)
  {
    return PyMapping_Check(obj) ? obj : 0;
  }
    
  static void
  copyContents( boost::python::object o, container_type &container )
  {
    using namespace boost::python;

    // handle<PyObject> items(PyMapping_Items(o.ptr()));
    // python macros in a string constant where a non-const char * is required
    char items_[] = "items";
    handle<PyObject> items(PyObject_CallMethod(o.ptr(), items_, NULL));

    handle<PyObject> iter(PyObject_GetIter(items.get()));
    while( handle<PyObject> pitem =
      handle<PyObject>(allow_null(PyIter_Next(iter.get()))) ) {
      //boost::python::handle<PyObject> titem(pitem);
      tuple item = extract<tuple>(pitem.get());
      container.insert(std::pair<KeyType, 
        ValueType>(extract<KeyType>(item[0]),
          extract<ValueType>(item[1])));
    }
  }
    
  static boost::python::list keys( container_type &container )
  {
    boost::python::list l;
    for( typename container_type::iterator iter = container.begin();
         iter != container.end(); ++iter ) {
      l.append( boost::python::object(iter->first) );
    }
    return l;
  }

  static boost::python::list values( container_type &container )
  {
    boost::python::list l;
    for( typename container_type::iterator iter = container.begin();
         iter != container.end(); ++iter ) {
      l.append( detail::getrefobj(iter->second) );
    }
    return l;
  }
    
  static bool has_key( container_type &container, KeyType key )
  {
    return container.find(key) != container.end();
  }

  static boost::python::object
  get(container_type &container, KeyType key,
    boost::python::object def = boost::python::object() )
  {
    typename container_type::iterator iter = container.find( key );
    if( iter == container.end() )
      return def;
    return detail::getrefobj(iter->second);
  }

  static boost::python::object
  getitem( container_type &container, KeyType key )
  {
    typename container_type::iterator iter = container.find(key);
    if( iter == container.end() ) {
      PyErr_SetObject( PyExc_KeyError, boost::python::object(key).ptr() );
      ::boost::python::throw_error_already_set();
    }
    return detail::getrefobj(iter->second);
  }
    
  static void
  setitem( container_type &container, KeyType key, ValueType value )
  {
    container[key] = value;
  }

  static void delitem( container_type &container, KeyType key )
  {
    typename container_type::iterator iter = container.find(key);
    if( iter == container.end() ) {
      PyErr_SetObject( PyExc_KeyError, boost::python::object(key).ptr() );
      ::boost::python::throw_error_already_set();
    }
    else
      container.erase(iter);
  }
    
  static boost::python::list items( container_type &container )
  {
    using namespace boost::python;
    boost::python::list l;
    for( typename container_type::iterator iter = container.begin();
         iter != container.end(); ++iter ) {
      l.append(boost::python::make_tuple(object(iter->first),
          detail::getrefobj(iter->second) ) );
    }
    return l;
  }

  BOOST_PYTHON_FUNCTION_OVERLOADS(map_get_overloads,
    register_container<container_type>::get,
    2, 3 );
    
  register_container()
  {
    if( this->class_ ) {
      this->class_->def( "keys", &reg::keys )
        .def( "values", &reg::values )
        .def( "has_key", &reg::has_key )
        .def( "__contains__", &reg::has_key )
        .def( "__getitem__", &reg::getitem )
        .def( "__setitem__", &reg::setitem )
        .def( "__delitem__", &reg::delitem )
        .def( "clear", &container_type::clear )
        .def( "items", &reg::items )
        .def( "get", &reg::get, map_get_overloads() )
        ;
    }
  }
};

template <typename ContainedType>
struct register_container< std::set<ContainedType> >
  : detail::register_std_container< std::set<ContainedType> >
{
  typedef std::set<ContainedType> container_type;
  static void * convertible(PyObject *obj)
  {
    boost::python::handle<PyObject> o(PyObject_GetIter(obj));
    if( !o ) {
      PyErr_Clear();
      return 0;
    }
    return obj;
  }
    
  static void add( container_type &container, ContainedType const &val )
  {
    container.insert(val);
  }
    
  static bool
  contains( container_type &container, ContainedType const &val )
  {
    typename container_type::iterator iter = container.find(val);
    return iter != container.end();
  }
    
  static void discard( container_type &container, ContainedType const &val )
  {
    container.erase(val);
  }
    
  static void remove( container_type &container, ContainedType const &val )
  {
    typename container_type::iterator iter = container.find(val);
    if( iter != container.end() )
      container.erase(iter);
    else {
      PyErr_SetObject( PyExc_KeyError, boost::python::object(val).ptr() );
      ::boost::python::throw_error_already_set();
    }
  }
    
  register_container()
  {
    if( this->class_ ) {
      this->class_->def( "__contains__",
        &register_container<container_type>::contains )
        .def( "__iter__",
          boost::python::iterator<container_type>() )
        .def( "add",
          &register_container<container_type>::add )
        .def( "clear", &container_type::clear )
        .def( "discard", &register_container<container_type>::discard )
        .def( "remove", &register_container<container_type>::remove )
        ;
        
    }
  }
};
  
template <typename ContainedType>
struct register_container< std::vector<ContainedType> >
  : detail::register_std_ra_sequence<std::vector<ContainedType> > {};
  
template <typename ContainedType>
struct register_container< std::deque<ContainedType> > 
  : detail::register_std_ra_sequence<std::deque<ContainedType> > {};
  
template <typename ContainedType>
struct register_container< std::list<ContainedType> >
  : detail::register_std_sequence<std::list<ContainedType> > {};

template <size_t bitsize>
struct register_container< std::bitset<bitsize> >
{
  typedef register_container< std::bitset<bitsize> > reg;
  typedef std::bitset<bitsize> container_type;
  static void * convertible(PyObject *obj)
  {
    return PySequence_Check(obj) ? obj : 0;
  }

  static container_type * make( boost::python::object o )
  {
    using namespace boost::python;
    if(!converter::implicit<object, container_type>::convertible(o.ptr())) {
      PyErr_SetObject(PyExc_TypeError, o.ptr());
      throw_error_already_set();
    }
    std::auto_ptr<container_type> c(new container_type);
    reg::copyContents(o, *c);
    return c.release();
  }

  static void
  copyContents(boost::python::object o, container_type &container )
  {
    using namespace boost::python;
    size_t len = PySequence_Length(o.ptr());
    for( size_t i = 0; i < len && i < bitsize; ++i )
    {
      handle<PyObject> val(PySequence_GetItem(o.ptr(), i));
      container.set( i, PyObject_IsTrue(val.get()) );
    }
  }

  static boost::python::list list( container_type &container )
  {
    boost::python::list l;
    for( size_t i = 0; i < bitsize; ++i )
      l.append( container[i] ? Py_True : Py_False );
    return l;
  }
    
  static void set( container_type &container, size_t index, int value )
  {
    container.set(index, value);
  }

  static container_type & setbit( container_type &container, size_t index )
  {
    return container.set(index);
  }

  static bool eq( container_type const &c1, container_type const &c2 )
  {
    return c1 == c2;
  }

  static bool ne( container_type const &c1, container_type const &c2 )
  {
    return c1 != c2;
  }

  static container_type or_( container_type const &c1,
    container_type const &c2 )
  {
    return c1 | c2;
  }

  static container_type xor_( container_type const &c1,
    container_type const &c2 )
  {
    return c1 ^ c2;
  }

  static container_type and_( container_type const &c1,
    container_type const &c2 )
  {
    return c1 & c2;
  }

  register_container()
  {
    using namespace boost::python;
    static bool registered = false;
    if( registered )
      return;
    class_<container_type>(toast::type_id<container_type>().name().c_str(),
      no_init)
      .def( "__init__", make_constructor(&reg::make) )
      .def( "__len__", &container_type::size )
      .def( "__getitem__", &container_type::test)
      .def( "__setitem__", &reg::set )
      .def( "__eq__", &reg::eq )
      .def( "__ne__", &reg::ne )
      .def( "__or__", &reg::or_ )
      .def( "__ior__", &container_type::operator |=, return_self<>() )
      .def( "__xor__", &reg::xor_ )
      .def( "__ixor__", &container_type::operator ^=, return_self<>() )
      .def( "__and__", &reg::and_ )
      .def( "__iand__", &container_type::operator &=, return_self<>() )
      .def( "__lshift__", &container_type::operator<< )
      .def( "__ilshift__", &container_type::operator <<=, return_self<>() )
      .def( "__rshift__", &container_type::operator>> )
      .def( "__irshift__", &container_type::operator >>=, return_self<>() )
      .def( "__invert__", &container_type::operator~ )
      .def( "__nonzero__", &container_type::any )
      .def( "__str__", &container_type::template
        to_string<char, std::char_traits<char>, std::allocator<char> > )
      .def( "count", &container_type::count )
      .def( "set", static_cast<container_type & (container_type::*)()>(&container_type::set), return_self<>() )
      .def( "set", &reg::setbit, return_self<>() )
      .def( "reset", static_cast<container_type & (container_type::*)()>(&container_type::reset), return_self<>() )
      .def( "reset", static_cast<container_type & (container_type::*)(size_t)>(&container_type::reset), return_self<>() )
      .def( "flip", static_cast<container_type & (container_type::*)()>(&container_type::flip), return_self<>() )
      .def( "flip", static_cast<container_type & (container_type::*)(size_t)>(&container_type::flip), return_self<>() )
      ;

    implicitly_convertible<object, container_type>();
    registered = true;
  }
};
  
} // namespace python
} // namespace toast

namespace boost { namespace python { namespace converter {

template <typename T>
struct implicit<boost::python::object, std::set<T> > 
  : ::toast::python::detail::implicit<std::set<T> > {};

template <typename T>
struct implicit<boost::python::object, std::vector<T> >
  : ::toast::python::detail::implicit<std::vector<T> > {};

template <typename T>
struct implicit<boost::python::object, std::deque<T> > 
  : ::toast::python::detail::implicit<std::deque<T> > {};

template <typename T>
struct implicit<boost::python::object, std::list<T> >
  : ::toast::python::detail::implicit<std::list<T> > {};

template <typename KeyType, typename ValueType>
struct implicit<boost::python::object, std::map<KeyType, ValueType> >
  : ::toast::python::detail::implicit<std::map<KeyType, ValueType> > {};

template <size_t n>
struct implicit<boost::python::object, std::bitset<n> >
  : ::toast::python::detail::implicit<std::bitset<n> > {};

} // namespace converter
} // namespace python
} // namespace boost

#endif // toast_python_container_hpp_INCLUDED


