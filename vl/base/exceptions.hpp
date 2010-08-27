#ifndef VL_EXCEPTIONS_HPP
#define VL_EXCEPTIONS_HPP

#include <boost/exception/all.hpp>

#include "math/math.hpp"

namespace vl
{
	/// Generic description, mostly development place holder
	typedef boost::error_info<struct tag_desc, std::string> desc;
	/// Name of the object
	typedef boost::error_info<struct tag_name, std::string> name;
	/// Quaternion used
	typedef boost::error_info<struct tag_quaternion, vl::quaternion> quat;
	/// File name used
	typedef boost::error_info<struct tag_file, std::string> file_name;

	// UDP number of bytes
	typedef boost::error_info<struct tag_bytes, size_t> bytes;
	
	struct exception : virtual std::exception, virtual boost::exception {};

	/// Dynamic cast failed when it was necessary
	struct cast_error : public exception
	{
		virtual const char* what() const throw()
		{
			return "dynamic cast failed";
		}
	};

	/// ------------------ Function Parameter errors ---------------
	
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

	// TODO no idea what purpose this exception has
	struct no_object : public exception {};


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

	/// 
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
}	// namespace vl

#endif
