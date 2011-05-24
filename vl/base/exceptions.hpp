/**	@author Joonatan Kuosa
 *	@date 2010-02
 *	@file exceptions.hpp
 *
 *	Exceptions.
 */
#ifndef VL_EXCEPTIONS_HPP
#define VL_EXCEPTIONS_HPP

#include <boost/exception/all.hpp>

#include <OGRE/OgreQuaternion.h>

namespace vl
{
	/// Generic description, mostly development place holder
	typedef boost::error_info<struct tag_desc, std::string> desc;
	/// Name of the object
	typedef boost::error_info<struct tag_name, std::string> name;
	/// Quaternion used
	typedef boost::error_info<struct tag_quaternion, Ogre::Quaternion> quat;
	/// File name used
	typedef boost::error_info<struct tag_file, std::string> file_name;
	/// Resource name
	typedef boost::error_info<struct tag_file, std::string> resource_name;

	/// Name of the Factory used for object creation
	typedef boost::error_info<struct tag_file, std::string> factory_name;

	/// requested object type name
	typedef boost::error_info<struct tag_file, std::string> object_type_name;

	/// UDP number of bytes
	typedef boost::error_info<struct tag_bytes, size_t> bytes;

	struct exception : virtual std::exception, virtual boost::exception
	{
		virtual const char* what() const throw()
		{
			return "vl::exception";
		}
	};

	/// Dynamic cast failed when it was necessary
	struct cast_error : public exception
	{
		virtual const char* what() const throw()
		{
			return "dynamic cast failed";
		}
	};

	/// ------------------ Function Parameter errors ---------------
	struct invalid_id : public exception
	{
		virtual const char* what() const throw()
		{ return "Distributed object has an invalid ID."; }
	};

	/// Trying to add object it self
	struct this_pointer : public exception
	{
		virtual const char* what() const throw()
		{
			return "Parameter is the object itself.";
		}
	};

	/// Null pointer provided
	struct null_pointer : public exception
	{
		virtual const char* what() const throw()
		{
			return "null pointer parameter";
		}
	};

	/// Empty parameter provided to a function where that parameter is required
	struct empty_param : public exception
	{
		virtual const char* what() const throw()
		{
			return "empty parameter provided to a function";
		}
	};

	/// ------------------ Object errors --------------------
	struct no_native : public exception
	{
		virtual const char* what() const throw()
		{
			return "native object missing";
		}
	};

	/// Object of that name already exists, can not create duplicate
	struct duplicate : public exception
	{
		virtual const char* what() const throw()
		{
			return "object with that name exists";
		}
	};

	/// Event object creation using Factories

	/// No registered factory for the object name
	/// Use details factory_name for the type of Factory and
	/// object_type_name for the type of object the factory is creating
	struct no_factory : public exception
	{
		virtual const char* what() const throw()
		{
			return "No factory for the requested object found!";
		}
	};

	/// The Object typeName of the objects this factory will create is already
	/// registered.
	/// Use details factory_name for the type of Factory and
	/// object_type_name for the type of object the factory is creating
	struct duplicate_factory : public exception
	{
		virtual const char* what() const throw()
		{
			return "Factory with that object typeName already exists!";
		}
	};

	/// Math and transformation errors, mostly useful for debug builds
	/// And should be replaced in release builds.

	/// Rotation quaternion contains scale element
	struct scale_quaternion : public exception
	{
		virtual const char* what() const throw()
		{
			return "quaternion provided for rotation contains scaling";
		}
	};

	/// Scale value provided is zero
	struct zero_scale : public exception
	{
		virtual const char* what() const throw()
		{
			return "scaling with zero element is not valid";
		}
	};


	/// ------------- Array based container errors --------------

	/// Fifo buffer is full
	struct fifo_full : public exception
	{
		virtual const char* what() const throw()
		{ return "fifo buffer is full"; }
	};

	/// Trying to index an array out of bounds
	struct bad_index : public exception
	{
		virtual const char* what() const throw()
		{ return "array index out of bounds"; }
	};


	/// --------- File errors -------------

	/// Generic file error
	struct file_error : public exception {};

	/// File missing
	struct missing_file : public file_error
	{
		virtual const char* what() const throw()
		{ return "file missing"; }
	};

	/// Directory missing
	struct missing_dir : public file_error
	{
		virtual const char* what() const throw()
		{ return "Directory missing"; }
	};

	/// ---------- File parsing errors --------------

	/// the file is correctly formated but the content is not valid
	struct parsing_error : public exception
	{
		virtual const char* what() const throw()
		{ return "parsing error"; }
	};

	struct invalid_settings  : public parsing_error {};

	struct invalid_dotscene : public parsing_error {};
	struct invalid_tracking : public parsing_error {};

	/// ---------- UDP errors --------------
	struct short_message : public exception
	{
		virtual const char* what() const throw()
		{ return "Received message is shorter than expected."; }
	};

	struct long_message : public exception
	{
		virtual const char* what() const throw()
		{ return "Received message constains extra bytes."; }
	};

	/// ----------- Resource errors -------------
	/// @brief thrown by Resource loaders
	struct missing_resource : public exception
	{
		virtual const char* what() const throw()
		{ return "Resource not found."; }
	};

	/// @brief thrown by the Python context manager
	struct missing_script : public exception
	{
		virtual const char* what() const throw()
		{ return "Python script is not loaded."; }
	};

	/// ----------- Development throws ----------
	struct not_implemented : public exception
	{
		virtual const char* what() const throw()
		{ return "Not implemented! Shouldn't be here."; }
	};

}	// namespace vl

#endif	// VL_EXCEPTIONS_HPP
