#ifndef VL_EXCEPTIONS_HPP
#define VL_EXCEPTIONS_HPP

/* TODO move to using strings
 * add support for descriptions on the specific casts
 * add what(), where() functions.
 */
namespace vl
{
	class exception
	{
		public :
			exception(const char *wha, const char *wher)
				: what(wha), where(wher)
			{}

			const char *what;
			const char *where;
	};

	struct null_pointer : public exception
	{
		null_pointer( const char *wher )
			: exception( "null_pointer", wher )
		{}
	};

	struct bad_cast : public exception
	{
		bad_cast( const char *wher )
			: exception( "bad_cast", wher )
		{}
	};

	struct bad_index : public exception
	{
		bad_index( const char *wher )
			: exception( "bad_index", wher )
		{}
	};

	class empty_param : public exception
	{
		public :
			empty_param( const char *wher )
				: exception( "empty_param", wher )
			{}

	};

	class no_object : public exception
	{
		public :
			no_object( const char *wher )
				: exception( "no_object", wher )
			{}

	};

	class scale_quaternion : public exception
	{
		public :
			scale_quaternion( const char *wher )
				: exception( "scale quaternion", wher )
			{}
	};

	class zero_scale : public exception
	{
		public :
			zero_scale( const char *wher )
				: exception( "zero scale", wher )
			{}
	};

	class duplicate : public exception
	{
		public :
			duplicate( const char *wher )
				: exception( "duplicate", wher )
			{}
	};

	class fifo_full : public exception
	{
		public :
			fifo_full( const char *wher )
				: exception( "fifo_full", wher )
			{}
	};

	class missing_file : public exception
	{
		public :
			missing_file( const char *wher )
				: exception( "missing_file", wher )
			{}
	};

	class invalid_file : public exception
	{
		public :
			invalid_file( const char *wher )
				: exception( "invalid file", wher )
			{}
	};
}	// namespace vl

#endif
